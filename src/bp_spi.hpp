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
    /**
     *  Supported Bus Pirate SPI Speeds
     */
    enum SpiSpeed
    {
      SPEED_30kHz         = 30000,
      SPEED_125kHz        = 125000,
      SPEED_250kHz        = 250000,
      SPEED_1MHz          = 1000000,
      SPEED_2MHz          = 2000000,
      SPEED_2_6MHz        = 2600000,
      SPEED_4MHz          = 4000000,
      SPEED_8MHz          = 8000000,
      SPEED_NOT_SUPPORTED = 9999999,
    };

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

      Chimera::Status_t deInit() noexcept override;

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

      Chimera::Status_t reserve( const uint32_t &timeout_ms = 0u ) override;

      Chimera::Status_t release( const uint32_t &timeout_ms = 0u ) override;

      /**
       *	Enables or disables the on-board power supplies
       *
       *	@param[in]	state         Enabled (true), disabled (false)
       *	@return Chimera::Status_t
       */
      Chimera::Status_t cfgPowerSupplies( const bool state );

      /**
       *	Enables or disables pullups on all pins
       *
       *	@param[in]	state         Enabled (true), disabled (false)
       *	@return Chimera::Status_t
       */
      Chimera::Status_t cfgPullups( const bool state );

      /**
       *	Enables or disables the Auxiliary pin
       *
       *	@param[in]	state         Enabled (true), disabled (false)
       *	@return Chimera::Status_t
       */
      Chimera::Status_t cfgAuxPin( const bool state );

      /**
       *	Enables or disables the chip select pin. This behavior will follow
       *  whatever the current HiZ configuration is for the device.
       *
       *	@param[in]	state         Enabled (true), disabled (false)
       *	@return Chimera::Status_t
       */
      Chimera::Status_t cfgChipSelect( const bool state );

      /**
       *	Enables or disables the SPI output pins
       *
       *	@param[in]	state     True (3.3V), false (HiZ)
       *	@return Chimera::Status_t
       */
      Chimera::Status_t cfgSPIPinOut( const bool state );

      /**
       *	Set the idle state of the clock signal, high or low
       *
       *	@param[in]	state     Logical idle state of the clock
       *	@return Chimera::Status_t
       */
      Chimera::Status_t cfgSPIClkIdle( const bool state );

      /**
       *	Set which edge the clock should be valid on.
       *
       *	@param[in]	direction     Idle to Active (false), Active to Idle (true)
       *	@return Chimera::Status_t
       */
      Chimera::Status_t cfgSPIClkEdge( const bool direction );

    protected:
    private:
      Device &busPirate;
      Chimera::SPI::ChipSelectMode csMode;

      bool systemInitialized;

      uint8_t reg_PeriphCfg;
      uint8_t reg_SPICfg;
      uint8_t reg_CS;
      uint8_t reg_SPISpeed;

      struct TXRXPacket_t
      {
        uint8_t command;
        uint16_t numWriteBytes;
        uint16_t numReadBytes;
        std::vector<uint8_t> writeData;
        std::vector<uint8_t> readData;
      };

      Chimera::Status_t bulkTransfer( TXRXPacket_t &transfer );

      Chimera::Status_t writeThenRead( TXRXPacket_t &transfer );
    };

  }  // namespace BusPirate
}  // namespace HWInterface

#endif /* !BUS_PIRATE_CPP_SPI_DRIVER_HPP */
