#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/atomic.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/format.hpp"
#include <boost/iostreams/categories.hpp>  // source_tag
#include <boost/iostreams/stream.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread.hpp>

#include <served/served.hpp>
#include "concurrent_queue.h"
#include "bounded_buffer.h"
#include "easylogging++.h"
#include "json.hpp"
#include "pentair.h"

std::string hex(const char* buffer, unsigned long size) {
  std::stringstream ss;
  for (const char* cp = buffer; cp < buffer + size; ++cp) {
    ss << std::hex << std::setw(2) << std::setfill('0') << +*cp << std::dec;
    // See http://cpp.indi.frih.net/blog/2014/09/tippet-printing-numeric-values-for-chars-and-uint8_t/ for why unary plus causes the correct thing to happen.
  }
  
  return ss.str();
}

std::string hex(char ch) {
  return hex(&ch, 1);
}

std::string hex(const std::vector<char>& buffer) {
  return hex(buffer.data(), buffer.size());
}

// This is a source adapter for any underlying container
// implementing pop_back() and unread(). This is actually
// tightly bound to the bounded_buffer but I was a little lazy
// to pull out the templating, since I pulled this from the boost
// examples.
template<typename Container>
class BlockingContainerSource {
public:
  typedef typename Container::value_type char_type;
  typedef boost::iostreams::source_tag category;
  BlockingContainerSource(Container& container) : mContainer(container) { }
  
  // Read up to n bytes into s.
  // Return the number of bytes read or -1 to indicate Eof, by protocol.
  // Our implementation will never return -1, and will block waiting
  // for at least 1 byte, but will try to return data immediately if > 0 bytes
  // are available.
  // Typically n is the underlying buffer size of the stream.
  std::streamsize read(char_type* s, std::streamsize n) {
    // We want to read whatever bytes are available our container, but
    // not more then the size of the passed in buffer.
    std::streamsize bytesToRead = std::min(n, mContainer.unread());
    
    // Since we know the underlying container will block, we insist on reading
    // at least 1 byte.
    if (bytesToRead == 0) {
      bytesToRead = 1;
    }
    
    // Read the data.
    for (auto curBufferPos = s; curBufferPos < s + bytesToRead; curBufferPos++) {
      mContainer.pop_back(curBufferPos);
    }
    
    return bytesToRead;
  }
  
private:
  typedef typename Container::size_type size_type;
  Container& mContainer;
};

const int kDefaultSerialByteToRead = 128;

// Reads from file and repeatedly writes it to the bounded_buffer.
// This means that the same file is pushed through the system multiple times.
class SerialBusFileWorker  {
private:
  bounded_buffer<char>& mBuffer;
  bool mStopped;
  std::string mFileContents;
  
public:
  SerialBusFileWorker(bounded_buffer<char>& buffer, const std::string& filePath) : mBuffer(buffer) {
    std::ifstream myFile(filePath, std::ios::in | std::ios::binary);
    mFileContents.assign((std::istreambuf_iterator<char>(myFile)),
                         std::istreambuf_iterator<char>());
  }
  
  void foo() {
    while (!mStopped) {
      for (int i = 0; i < mFileContents.size(); i++) {
        mBuffer.push_front(mFileContents[i]);
      }
      
      // Wait some before we do this again.
      boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
    }
  }
};

// Reads from serial port and writes to the bounded_buffer.
// Writes data as soon as it's ready.
class SerialBusWorker  {
  bounded_buffer<char>& mBuffer;
  bool mStopped;
  boost::asio::serial_port& mSerialPort;
  
public:
  SerialBusWorker(bounded_buffer<char>& buffer, boost::asio::serial_port& serial_port) : mBuffer(buffer), mSerialPort(serial_port) {
    el::Loggers::getLogger("serial-bus-worker");
  }
  
  void foo() {
    while (!mStopped) {
      char buffer[kDefaultSerialByteToRead];
      boost::system::error_code error;
      auto bytesRead = mSerialPort.read_some(boost::asio::buffer(buffer), error);
      
      if (!error) {
        CLOG(INFO, "serial-bus-worker") << "read " << bytesRead << ": " << hex(buffer, bytesRead);
        
        // FIX-ME(ssilver) - This is kind of lame pushing in a character at at a time.
        // A future optimization would be to add a new method to the underlying buffer to
        // allow additions of more than one character at a time.
        for (int i = 0; i < bytesRead; i++) {
          mBuffer.push_front(buffer[i]);
        }
      } else {
        CLOG(DEBUG, "serial-bus-worker") << "error: " << error;
      }
    }
  }
};

