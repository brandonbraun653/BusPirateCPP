/********************************************************************************
*   File Name:
*       bp_spi.cpp
*
*   Description:
*       Implements the SPI interface to the Bus Pirate hardware
*
*   2019 | Brandon Braun | brandonbraun653@gmail.com
********************************************************************************/

#include "bp_spi.hpp"

namespace HWInterface
{
  Chimera::SPI::Status BPSpi::init( const Chimera::SPI::Setup &setupStruct )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::setChipSelect( const Chimera::GPIO::State &value )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::setChipSelectControlMode( const Chimera::SPI::ChipSelectMode &mode )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::writeBytes( const uint8_t *const txBuffer, size_t length, const bool &disableCS /*= true*/,
                                          const bool &autoRelease /*= false*/, uint32_t timeoutMS /*= 10*/ )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::readBytes( uint8_t *const rxBuffer, size_t length, const bool &disableCS /*= true*/,
                                         const bool &autoRelease /*= false*/, uint32_t timeoutMS /*= 10*/ )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::readWriteBytes( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                              const bool &disableCS /*= true*/, const bool &autoRelease /*= false*/,
                                              uint32_t timeoutMS /*= 10*/ )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::setPeripheralMode( const Chimera::SPI::SubPeripheral &periph,
                                                 const Chimera::SPI::SubPeripheralMode &mode )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::setClockFrequency( const uint32_t &freq )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

  Chimera::SPI::Status BPSpi::getClockFrequency( uint32_t *const freq )
  {
    throw std::logic_error( "The method or operation is not implemented." );
  }

}

