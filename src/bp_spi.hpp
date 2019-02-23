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

#include <Chimera/interface.hpp>

namespace HWInterface
{
  class BPSpi : public Chimera::SPI::Interface
  {
  public:
    Chimera::SPI::Status init( const Chimera::SPI::Setup &setupStruct ) override;


    Chimera::SPI::Status setChipSelect( const Chimera::GPIO::State &value ) override;


    Chimera::SPI::Status setChipSelectControlMode( const Chimera::SPI::ChipSelectMode &mode ) override;


    Chimera::SPI::Status writeBytes( const uint8_t *const txBuffer, size_t length, const bool &disableCS = true,
                                     const bool &autoRelease = false, uint32_t timeoutMS = 10 ) override;


    Chimera::SPI::Status readBytes( uint8_t *const rxBuffer, size_t length, const bool &disableCS = true,
                                    const bool &autoRelease = false, uint32_t timeoutMS = 10 ) override;


    Chimera::SPI::Status readWriteBytes( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                         const bool &disableCS = true, const bool &autoRelease = false,
                                         uint32_t timeoutMS = 10 ) override;


    Chimera::SPI::Status setPeripheralMode( const Chimera::SPI::SubPeripheral &periph,
                                            const Chimera::SPI::SubPeripheralMode &mode ) override;


    Chimera::SPI::Status setClockFrequency( const uint32_t &freq ) override;


    Chimera::SPI::Status getClockFrequency( uint32_t *const freq ) override;
  };
}

#endif /* !BUS_PIRATE_CPP_SPI_DRIVER_HPP */