// Reads from serial port and writes to the bounded_buffer.
// Writes data as soon as it's ready.
class SerialBusWriter  {
  bounded_buffer<char>& mBuffer;
  bool mStopped;
  boost::asio::serial_port& mSerialPort;
  static constexpr const char* kLoggerName = "serial-bus-writer-worker";

public:
  SerialBusWriter(bounded_buffer<char>& buffer, boost::asio::serial_port& serial_port) : mBuffer(buffer), mSerialPort(serial_port) {
    el::Loggers::getLogger(kLoggerName);
  }
  
  void foo() {
    while (!mStopped) {
      // FIX-ME(ssilver) - This is kind of lame pushing in a character at at a time.
      // A future optimization would be to add a new method to the underlying buffer to
      // allow additions of more than one character at a time.
      for (int i = 0; i < mBuffer.unread(); i++) {
        char byte;
        boost::system::error_code error;

        mBuffer.pop_back(&byte);
        auto bytesWritten = mSerialPort.write_some(boost::asio::buffer(&byte, 1), error);
        
        if (!error && bytesWritten == 1) {
          CLOG(DEBUG, kLoggerName) << "Wrote a byte";
        } else {
          CLOG(DEBUG, kLoggerName) << "error: " << error << " bytesWritten: " << bytesWritten;
        }
      }
    }
  }
};

// Reads data from the shared buffer, turns them into pentair_t::message_t's and puts
// them on a shared queue passed in at construction time.
class BufferReaderMessageWriter {
  bounded_buffer<char>& mBuffer;
  moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>>& mQueue;
  bool mStopped;
  
public:
  BufferReaderMessageWriter(bounded_buffer<char>& buffer, moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>>& queue) : mBuffer(buffer), mQueue(queue) {}
  
  // Return the sum of the bytes in the std::string, treating them as unsigned.
  // If I were more C++ wizard I'm sure there was some way to use
  // std::accumulate on a signed char array and not have the values be treated
  // as signed...
  int sumUnsignedBytes(const std::string& buffer) const {
    int bodySum = 0;
    for (auto c : buffer) {
      bodySum += static_cast<unsigned char>(c);
    }
    
    return bodySum;
  }
  
  void foo() {
    // A BoundedBuffer source is essentially an iostream adapter that
    // we use to interface with the kstream, which wants to read from an iostream.
    // You can logically think of this as glue between the buffer and the kstream.
    typedef BlockingContainerSource<bounded_buffer<char>> BoundedBufferSource;
    BoundedBufferSource source(mBuffer);
    boost::iostreams::stream<BoundedBufferSource> in(source);
    kaitai::kstream stream(&in);
    
    while (!mStopped) {
      try {
        std::shared_ptr<pentair_t::message_t> message(new pentair_t::message_t(&stream));
        
        // The checksum is the sum of all the bytes from the 0xa5 until the checksum bytes, but not including them.
        int calculatedChecksum = 0xa5 + message->header()->unknown0() +
        message->address()->destination() + message->address()->source() +
        message->command()->type() + message->command()->size() +
        sumUnsignedBytes(message->command()->_raw_body());
        
        if (calculatedChecksum != message->checksum()->value()) {
          // Toss the packet if checksums don't match.
          LOG(ERROR) << "Tossing out bad packet" << std::endl;
        } else {
          mQueue.enqueue(message);
        }
      } catch (...) {
        LOG(ERROR) << "Couldn't parse message: " << boost::current_exception_diagnostic_information();
      }
    }
  }
};


struct PoolOrSpaStatus {
  bool mOn;
  int mTemperature;
  bool mIsFarenheit;
};


void to_json(nlohmann::json& j, const PoolOrSpaStatus& status) {
  j = nlohmann::json({{"temperature", status.mTemperature},
                      {"on", status.mOn},
                      {"isFarenheit", status.mIsFarenheit}});
}

void from_json(const nlohmann::json& j, PoolOrSpaStatus& status) {
  status.mTemperature = j.at("temperature");
}

struct EquipmentStatus {
  bool mOn;
};

void to_json(nlohmann::json& j, const EquipmentStatus& status) {
  j = nlohmann::json({{"on", status.mOn}});
}

