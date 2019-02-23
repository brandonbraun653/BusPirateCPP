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

/* Boost Includes */
#include <boost/bind.hpp>
#include <boost/asio/serial_port_base.hpp>

using namespace Chimera::Serial;

namespace HWInterface
{
  SerialDriver::SerialDriver( std::string &device )
  {
    timer  = std::make_unique<boost::asio::deadline_timer>( ioService );
    serial = std::make_unique<boost::asio::serial_port>( ioService );

    serialDevice = device;
  }

  Chimera::Serial::Status SerialDriver::begin( const Chimera::Serial::Modes txMode, const Chimera::Serial::Modes rxMode )
  {
    Status error = Status::OK;
    boost::system::error_code hw_err;

    /*------------------------------------------------
    Configure to requested parameters. RX and TX mode are ignored as
    they are handled by the OS.
    ------------------------------------------------*/
    error = open();

    return error;
  }

  Chimera::Serial::Status SerialDriver::configure(
      const uint32_t baud, const CharWid width, const Parity parity, const StopBits stop, const FlowControl flow )
  {
    using namespace boost::asio;

    Status error = Status::OK;

    if ( !serial )
    {
      error = Status::NOT_INITIALIZED;
    }
    else
    {
      /*------------------------------------------------
      System Baud Rate
      ------------------------------------------------*/
      boost::asio::serial_port_base::baud_rate BAUD( baud );
      serial->set_option( BAUD );

      /*------------------------------------------------
      Data Width
      ------------------------------------------------*/
      boost::asio::serial_port_base::character_size CHAR( static_cast<uint8_t>( width ) );
      serial->set_option( CHAR );

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
      serial->set_option( PARITY );

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
      serial->set_option( STOP );

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
      serial->set_option( FLOW );
    }

    return error;
  }

  Chimera::Serial::Status SerialDriver::end()
  {
    Status error = Status::OK;

    serial->close();

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
    Status error     = Status::OK;
    auto asio_buffer = boost::asio::buffer( buffer, length );

    /*------------------------------------------------
    Open the device for reading
    ------------------------------------------------*/
    error = open();

    /*------------------------------------------------
    Execute the read operation
    ------------------------------------------------*/
    if ( error == Status::OK )
    {
      try
      {
        serial->write_some( asio_buffer );
      }
      catch ( const boost::system::system_error & )
      {
        error = Status::FAILED_WRITE;
      }
    }

    return error;
  }

  Chimera::Serial::Status SerialDriver::read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS )
  {
    Status error       = Status::OK;
    bool dataAvailable = false;
    auto asio_buffer   = boost::asio::buffer( buffer, length );
    auto read_callback = boost::bind( &SerialDriver::readCallback,
                                      this,
                                      boost::ref( dataAvailable ),
                                      boost::ref( timer ),
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred );

    auto wait_callback =
        boost::bind( &SerialDriver::waitCallback, this, boost::ref( serial ), boost::asio::placeholders::error );

    /*------------------------------------------------
    Open the device for reading
    ------------------------------------------------*/
    error = open();

    /*------------------------------------------------
    Execute the read operation
    ------------------------------------------------*/
    if ( error == Status::OK )
    {
      /*------------------------------------------------
      Set up the asynchronous read parameters
      ------------------------------------------------*/
      serial->async_read_some( asio_buffer, read_callback );
      timer->expires_from_now( boost::posix_time::milliseconds( timeout_mS ) );
      timer->async_wait( wait_callback );

      /*------------------------------------------------
      Blocks until all data is read or the timeout expires.
      ------------------------------------------------*/
      ioService.run();

      if ( !dataAvailable )
      {
        error = Status::FAILED_READ;
      }
    }

    return error;
  }

  Chimera::Serial::Status SerialDriver::open()
  {
    Status error = Status::OK;

    try
    {
      if ( !serial->is_open() )
      {
        serial->open( serialDevice );
      }
    }
    catch ( const boost::system::system_error & )
    {
      error = Status::FAILED_OPEN;
    }

    return error;
  }


  void SerialDriver::readCallback( bool &data_available,
                                   DeadlineTimer_sPtr &timeout,
                                   const boost::system::error_code &error,
                                   std::size_t bytes_transferred )
  {
    if ( error || !bytes_transferred )
    {
      std::cout << "Serial read canceled" << std::endl;
      data_available = false;
      return;
    }

    std::cout << "Data read successfully, canceling timeout" << std::endl;
    timeout->cancel();    // will cause wait_callback to fire with an error
    data_available = true;
  }

  void SerialDriver::waitCallback( SerialPort_sPtr &serialPort, const boost::system::error_code &error )
  {
    if ( error )
    {
      // Data was read and this timeout was canceled
      std::cout << "Timeout canceled" << std::endl;
      return;
    }

    std::cout << "No data read, closing the serial port." << std::endl;
    serialPort->cancel();    // will cause read_callback to fire with an error
  }

}    // namespace HWInterface
