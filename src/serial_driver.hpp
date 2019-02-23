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
    SerialDriver( std::string &device );
    ~SerialDriver() = default;

    Chimera::Serial::Status begin( const Chimera::Serial::Modes txMode, const Chimera::Serial::Modes rxMode ) override;
    Chimera::Serial::Status configure( const uint32_t baud,
                                       const Chimera::Serial::CharWid width,
                                       const Chimera::Serial::Parity parity,
                                       const Chimera::Serial::StopBits stop,
                                       const Chimera::Serial::FlowControl flow ) override;

    Chimera::Serial::Status end() override;

    Chimera::Serial::Status setBaud( const uint32_t buad ) override;

    Chimera::Serial::Status setMode( const Chimera::Serial::SubPeripheral periph, const Chimera::Serial::Modes mode ) override;

    Chimera::Serial::Status write( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) override;

    Chimera::Serial::Status read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) override;

  private:
    typedef std::shared_ptr<boost::asio::deadline_timer> DeadlineTimer_sPtr;
    typedef std::shared_ptr<boost::asio::serial_port> SerialPort_sPtr;

    boost::asio::io_context ioService;

    DeadlineTimer_sPtr timer;
    SerialPort_sPtr serial;
    std::string serialDevice;

    Chimera::Serial::Status open();

    void readCallback( bool &data_available,
                       DeadlineTimer_sPtr &timeout,
                       const boost::system::error_code &error,
                       std::size_t bytes_transferred );

    void waitCallback( SerialPort_sPtr &serialPort, const boost::system::error_code &error );
  };
}    // namespace HWInterface


#endif /* !BUS_PIRATE_CPP_SERIAL_DRIVER_HPP */
