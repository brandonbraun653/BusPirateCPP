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


namespace BusPirate
{
  SerialDriver::SerialDriver( std::string &comPort )
  {
  }

  Chimera::Serial::Status SerialDriver::begin( const uint32_t baud, const Chimera::Serial::Modes txMode,
                                               const Chimera::Serial::Modes rxMode )
  {
    throw std::logic_error( "The method or operation is not implemented." );
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
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::Serial::Status SerialDriver::read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

}