void from_json(const nlohmann::json& j, EquipmentStatus& status) {
  status.mOn = j.at("on");
}


PoolOrSpaStatus gPoolStatus;
PoolOrSpaStatus gSpaStatus;
EquipmentStatus gAux1Status;
EquipmentStatus gAux2Status;
EquipmentStatus gAux3Status;
EquipmentStatus gAux4Status;

// Reads pentair_t::message_t's from the shared queue and ...
class MessageReader {
  moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>>& mQueue;
  bool mStopped;
  
public:
  MessageReader(moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>>& queue) : mQueue(queue) {}
  
  void foo() {
    using boost::format;
    
    while (!mStopped) {
      std::shared_ptr<pentair_t::message_t> message;
      mQueue.wait_dequeue(message);
      
      std::cout << format("%03u: %02X < %02X") % static_cast<unsigned int>(message->command()->type()) % static_cast<unsigned int>(message->address()->destination()) % static_cast<unsigned int>(message->address()->source()) << " " << std::endl;
      
      if (message->command()->type() == pentair_t::command_t::COMMAND_TYPE_CONTROLLER_STATUS_TYPE) {
        pentair_t::controller_status_t* controllerStatus = static_cast<pentair_t::controller_status_t*>(message->command()->body());
        
        gPoolStatus.mTemperature = static_cast<unsigned int>(controllerStatus->pool_temp());
        gPoolStatus.mOn =  controllerStatus->pool_status();
        gPoolStatus.mIsFarenheit = controllerStatus->temperature_units() == pentair_t::controller_status_t::TEMPERATURE_UNITS_FARENHEIT;
        
        gSpaStatus.mTemperature = static_cast<unsigned int>(controllerStatus->spa_temp());
        gSpaStatus.mOn =  controllerStatus->spa_status();
        gSpaStatus.mIsFarenheit = controllerStatus->temperature_units() == pentair_t::controller_status_t::TEMPERATURE_UNITS_FARENHEIT;
        
        gAux1Status.mOn = controllerStatus->aux1_status();
        gAux2Status.mOn = controllerStatus->aux2_status();
        gAux3Status.mOn = controllerStatus->aux3_status();
        gAux4Status.mOn = controllerStatus->aux4_status();
      }
    }
  }
};


// Write a Json version of t using nlohmann::json. This means that
// t must support to_json() and from_json().
template <class T>
void writeJsonResponse(served::response& res, T&& t) {
  nlohmann::json responseJson(t);
  std::stringstream responseStream;
  responseStream << responseJson;
  res << responseStream.str();
}

INITIALIZE_EASYLOGGINGPP

int main2(int argc, char* argv[]) {
  // Configure logging.
  START_EASYLOGGINGPP(argc, argv);
  el::Loggers::configureFromGlobal("logging.conf");
  
  // Set up our shared data structures between threads.
  bounded_buffer<char> buffer(2048);
  moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>> queue;
  
  
  // Construct the right kind of workers.
  std::unique_ptr<SerialBusFileWorker> serialBusFileWorker;
  std::unique_ptr<SerialBusWorker> serialBusWorker;
  std::unique_ptr<boost::asio::io_service> io_svc;
  std::unique_ptr<boost::asio::serial_port> serial_port;
  
  if (argc > 1 && strcmp("serial", argv[1]) == 0) {
    std::cout << "Using serial." << std::endl;
    io_svc.reset(new boost::asio::io_service());
    serial_port.reset(new boost::asio::serial_port(*io_svc, "/dev/ttyUSB0"));
    serial_port->set_option(boost::asio::serial_port_base::baud_rate(9600));
    serial_port->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    serial_port->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    serialBusWorker.reset(new SerialBusWorker(buffer, *serial_port));
  } else if (argc > 1) {
    // Support reading from a file for testing reasons. The file is just a dump
    // from the serial port obtained via "cat < /dev/ttyUSB0 > myfile"
    auto filePath = argv[1];
    serialBusFileWorker.reset(new SerialBusFileWorker (buffer, filePath));
    std::cout << "Using dummy file: \"" << filePath << "\"." << std::endl;
  } else {
    std::cerr << "Must specify \"serial\" or a path/to/file." << std::endl;
    exit(-1);
  }
  
  // Get and set pool temperature.
  // Get and set spa temperature.
  // Get and set spa jets and on.
  // Get and set lights on.
  // Set light pattern.
  
  BufferReaderMessageWriter messageWriter(buffer, queue);
  MessageReader messageReader(queue);
  
  // Kick off the key threads to do work.
  auto serialBusWorkerThread = serialBusFileWorker.get() ? boost::thread(&SerialBusFileWorker::foo, serialBusFileWorker.release()) : boost::thread(&SerialBusWorker::foo, serialBusWorker.release());
  auto messageWriterThread = boost::thread(&BufferReaderMessageWriter::foo, &messageWriter);
  auto messageReaderThread = boost::thread(&MessageReader::foo, &messageReader);
  
  served::multiplexer mux;
  
  mux.handle("/pool/v1/{property}/status")
  .get([](served::response & res, const served::request & req) {
    std::string property = req.params["property"];

    std::stringstream responseStream;

    if (property.compare("pool") == 0) {
      writeJsonResponse(res, gPoolStatus);
    } else if (property.compare("spa") == 0) {
      writeJsonResponse(res, gSpaStatus);
    } else if (property.compare("lights") == 0) {
      writeJsonResponse(res, gAux1Status);
    }
    
    res << responseStream.str();
  });
  
  served::net::server server("127.0.0.1", "8123", mux);
  server.run(10);
  
  // Wait for all the threads to stop. Which practically speaking never happens.
  serialBusWorkerThread.join();
  messageWriterThread.join();
  messageReaderThread.join();
}

// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/program_options.hpp>
using namespace boost::program_options;

#include <iostream>
using namespace std;

/* Auxiliary functions for checking input for validity. */

/* Function used to check that 'opt1' and 'opt2' are not specified
 at the same time. */
void conflicting_options(const variables_map& vm,
                         const char* opt1, const char* opt2)
{
  if (vm.count(opt1) && !vm[opt1].defaulted()
      && vm.count(opt2) && !vm[opt2].defaulted())
    throw logic_error(string("Conflicting options '")
                      + opt1 + "' and '" + opt2 + "'.");
}

/* Function used to check that of 'for_what' is specified, then
 'required_option' is specified too. */
void option_dependency(const variables_map& vm,
                       const char* for_what, const char* required_option)
{
  if (vm.count(for_what) && !vm[for_what].defaulted())
    if (vm.count(required_option) == 0 || vm[required_option].defaulted())
      throw logic_error(string("Option '") + for_what
                        + "' requires option '" + required_option + "'.");
}

int main(int argc, char* argv[])
{
  try {
    string ofile;
    string macrofile, libmakfile;
    bool t_given = false;
    bool b_given = false;
    string mainpackage;
    string depends = "deps_file";
    string sources = "src_file";
    string root = ".";
    
    options_description desc("Allowed options");
    desc.add_options()
    // First parameter describes option name/short name
    // The second is parameter to option
    // The third is description
    ("help,h", "print usage message")
    ("output,o", value(&ofile), "pathname for output")
    ("count", value<int>()->required(), "number of executions")
    ("macrofile,m", value(&macrofile), "full pathname of macro.h")
    ("two,t", bool_switch(&t_given), "preprocess both header and body")
    ("body,b", bool_switch(&b_given), "preprocess body in the header context")
    ("libmakfile,l", value(&libmakfile),
     "write include makefile for library")
    ("mainpackage,p", value(&mainpackage),
     "output dependency information")
    ("depends,d", value(&depends),
     "write dependencies to <pathname>")
    ("sources,s", value(&sources), "write source package list to <pathname>")
    ("root,r", value(&root), "treat <dirname> as project root directory")
    ;
    
    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    
    if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
    }
    
    conflicting_options(vm, "output", "two");
    conflicting_options(vm, "output", "body");
    conflicting_options(vm, "output", "mainpackage");
    conflicting_options(vm, "two", "mainpackage");
    conflicting_options(vm, "body", "mainpackage");
    
    conflicting_options(vm, "two", "body");
    conflicting_options(vm, "libmakfile", "mainpackage");
    conflicting_options(vm, "libmakfile", "mainpackage");
    
    option_dependency(vm, "depends", "mainpackage");
    option_dependency(vm, "sources", "mainpackage");
    option_dependency(vm, "root", "mainpackage");
    
    cout << "two = " << vm["two"].as<bool>() << "\n";
  }
  catch(exception& e) {
    cerr << e.what() << "\n";
  }
}
