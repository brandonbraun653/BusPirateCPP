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

using namespace Chimera;
using namespace Chimera::GPIO;
using namespace Chimera::SPI;

namespace HWInterface
{
  namespace BusPirate
  {


    static constexpr uint8_t CMD_ENTER_RAW_SPI   = 0x01;
    static constexpr uint8_t CMD_SET_CS_LOW      = 0x02;
    static constexpr uint8_t CMD_SET_CS_HIGH     = 0x03;
    static constexpr uint8_t CMD_WRITE_THEN_READ = 0x04;

    static constexpr uint8_t CMD_BULK_SPI_TXFR = 0x10;
    static constexpr uint8_t MSK_BULK_SPI_TXFR_BYTES = 0x0F;

    /*------------------------------------------------
    Board Configuration Options
    ------------------------------------------------*/
    static constexpr uint8_t CMD_CFG_PERIPH     = 0x40;
    static constexpr uint8_t MSK_CFG_PERIPH     = 0x0F;
    static constexpr uint8_t CFG_PERIPH_POWER   = ( 1u << 3 );
    static constexpr uint8_t CFG_PERIPH_PULLUP  = ( 1u << 2 );
    static constexpr uint8_t CFG_PERIPH_AUX_PIN = ( 1u << 1 );
    static constexpr uint8_t CFG_PERIPH_CS_PIN  = ( 1u << 0 );

    /*------------------------------------------------
    SPI Speed Configuration Options
    ------------------------------------------------*/
    static constexpr uint8_t CMD_CFG_SPEED = 0x60;
    static constexpr uint8_t MSK_CFG_SPEED = 0x07;

    enum SpiSpeed
    {
      SPEED_30kHz = 0,
      SPEED_125kHz,
      SPEED_250kHz,
      SPEED_1MHz,
      SPEED_2MHz,
      SPEED_2_6MHz,
      SPEED_4MHz,
      SPEED_8MHz
    };

    /*------------------------------------------------
    SPI Configuration Options
    ------------------------------------------------*/
    static constexpr uint8_t CMD_CFG_SPI = 0x80;
    static constexpr uint8_t MSK_CFG_SPI = 0x0F;

    static constexpr uint8_t CFG_SPI_PIN_3V3 = (1u << 3 );
    static constexpr uint8_t CFG_SPI_PIN_HIZ = ~CFG_SPI_PIN_3V3;

    static constexpr uint8_t CFG_SPI_CPOL_1 = ( 1u << 2);
    static constexpr uint8_t CFG_SPI_CPOL_0 = ~CFG_SPI_CPOL_1;

    static constexpr uint8_t CFG_SPI_CPHA_ACT_TO_IDLE = ( 1u << 1 );
    static constexpr uint8_t CFG_SPI_CPHA_IDLE_TO_ACT = ~CFG_SPI_CPHA_ACT_TO_IDLE;


    BinarySPI::BinarySPI( Device &device ) : busPirate( device )
    {

    }

    Chimera::Status_t BinarySPI::init( const Chimera::SPI::Setup &setupStruct ) noexcept
    {
      Chimera::Status_t result = SPI::Status::NOT_INITIALIZED;

      if (busPirate.bbInit())
      {
        result |= cfgSPIPinOut( true );
        result |= cfgPullups(true);
        result |= cfgChipSelect(true);
        result |= setChipSelect( State::HI );
        result |= cfgPowerSupplies(false);

        switch (setupStruct.clockMode)
        {
          case ClockMode::MODE0:
            result |= cfgSPIClkIdle( false );
            result |= cfgSPIClkEdge( true );
            break;

          case ClockMode::MODE1:
            result |= cfgSPIClkIdle( false );
            result |= cfgSPIClkEdge( false );
            break;

          case ClockMode::MODE2:
            result |= cfgSPIClkIdle( true );
            result |= cfgSPIClkEdge( true );
            break;

          case ClockMode::MODE3:
            result |= cfgSPIClkIdle( true );
            result |= cfgSPIClkEdge( false );
            break;

          default:
            break;
        }

        result |= setClockFrequency(setupStruct.clockFrequency);
      }

      return result;
    }

    Chimera::Status_t BinarySPI::setChipSelect( const Chimera::GPIO::State &value ) noexcept
    {
      Chimera::Status_t result = SPI::Status::FAILED_CHIP_SELECT_WRITE;

      uint8_t command = 0;
      if (static_cast<bool>(value))
      {
        command = CMD_SET_CS_HIGH;
      }
      else
      {
        command = CMD_SET_CS_LOW;
      }

      std::vector<uint8_t> cmd = { command };
      auto rx = busPirate.sendResponsiveCommand(cmd);

      // TODO: This might be a bug. I think it only returns one byte
      if (std::find(rx.begin(), rx.end(), BitBangCommands::success) != rx.end())
      {
        result = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::setChipSelectControlMode( const Chimera::SPI::ChipSelectMode &mode ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::writeBytes( const uint8_t *const txBuffer, size_t length, const bool &disableCS /*= true*/,
                                         const bool &autoRelease /*= false*/, uint32_t timeoutMS /*= 10*/ ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::readBytes( uint8_t *const rxBuffer, size_t length, const bool &disableCS /*= true*/,
                                        const bool &autoRelease /*= false*/, uint32_t timeoutMS /*= 10*/ ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::readWriteBytes( const uint8_t *const txBuffer, uint8_t *const rxBuffer, size_t length,
                                             const bool &disableCS /*= true*/, const bool &autoRelease /*= false*/,
                                             uint32_t timeoutMS /*= 10*/ ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::setPeripheralMode( const Chimera::SPI::SubPeripheral &periph,
                                                const Chimera::SPI::SubPeripheralMode &mode ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::setClockFrequency( const uint32_t &freq ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::getClockFrequency( uint32_t *const freq ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::cfgPowerSupplies( const bool state )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::cfgPullups( const bool state )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::cfgAuxPin( const bool state )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::cfgChipSelect( const bool state )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::cfgSPIPinOut( const bool state )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::cfgSPIClkIdle( const bool state )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::cfgSPIClkEdge( const bool direction )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

  }  // namespace BusPirate
}  // namespace HWInterface
