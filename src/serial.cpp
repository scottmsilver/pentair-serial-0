#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/iostreams/categories.hpp>  // source_tag
#include <boost/iostreams/stream.hpp>

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

// This is a source adapter for any underlying container
// implementing pop_back() and unread(). This is typically
// some kind of bounded buffer.
template<typename Container>
class BlockingContainerSource {
public:
  typedef typename Container::value_type char_type;
  typedef boost::iostreams::source_tag category;
  BlockingContainerSource(Container& container) : container_(container) { }
  
  // Read up to n bytes into s.
  // Return the number of bytes read or -1 to indicate Eof, by protocol.
  // However, our implementation will never return -1, and will block waiting
  // for at least 1 byte, but will try to return data immediately if > 0 bytes
  // are available.
  // Typically n is the underlying buffer size. of the stream.
  std::streamsize read(char_type* s, std::streamsize n) {
    // We want to read whatever bytes are available our container, but
    // not more then the size of the passed in buffer.
    std::streamsize bytesToRead = std::min(n, container_.unread());
    
    // Since we know the underlying container will block, we insist on reading
    // at least 1 byte.
    if (bytesToRead == 0) {
      bytesToRead = 1;
    }
    std::cout << "reading: " << bytesToRead << std::endl;
    
    // Read the data.
    for (auto curBufferPos = s; curBufferPos < s + bytesToRead; curBufferPos++) {
      container_.pop_back(curBufferPos);
    }
    
    std::cout << "read: " << hex(s, bytesToRead) << std::endl;
    return bytesToRead;
  }
  
private:
  typedef typename Container::size_type size_type;
  Container& container_;
};

std::string hex(char ch) {
  return hex(&ch, 1);
}

std::string hex(const std::vector<char>& buffer) {
  return hex(buffer.data(), buffer.size());
}

const int kDefaultSerialByteToRead = 128;

// Worker function reads from file and repeatedly writes it to the bassed in buffer.
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
      
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
  }
};

// Worker function reads from serial port and writes output to buffer.
class SerialBusWorker  {
  bounded_buffer<char>& mBuffer;
  bool mStopped;
  boost::asio::serial_port& mSerialPort;
  
public:
  SerialBusWorker(bounded_buffer<char>& buffer, boost::asio::serial_port& serial_port) : mBuffer(buffer), mSerialPort(serial_port) { }
  
  void foo() {
    while (!mStopped) {
      char buffer[kDefaultSerialByteToRead];
      boost::system::error_code error;
      auto bytesRead = mSerialPort.read_some(boost::asio::buffer(buffer), error);
      
      if (!error) {
        for (int i = 0; i < bytesRead; i++) {
          CLOG(INFO, "serial-bus-worker") << "read " << bytesRead << ": " << hex(buffer, bytesRead);
          
          mBuffer.push_front(buffer[i]);
        }
      } else {
        CLOG(DEBUG, "serial-bus-worker") << "read " << bytesRead << ": " << hex(buffer, bytesRead);
      }
    }
  }
};

// Worker thread reads data from buffer, turns them into pentair_t::message_t's
class BufferReaderMessageWriter {
  bounded_buffer<char>& mBuffer;
  bool mStopped;
  
public:
  BufferReaderMessageWriter(bounded_buffer<char>& buffer) : mBuffer(buffer) {}
  
  void foo() {
    typedef BlockingContainerSource<bounded_buffer<char>> bounded_buffer_source;
    bounded_buffer_source source(mBuffer);
    boost::iostreams::stream<bounded_buffer_source> in(source);
    kaitai::kstream stream(&in);
    
    while (!mStopped) {
      pentair_t::message_t message(&stream);
      
      auto command = message.command();
      std::cout << "type: " << +command->command_type() << " checksum: " << message.checksum()->value() << std::endl;
    }
  }
};

class MessageReader {
};

//CLOG(DEBUG, "serial") << "--> (size = " << bufferSize << ", timeout = " << timeoutMs << ")";
//el::Loggers::getLogger("serial");

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
  START_EASYLOGGINGPP(argc, argv);
  // Load configuration from file
  el::Loggers::configureFromGlobal("/Users/ssilver/Google Drive/development/pool/serial/src/logging.conf");
  
  if (false) {
    std::ifstream myFile("/Users/ssilver/Google Drive/development/pool/serial/src/serial.out", std::ios::in | std::ios::binary);
    kaitai::kstream stream(&myFile);
    pentair_t pentair(&stream);
    
    for (auto message : *pentair.messages()) {
      auto command = message->command();
      std::cout << "type: " << +command->command_type() << " checksum: " << message->checksum()->value() << std::endl;
    }
  }
  
  if (false) {
    boost::asio::io_service io_svc;
    boost::asio::serial_port ser_port(io_svc, "/dev/ttyUSB0");
    ser_port.set_option(boost::asio::serial_port_base::baud_rate(9600));
    ser_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    ser_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
  }
  
  bounded_buffer<char> buffer(2048);
  
  SerialBusFileWorker serialBusFileWorker(buffer, "/Users/ssilver/Google Drive/development/pool/serial/src/serial3.out");
  BufferReaderMessageWriter messageWriter(buffer);
  
  //serialBusFileWorker.foo();
  //messageWriter.foo();
  
  auto serialBusFileWorkerThread = boost::thread(&SerialBusFileWorker::foo, &serialBusFileWorker);
  auto messageWriterThread = boost::thread(&BufferReaderMessageWriter::foo, &messageWriter);
  
  serialBusFileWorkerThread.join();
  messageWriterThread.join();
  
  // Now all the loggers will use configuration from file
  //messageLoop();
}
