/********************************************************************************
 *   File Name:
 *       serial_driver.cpp
 *
 *   Description:
 *       Implements the interface to the host PC's serial port
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Module Includes */
#include "serial_driver.hpp"

/* C++ Includes */
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

/* Boost Includes */
#include <boost/bind.hpp>
#include <boost/asio/serial_port_base.hpp>

using namespace Chimera::Serial;

namespace HWInterface
{
  SerialDriver::SerialDriver( std::string &device ) : io(), serialPort( io ), timer( io )
  {
    serialDevice = device;
  }

  Chimera::Serial::Status SerialDriver::begin( const Chimera::Serial::Modes txMode, const Chimera::Serial::Modes rxMode )
  {
    return open();
  }

  Chimera::Serial::Status SerialDriver::configure(
      const uint32_t baud, const CharWid width, const Parity parity, const StopBits stop, const FlowControl flow )
  {
    using namespace boost::asio;

    Status error = Status::OK;

    if ( !serialPort.is_open() )
    {
      error = Status::NOT_INITIALIZED;
    }
    else
    {
      /*------------------------------------------------
      System Baud Rate
      ------------------------------------------------*/
      serial_port_base::baud_rate BAUD( baud );
      serialPort.set_option( BAUD );

      /*------------------------------------------------
      Data Width
      ------------------------------------------------*/
      serial_port_base::character_size CHAR( static_cast<uint8_t>( width ) );
      serialPort.set_option( CHAR );

      /*------------------------------------------------
      Parity
      ------------------------------------------------*/
      static_assert( serial_port_base::parity::type::none == static_cast<int>( Parity::PAR_NONE ),
                      "Parity 'none' type does not match!" );
      static_assert( serial_port_base::parity::type::odd == static_cast<int>( Parity::PAR_ODD ),
                      "Parity 'odd' type does not match!" );
      static_assert( serial_port_base::parity::type::even == static_cast<int>( Parity::PAR_EVEN ),
                      "Parity 'even' type does not match!" );

      serial_port_base::parity PARITY( static_cast<serial_port_base::parity::type>( parity ) );
      serialPort.set_option( PARITY );

      /*------------------------------------------------
      Stop Bits
      ------------------------------------------------*/
      static_assert( serial_port_base::stop_bits::type::one == static_cast<int>( StopBits::SBITS_ONE ),
                      "Stop bits 'one' type does not match!" );
      static_assert( serial_port_base::stop_bits::type::onepointfive == static_cast<int>( StopBits::SBITS_ONE_POINT_FIVE ),
                      "Stop bits '1.5' type does not match!" );
      static_assert( serial_port_base::stop_bits::type::two == static_cast<int>( StopBits::SBITS_TWO ),
                      "Stop bits 'two' type does not match!" );

      serial_port_base::stop_bits STOP( static_cast<serial_port_base::stop_bits::type>( stop ) );
      serialPort.set_option( STOP );

      /*------------------------------------------------
      Flow Control
      ------------------------------------------------*/
      static_assert( serial_port_base::flow_control::type::none == static_cast<int>( FlowControl::FCTRL_NONE ),
                      "Flow control 'none' type does not match!" );
      static_assert( serial_port_base::flow_control::type::software == static_cast<int>( FlowControl::FCTRL_SW ),
                      "Flow control 'sw' type does not match!" );
      static_assert( serial_port_base::flow_control::type::hardware == static_cast<int>( FlowControl::FCTRL_HW ),
                      "Flow control 'hw' type does not match!" );

      serial_port_base::flow_control FLOW( static_cast<serial_port_base::flow_control::type>( flow ) );
      serialPort.set_option( FLOW );
    }

    return error;
  }

  Chimera::Serial::Status SerialDriver::end()
  {
    Status error = Status::OK;

    serialPort.close();

    return error;
  }

  Chimera::Serial::Status SerialDriver::setBaud( const uint32_t buad )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::Serial::Status SerialDriver::setMode( const Chimera::Serial::SubPeripheral periph,
                                                 const Chimera::Serial::Modes mode )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::Serial::Status SerialDriver::write( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS )
  {
    boost::asio::write( serialPort, boost::asio::buffer( buffer, length ) );
    return Status::OK;
  }

