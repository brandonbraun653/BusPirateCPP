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

    static constexpr uint8_t CFG_SPI_SMP_MID = ~(1u << 0);




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

    bool BinarySPI::enterBinaryMode()
    {
      bool error = false;

      if (busPirate.isConnected())
      {
        
      }

      return error;
    }

  }  // namespace BusPirate
}  // namespace HWInterface
