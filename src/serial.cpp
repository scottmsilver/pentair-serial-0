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

class SerialBusFileWorker  {
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

class SerialHandler {
  int mBytesRead;
  boost::system::error_code mLastError;
  
  SerialHandler(const SerialHandler& that) = delete;
  
public:
  SerialHandler() : mBytesRead(0) {
  }
  
  void read_callback(bool& data_available, boost::asio::deadline_timer& timeout, const boost::system::error_code& error, std::size_t bytes_transferred) {
    LOG(DEBUG) << "read_callback: e:" << error << " bytes: " << bytes_transferred ;
    mLastError = error;
    
    if (error || !bytes_transferred) {
      // No data was read!
      data_available = false;
    } else {
      // will cause wait_callback to fire with an error
      mBytesRead += bytes_transferred;
      data_available = true;
      timeout.cancel();
    }
  }
  
  void wait_callback(boost::asio::serial_port& ser_port, const boost::system::error_code& error) {
    LOG(DEBUG) << "wait_callback e:" << error;
    
    if (error == boost::asio::error::operation_aborted) {
      // Data was read and this timeout was canceled
      return;
    } else {
      ser_port.cancel();  // will cause read_callback to fire with an error
    }
  }
  
  int getBytesRead() { return mBytesRead; }
  boost::system::error_code getLastError() { return mLastError; }
};


class SerialReader {  int mTimeoutMs;
public:
  boost::asio::io_service io_svc;
  boost::asio::serial_port ser_port;
  
  SerialReader(int timeoutMs) : mTimeoutMs(timeoutMs), ser_port(io_svc, "/dev/ttyUSB0") {
    el::Loggers::getLogger("serial");
    
    // Set serial port to 9600, N, 1.
    ser_port.set_option(boost::asio::serial_port_base::baud_rate(9600));
    ser_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    ser_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
  }
  
  void readBlocking(char* wholeBuffer, int bufferSize, int timeoutMs) {
    CLOG(DEBUG, "serial") << "--> (size = " << bufferSize << ", timeout = " << timeoutMs << ")";
    
    boost::asio::read(ser_port, boost::asio::buffer(wholeBuffer, bufferSize));
    CLOG(DEBUG, "serial") << hex(wholeBuffer, bufferSize);
    CLOG(DEBUG, "serial") << "<-- readBlocking (size = " << bufferSize << ", timeout = " << timeoutMs << ")";
  }
  
  void readUntil(char x) {
    CLOG(DEBUG, "serial") << "Looking for " << std::hex << +x;
    for (;;) {
      char ch;
      readBlocking(&ch, 1, 100000); // FIX-ME supposed to be infinity.
      if (ch == x) {
        break;
      }
    }
  }
  
  char readByte() {
    char ch = 0;
    readBlocking(&ch, 1, mTimeoutMs);
    return ch;
  }
  
  int readShort() {
    int s16 = 0;
    
    // FIX-ME: endianness
    readBlocking(reinterpret_cast<char*>(&s16) + 1, 1, mTimeoutMs);
    readBlocking(reinterpret_cast<char*>(&s16), 1, mTimeoutMs);
    return s16;
  }
};

struct RawMessage {
  char mDestination;
  char mSource;
  char mCfiType;
  std::vector<char> mCfi;
  
  RawMessage(char destination, char source, char cfiType, char* cfi, int cfiSize) :
  mDestination(destination), mSource(source), mCfiType(cfiType), mCfi(cfi, cfi + cfiSize) {
  }
  
  std::string addressType(char address) const {
    if (address == 0x0f) {
      return "broad";
    } else if ((address & 0xf0) == 0x10) {
      // All 0x1x are main controllers.
      return "cntrl";
    } else if ((address & 0xf0) == 0x20) {
      // All 0x1x are remote controllers.
      return "remte";
    } else if ((address & 0xf0) == 0x60) {
      // All 0x1x are pumps. x are the pump numbers.
      return "pumpX";
    } else {
      return "unkwn";
    }
  }
  
  void decodeMessage() {
    switch (mCfiType) {
      default:
        LOG(ERROR) << "Decode Failure: " << *this;
    }
  }
};

std::ostream& operator<<(std::ostream& os, RawMessage const& message) {
  os << message.addressType(message.mSource) << "|" << hex(message.mSource) << "->" << message.addressType(message.mDestination) << "|" << hex(message.mDestination) << " | " << hex(message.mCfiType) << ": " << hex(message.mCfi);
  return os;
}

void messageLoop() {
  SerialReader reader(10000);
  
  for (;;) {
    reader.readUntil(0xa5);
    LOG(DEBUG) << "Message Begin (0xa5)";
    auto unknown1 = reader.readByte();
    LOG(DEBUG) << "Unknown: " << hex(&unknown1, 1);
    char destinationAddress = reader.readByte();
    LOG(DEBUG) << "Destination Address: " << hex(&destinationAddress, 1);
    auto sourceAddress = reader.readByte();
    LOG(DEBUG) << "Source Address: " << hex(&sourceAddress, 1);
    auto cfiType = reader.readByte();
    LOG(DEBUG) << "Command Function Instruction Type: " << hex(&cfiType, 1);
    auto cfiSize = reader.readByte();
    LOG(DEBUG) << "Command Function Instruction Size: " << static_cast<int>(cfiSize);
    char cfi[255];
    reader.readBlocking(cfi, cfiSize, 5000);
    LOG(DEBUG) << "Command Function Instruction Data: " << hex(cfi, cfiSize);
    auto checkSum = reader.readShort();
    LOG(DEBUG) << "Check Sum: " << checkSum;
    int calculatedCheckSum = 0xa5 + unknown1 + destinationAddress + sourceAddress + cfiType + cfiSize + std::accumulate(cfi, cfi + cfiSize, 0, std::plus<int>());
    LOG(DEBUG) << "Calculated sum: " << calculatedCheckSum;
    
    if (calculatedCheckSum == checkSum) {
      RawMessage rawMessage(destinationAddress, sourceAddress, cfiType, cfi, cfiSize);
      rawMessage.decodeMessage();
    } else {
      LOG(ERROR) << "Tossing packet - failed checksum";
    }
  }
}

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