  Chimera::Serial::Status SerialDriver::read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS )
  {
    /*------------------------------------------------
    Start the asynchronous read
    ------------------------------------------------*/
    boost::asio::async_read( serialPort,
                             boost::asio::buffer( buffer, length ),
                             boost::bind( &SerialDriver::callback_readComplete,
                                          this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred ) );

    /*------------------------------------------------
    Initialize the timeout service
    ------------------------------------------------*/
    timer.expires_from_now( boost::posix_time::milliseconds( timeout_mS ) );
    timer.async_wait( boost::bind( &SerialDriver::callback_timeoutExpired, this, boost::asio::placeholders::error ) );

    /*------------------------------------------------
    Periodically grab updates from the io_service. Updates
    are propagated through the class callback_*() functions.
    ------------------------------------------------*/
    asyncResult      = Status::RX_IN_PROGRESS;
    bytesTransferred = 0;

    while ( asyncResult == Status::RX_IN_PROGRESS )
    {
      io.run_one();

      switch ( asyncResult )
      {
        case Status::RX_COMPLETE:
          timer.cancel();
          break;

        case Status::GENERIC_ERROR:
          timer.cancel();
          serialPort.cancel();
          break;

        case Status::TIMEOUT:
          serialPort.cancel();
          break;

        case Status::RX_IN_PROGRESS:
        default:
          break;
      }
    }

    return asyncResult;
  }

  Chimera::Serial::Status SerialDriver::readUntil( std::vector<uint8_t> &buffer,
                                                   const boost::regex &expr,
                                                   const uint32_t timeout_mS )
  {
    /*------------------------------------------------
    Start the asynchronous read
    ------------------------------------------------*/
    boost::asio::async_read_until( serialPort, boost::asio::dynamic_buffer( buffer ), expr,
                                   boost::bind( &SerialDriver::callback_readComplete,
                                                this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred ) );

    /*------------------------------------------------
    Initialize the timeout service
    ------------------------------------------------*/
    timer.expires_from_now( boost::posix_time::milliseconds( timeout_mS ) );
    timer.async_wait( boost::bind( &SerialDriver::callback_timeoutExpired, this, boost::asio::placeholders::error ) );

    /*------------------------------------------------
    Periodically grab updates from the io_service. Updates
    are propagated through the class callback_*() functions.
    ------------------------------------------------*/
    asyncResult      = Status::RX_IN_PROGRESS;
    bytesTransferred = 0;

    while ( asyncResult == Status::RX_IN_PROGRESS )
    {
      io.run_one();

      switch ( asyncResult )
      {
        case Status::RX_COMPLETE:
          timer.cancel();
          break;

        case Status::GENERIC_ERROR:
          timer.cancel();
          serialPort.cancel();
          break;

        case Status::TIMEOUT:
          serialPort.cancel();
          break;

        case Status::RX_IN_PROGRESS:
        default:
          break;
      }
    }

    return asyncResult;
  }

  Chimera::Serial::Status SerialDriver::open()
  {
    Status error = Status::OK;

    try
    {
      if ( !serialPort.is_open() )
      {
        serialPort.open( serialDevice );
      }
    }
    catch ( const boost::system::system_error & )
    {
      error = Status::FAILED_OPEN;
    }

    return error;
  }

  void SerialDriver::callback_readComplete( const boost::system::error_code &error, const size_t bytesTransferred )
  {
    asyncResult = Status::GENERIC_ERROR;

    if ( !error )
    {
      asyncResult            = Status::RX_COMPLETE;
      this->bytesTransferred = bytesTransferred;
      return;
    }
  }

  void SerialDriver::callback_timeoutExpired( const boost::system::error_code &error )
  {
    if ( !error && asyncResult == Status::RX_IN_PROGRESS )
    {
      asyncResult = Status::TIMEOUT;
    }
  }

}    // namespace HWInterface
