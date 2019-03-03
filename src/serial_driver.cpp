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
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/asio/serial_port_base.hpp>

/* Windows Includes */
#if defined( _WIN32 ) || defined( _WIN64 )
#include <Windows.h>
#include <Ntddser.h>
#endif

using namespace Chimera;
using namespace Chimera::Serial;

namespace HWInterface
{
  SerialDriver::SerialDriver( std::string &device, const uint32_t delay_mS ) : io(), serialPort( io ), timer( io )
  {
    serialDevice = device;
    ioDelay_mS = delay_mS;
  }

  Chimera::Status_t SerialDriver::begin( const Chimera::Serial::Modes txMode, const Chimera::Serial::Modes rxMode ) noexcept
  {
    return open();
  }

  Chimera::Status_t SerialDriver::configure( const uint32_t baud, const CharWid width, const Parity parity, const StopBits stop,
                                             const FlowControl flow ) noexcept
  {
    using namespace boost::asio;

    Status_t error = Status::OK;

    if ( !serialPort.is_open() )
    {
      error = Status::NOT_INITIALIZED;
    }
    else
    {
      try
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

        /*------------------------------------------------
        HW Timeouts
        ------------------------------------------------*/
        //#if defined(WIN32) || defined(WIN64)
        //SERIAL_TIMEOUTS timeout;
        //timeout.ReadTotalTimeoutConstant = 0x2c010000;
        //timeout.WriteTotalTimeoutConstant = 0x2c010000;
        //IOCTL_SERIAL_SET_TIMEOUTS(timeout);
        //#endif
      }
      catch ( const boost::system::system_error & )
      {
        error = Status::FAILED_CONFIGURE;
      }
    }

    return error;
  }

  Chimera::Status_t SerialDriver::end() noexcept
  {
    io.reset();
    timer.cancel();
    serialPort.cancel();
    serialPort.close();
    return Status::OK;
  }

  Chimera::Status_t SerialDriver::setBaud( const uint32_t buad ) noexcept
  {
    return Status::NOT_SUPPORTED;
  }

  Chimera::Status_t SerialDriver::setMode( const Chimera::Serial::SubPeripheral periph,
                                           const Chimera::Serial::Modes mode ) noexcept
  {
    return Status::NOT_SUPPORTED;
  }

  Chimera::Status_t SerialDriver::write( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS ) noexcept
  {
    boost::asio::write( serialPort, boost::asio::buffer( buffer, length ) );
    boost::this_thread::sleep_for( boost::chrono::milliseconds( ioDelay_mS ) );
    return Status::OK;
  }

  Chimera::Status_t SerialDriver::read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS ) noexcept
  {
    /*------------------------------------------------
    The io_service must be reset before calling run_one()
    <https://stackoverflow.com/questions/35643311/why-must-io-servicereset-be-called>
    ------------------------------------------------*/
    io.reset();

    /*------------------------------------------------
    Start the asynchronous read
    ------------------------------------------------*/
    boost::asio::async_read( serialPort, boost::asio::buffer( buffer, length ),
                             boost::bind( &SerialDriver::callback_readComplete, this, boost::asio::placeholders::error,
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

        case Status::UNKNOWN_ERROR:
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

  Chimera::Status_t SerialDriver::readUntil( std::vector<uint8_t> &buffer, const boost::regex &expr,
                                             const uint32_t timeout_mS ) noexcept
  {
    if( serialPort.is_open() )
    {
      /*------------------------------------------------
      The io_service must be reset before calling run_one()
      <https://stackoverflow.com/questions/35643311/why-must-io-servicereset-be-called>
      ------------------------------------------------*/
      io.reset();

      /*------------------------------------------------
      Start the asynchronous read
      ------------------------------------------------*/
      boost::asio::async_read_until( serialPort, inputStream, expr,
                                     boost::bind( &SerialDriver::callback_readComplete, this, boost::asio::placeholders::error,
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
            buffer.resize( inputStream.size(), 0 );
            boost::asio::buffer_copy( boost::asio::buffer( buffer ), inputStream.data() );
            inputStream.consume(inputStream.size());
            break;

          case Status::UNKNOWN_ERROR:
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

      /*------------------------------------------------
      Reset the asio async functionality
      ------------------------------------------------*/
      timer.cancel();
      serialPort.cancel();
    }
    else
    {
      asyncResult = Status::FAILED_READ;
    }


    return asyncResult;
  }

  bool SerialDriver::isOpen() noexcept
  {
    return serialPort.is_open();
  }

  bool SerialDriver::flush() noexcept
  {
    /*------------------------------------------------
    Clears the input stream effectively erasing the object's cache of old data
    ------------------------------------------------*/
    inputStream.consume(inputStream.size());

    serialPort.cancel();

    /*------------------------------------------------
    Platform specific serial port driver buffer clearing
    ------------------------------------------------*/
#if defined( _WIN32 ) || defined( _WIN64 )
    HANDLE hSerial = serialPort.lowest_layer().native_handle();
    return static_cast<bool>( PurgeComm( hSerial, ( PURGE_RXCLEAR | PURGE_TXCLEAR ) ) );
#else
    return false;
#endif
  }

  bool SerialDriver::reset() noexcept
  {
    serialPort.cancel();
    io.reset();

    return open();
  }

  Chimera::Status_t SerialDriver::open() noexcept
  {
    Status_t error = Status::OK;

    try
    {
      if ( !serialPort.is_open() )
      {
        serialPort.open( serialDevice );
      }
    }
    catch ( const boost::system::system_error &err )
    {
      std::cout << err.what() << std::endl;
      error = Status::FAILED_OPEN;
    }

    return error;
  }

  void SerialDriver::callback_readComplete( const boost::system::error_code &error, const size_t bytesTransferred ) noexcept
  {
    asyncResult = Status::UNKNOWN_ERROR;

    if ( !error )
    {
      asyncResult            = Status::RX_COMPLETE;
      this->bytesTransferred = bytesTransferred;
      return;
    }
    else
    {
      std::string err = error.message();
      std::cout << err << std::endl;
    }
  }

  void SerialDriver::callback_timeoutExpired( const boost::system::error_code &error ) noexcept
  {
    if ( !error && asyncResult == Status::RX_IN_PROGRESS )
    {
      asyncResult = Status::TIMEOUT;
    }
  }

}  // namespace HWInterface
