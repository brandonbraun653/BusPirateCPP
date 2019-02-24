/********************************************************************************
 *   File Name:
 *       bp_spi.hpp
 *
 *   Description:
 *       Provides an interface to the SPI hardware on the Bus Pirate. Conforms with
 *       the Chimera HAL SPI interface.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef BUS_PIRATE_CPP_SPI_DRIVER_HPP
#define BUS_PIRATE_CPP_SPI_DRIVER_HPP

/* Chimera Includes */
#include <Chimera/interface.hpp>

/* BusPirate Includes */
#include "bus_pirate.hpp"
  

namespace HWInterface
{
  namespace BusPirate
  {
    class BinarySPI : public Chimera::SPI::Interface
    {
    public:
      
      /**
       *  Primary constructor for creating the SPI interface
       *
       *  @param[in]  device    An instance of the low level hardware interface to the Bus Pirate
       */
      BinarySPI( Device &device );
      ~BinarySPI() = default;
      
      Chimera::Status_t init( const Chimera::SPI::Setup &setupStruct ) noexcept override;

      Chimera::Status_t setChipSelect( const Chimera::GPIO::State &value ) noexcept override;

      Chimera::Status_t setChipSelectControlMode( const Chimera::SPI::ChipSelectMode &mode ) noexcept override;

      Chimera::Status_t writeBytes( const uint8_t *const txBuffer, size_t length, const bool &disableCS = true,
                                    const bool &autoRelease = false, uint32_t timeoutMS = 10 ) noexcept override;

      Chimera::Status_t readBytes( uint8_t *const rxBuffer, size_t length, const bool &disableCS = true,
                                   const bool &autoRelease = false, uint32_t timeoutMS = 10 ) noexcept override;

      Chimera::Status_t readWriteBytes( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                        const bool &disableCS = true, const bool &autoRelease = false,
                                        uint32_t timeoutMS = 10 ) noexcept override;

      Chimera::Status_t setPeripheralMode( const Chimera::SPI::SubPeripheral &periph,
                                           const Chimera::SPI::SubPeripheralMode &mode ) noexcept override;

      Chimera::Status_t setClockFrequency( const uint32_t &freq ) noexcept override;

      Chimera::Status_t getClockFrequency( uint32_t *const freq ) noexcept override;

    private:
      Device &busPirate;
    };
  
  }
}  // namespace HWInterface

#endif /* !BUS_PIRATE_CPP_SPI_DRIVER_HPP */
