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

using namespace Chimera::SPI;

namespace HWInterface
{
  namespace BusPirate
  {
    BinarySPI::BinarySPI( Device &device ) : busPirate( device )
    {
      
    }

    Chimera::Status_t BinarySPI::init( const Chimera::SPI::Setup &setupStruct ) noexcept
    {
      /*------------------------------------------------
      We don't have control over the physical GPIO pins, so the only parts that
      matter with the setupStruct is the peripheral configuration settings.
      ------------------------------------------------*/

      return Status::OK;
    }

    Chimera::Status_t BinarySPI::setChipSelect( const Chimera::GPIO::State &value ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::setChipSelectControlMode( const Chimera::SPI::ChipSelectMode &mode ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::writeBytes( const uint8_t *const txBuffer, size_t length, const bool &disableCS /*= true*/,
                                         const bool &autoRelease /*= false*/, uint32_t timeoutMS /*= 10*/ ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::readBytes( uint8_t *const rxBuffer, size_t length, const bool &disableCS /*= true*/,
                                        const bool &autoRelease /*= false*/, uint32_t timeoutMS /*= 10*/ ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::readWriteBytes( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                             const bool &disableCS /*= true*/, const bool &autoRelease /*= false*/,
                                             uint32_t timeoutMS /*= 10*/ ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::setPeripheralMode( const Chimera::SPI::SubPeripheral &periph,
                                                const Chimera::SPI::SubPeripheralMode &mode ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::setClockFrequency( const uint32_t &freq ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::getClockFrequency( uint32_t *const freq ) noexcept
    {
      return Status::NOT_SUPPORTED;
    }
  }  // namespace BusPirate
}  // namespace HWInterface
