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

#include "concurrent_queue.h"
#include "bounded_buffer.h"
#include "easylogging++.h"
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
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
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

// Reads data from the shared buffer, turns them into pentair_t::message_t's and puts
// them on a shared queue passed in at construction time.
class BufferReaderMessageWriter {
  bounded_buffer<char>& mBuffer;
  moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>>& mQueue;
  bool mStopped;
  
public:
  BufferReaderMessageWriter(bounded_buffer<char>& buffer, moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>>& queue) : mBuffer(buffer), mQueue(queue) {}
  
  // Return the sum of the bytes in the buffer, treating them as unsigned.
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
        message->command()->command_type() + message->command()->size() +
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
      
      std::cout << format("%03u: %02X < %02X") % static_cast<unsigned int>(message->command()->command_type()) % static_cast<unsigned int>(message->address()->destination()) % static_cast<unsigned int>(message->address()->source()) << std::endl;
    }
  }
};

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
  START_EASYLOGGINGPP(argc, argv);
  // Load configuration from file
  el::Loggers::configureFromGlobal("logging.conf");
  
  bounded_buffer<char> buffer(2048);
  moodycamel::BlockingReaderWriterQueue<std::shared_ptr<pentair_t::message_t>> queue;
  
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
  
  BufferReaderMessageWriter messageWriter(buffer, queue);
  MessageReader messageReader(queue);
  
  // Kick off the key threads to do work.
  auto serialBusWorkerThread = serialBusFileWorker.get() ? boost::thread(&SerialBusFileWorker::foo, serialBusFileWorker.release()) : boost::thread(&SerialBusWorker::foo, serialBusWorker.release());
  auto messageWriterThread = boost::thread(&BufferReaderMessageWriter::foo, &messageWriter);
  auto messageReaderThread = boost::thread(&MessageReader::foo, &messageReader);
  
  // Wait for all the threads to stop. Which practically speaking never happens.
  serialBusWorkerThread.join();
  messageWriterThread.join();
  messageReaderThread.join();
}
