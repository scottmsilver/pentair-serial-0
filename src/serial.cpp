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

#include "bounded_buffer.h"
#include "easylogging++.h"
#include "pentair.h"

std::string hex(const unsigned char* buffer, int size) {
  std::stringstream ss;
  for (const unsigned char* cp = buffer; cp < buffer + size; ++cp) {
    ss << std::hex << std::setw(2) << std::setfill('0') << +*cp << std::dec;
    // See http://cpp.indi.frih.net/blog/2014/09/tippet-printing-numeric-values-for-chars-and-uint8_t/ for why unary plus causes the correct thing to happen.
  }

  return ss.str();
}


std::string hex(unsigned char ch) {
  return hex(&ch, 1);
}

std::string hex(const std::vector<unsigned char>& buffer) {
  return hex(buffer.data(), buffer.size());
}


const int kDefaultSerialByteToRead = 128;


class SerialBusWorker  {
  bounded_buffer<unsigned char>& mBuffer;
  bool mStopped;
  boost::asio::serial_port& mSerialPort;

  SerialBusWorker(bounded_buffer<unsigned char>& buffer, boost::asio::serial_port& serial_port) : mBuffer(buffer), mSerialPort(serial_port) { }
  
  void foo() {
    while (!mStopped) {
      unsigned char buffer[kDefaultSerialByteToRead];
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
  bounded_buffer<unsigned char>& mBuffer;

  bool mStopped;
  
  void foo() {
    while (!mStopped) {
      
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


// 
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
  
  void readBlocking(unsigned char* wholeBuffer, int bufferSize, int timeoutMs) {
    CLOG(DEBUG, "serial") << "--> (size = " << bufferSize << ", timeout = " << timeoutMs << ")";
    
      boost::asio::read(ser_port, boost::asio::buffer(wholeBuffer, bufferSize));
    CLOG(DEBUG, "serial") << hex(wholeBuffer, bufferSize);
    CLOG(DEBUG, "serial") << "<-- readBlocking (size = " << bufferSize << ", timeout = " << timeoutMs << ")";
  }

  void readUntil(unsigned char x) {
    CLOG(DEBUG, "serial") << "Looking for " << std::hex << +x;
    for (;;) {
      unsigned char ch;
      readBlocking(&ch, 1, 100000); // FIX-ME supposed to be infinity.
      if (ch == x) {
	break;
      }
    }
  }

  unsigned char readByte() {
    unsigned char ch = 0;
    readBlocking(&ch, 1, mTimeoutMs);
    return ch;
  }

  int readShort() {
    int s16 = 0;

    // FIX-ME: endianness
    readBlocking(reinterpret_cast<unsigned char*>(&s16) + 1, 1, mTimeoutMs);
    readBlocking(reinterpret_cast<unsigned char*>(&s16), 1, mTimeoutMs);
    return s16;
  }
};

struct RawMessage {
  unsigned char mDestination;
  unsigned char mSource;
  unsigned char mCfiType;
  std::vector<unsigned char> mCfi;
  
  RawMessage(unsigned char destination, unsigned char source, unsigned char cfiType, unsigned char* cfi, int cfiSize) :
    mDestination(destination), mSource(source), mCfiType(cfiType), mCfi(cfi, cfi + cfiSize) {
  }

  std::string addressType(unsigned char address) const {
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
    unsigned char destinationAddress = reader.readByte();
    LOG(DEBUG) << "Destination Address: " << hex(&destinationAddress, 1);
    auto sourceAddress = reader.readByte();
    LOG(DEBUG) << "Source Address: " << hex(&sourceAddress, 1);
    auto cfiType = reader.readByte();
    LOG(DEBUG) << "Command Function Instruction Type: " << hex(&cfiType, 1);
    auto cfiSize = reader.readByte();
    LOG(DEBUG) << "Command Function Instruction Size: " << static_cast<int>(cfiSize);
    unsigned char cfi[255];
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
  el::Loggers::configureFromGlobal("logging.conf");

  // Now all the loggers will use configuration from file
  messageLoop();
}
