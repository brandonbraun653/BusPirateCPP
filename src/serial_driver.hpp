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
    SerialDriver( std::string &device );
    ~SerialDriver() = default;

    Chimera::Serial::Status begin( const Chimera::Serial::Modes txMode = Chimera::Serial::Modes::BLOCKING,
                                   const Chimera::Serial::Modes rxMode = Chimera::Serial::Modes::BLOCKING ) override;

    Chimera::Serial::Status
        configure( const uint32_t baud                     = 115200,
                   const Chimera::Serial::CharWid width    = Chimera::Serial::CharWid::CW_8BIT,
                   const Chimera::Serial::Parity parity    = Chimera::Serial::Parity::PAR_NONE,
                   const Chimera::Serial::StopBits stop    = Chimera::Serial::StopBits::SBITS_ONE,
                   const Chimera::Serial::FlowControl flow = Chimera::Serial::FlowControl::FCTRL_NONE ) override;

    Chimera::Serial::Status end() override;

    Chimera::Serial::Status setBaud( const uint32_t buad ) override;

    Chimera::Serial::Status setMode( const Chimera::Serial::SubPeripheral periph, const Chimera::Serial::Modes mode ) override;

    Chimera::Serial::Status write( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) override;

    Chimera::Serial::Status read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) override;

    /**
     *
     *
     *	@param[in]	buffer
     *	@param[in]	length
     *	@param[in]	expr
     *	@param[in]	bytesTransferred
     *	@param[in]	timeout_mS
     *	@return Chimera::Serial::Status
     */
    Chimera::Serial::Status readUntil( std::vector<uint8_t> &buffer,
                                       const boost::regex &expr,
                                       const uint32_t timeout_mS = 500 );

  private:
    std::string serialDevice;


    boost::asio::io_service io;
    boost::asio::serial_port serialPort;
    boost::asio::deadline_timer timer;
    boost::asio::streambuf readData;

    Chimera::Serial::Status open();

    void callback_readComplete( const boost::system::error_code &error, const size_t bytesTransferred );
    void callback_timeoutExpired( const boost::system::error_code &error );

    Chimera::Serial::Status asyncResult;
    size_t bytesTransferred;
  };
}    // namespace HWInterface

#endif /* !BUS_PIRATE_CPP_SERIAL_DRIVER_HPP */
