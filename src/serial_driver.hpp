/********************************************************************************
 *   File Name:
 *     serial_driver.hpp
 *
 *   Description:
 *     Provides an interface to the host computer's serial port.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/
#pragma once
#ifndef BUS_PIRATE_CPP_SERIAL_DRIVER_HPP
#define BUS_PIRATE_CPP_SERIAL_DRIVER_HPP

/* C++ Includes */
#include <string>
#include <memory>

/* Boost Includes */
#include <boost/asio.hpp>
#include <boost/regex.hpp>

/* Chimera Includes */
#include <Chimera/interface.hpp>
#include <Chimera/threading.hpp>

namespace HWInterface
{
  class SerialDriver;
  typedef std::shared_ptr<SerialDriver> SerialDriver_sPtr;
  typedef std::unique_ptr<SerialDriver> SerialDriver_uPtr;

  class SerialDriver : public Chimera::Serial::Interface
  {
  public:
    SerialDriver( std::string &device, const uint32_t delay_mS = 25 );
    ~SerialDriver() = default;

    Chimera::Status_t begin( const Chimera::Serial::Modes txMode = Chimera::Serial::Modes::BLOCKING,
                             const Chimera::Serial::Modes rxMode = Chimera::Serial::Modes::BLOCKING ) noexcept override;

    Chimera::Status_t configure(
        const uint32_t baud = 115200, const Chimera::Serial::CharWid width = Chimera::Serial::CharWid::CW_8BIT,
        const Chimera::Serial::Parity parity    = Chimera::Serial::Parity::PAR_NONE,
        const Chimera::Serial::StopBits stop    = Chimera::Serial::StopBits::SBITS_ONE,
        const Chimera::Serial::FlowControl flow = Chimera::Serial::FlowControl::FCTRL_NONE ) noexcept override;

    Chimera::Status_t end() noexcept override;

    Chimera::Status_t setBaud( const uint32_t buad ) noexcept override;

    Chimera::Status_t setMode( const Chimera::Serial::SubPeripheral periph,
                               const Chimera::Serial::Modes mode ) noexcept override;

    Chimera::Status_t write( const uint8_t *const buffer, const size_t length,
                             const uint32_t timeout_mS = 500 ) noexcept override;

    Chimera::Status_t read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) noexcept override;


    Chimera::Status_t readUntil( std::vector<uint8_t> &buffer, const boost::regex &expr,
                                 const uint32_t timeout_mS = 500 ) noexcept;

    /**
     *	Checks if the serial port is open or not
     *
     *	@return True if open, false if not
     */
    bool isOpen() noexcept;

    /**
     *	Flushes the system TX and RX serial port buffers
     *
     *	@return bool: True if success, false if not
     */
    bool flush() noexcept;


    /**
     *	Clears all configuration settings and re-opens the serial port
     *
     *	@return bool: True if success, false if not
     */
    bool reset() noexcept;

  private:
    std::string serialDevice;

    uint32_t ioDelay_mS; /**< Inserts a delay into IO operations so slower devices can keep up */


    boost::asio::io_service io;
    boost::asio::serial_port serialPort;
    boost::asio::deadline_timer timer;
    boost::asio::streambuf readData;

    boost::asio::streambuf inputStream;

    Chimera::Status_t open() noexcept;

    void callback_readComplete( const boost::system::error_code &error, const size_t bytesTransferred ) noexcept;
    void callback_timeoutExpired( const boost::system::error_code &error ) noexcept;

    Chimera::Status_t asyncResult;
    size_t bytesTransferred;
  };
}  // namespace HWInterface

#endif /* !BUS_PIRATE_CPP_SERIAL_DRIVER_HPP */
