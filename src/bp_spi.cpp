/********************************************************************************
 *   File Name:
 *       bp_spi.cpp
 *
 *   Description:
 *       Implements the SPI interface to the Bus Pirate hardware
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Driver Includes */
#include "bp_spi.hpp"

/* C++ Includes */
#include <map>
#include <string>
#include <algorithm>

/* Boost Includes */
#include <boost/bimap.hpp>
#include <boost/assign.hpp>

/* Library Includes */
#include <spdlog/spdlog.h>

using namespace Chimera;
using namespace Chimera::GPIO;
using namespace Chimera::SPI;

namespace HWInterface
{
  namespace BusPirate
  {
    static constexpr uint8_t CMD_ENTER_RAW_SPI   = 0x01;
    static constexpr uint8_t CMD_WRITE_THEN_READ = 0x04;

    static constexpr uint8_t CMD_BULK_SPI_TXFR       = 0x10;
    static constexpr uint8_t MSK_BULK_SPI_TXFR_BYTES = 0x0F;

    /*------------------------------------------------
    Chip Select Options
    ------------------------------------------------*/
    static constexpr uint8_t CMD_SET_CS = 0x02;
    static constexpr uint8_t MSK_SET_CS = 0x01;
    static constexpr uint8_t SET_CS     = ( 1u << 0 );

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
    SPI Configuration Options
    ------------------------------------------------*/
    static constexpr uint8_t CMD_CFG_SPI = 0x80;
    static constexpr uint8_t MSK_CFG_SPI = 0x0F;

    static constexpr uint8_t CFG_SPI_PIN_3V3 = ( 1u << 3 );
    static constexpr uint8_t CFG_SPI_PIN_HIZ = ~CFG_SPI_PIN_3V3;

    static constexpr uint8_t CFG_SPI_CPOL_1 = ( 1u << 2 );
    static constexpr uint8_t CFG_SPI_CPOL_0 = ~CFG_SPI_CPOL_1;

    static constexpr uint8_t CFG_SPI_CPHA_ACT_TO_IDLE = ( 1u << 1 );
    static constexpr uint8_t CFG_SPI_CPHA_IDLE_TO_ACT = ~CFG_SPI_CPHA_ACT_TO_IDLE;

    /*------------------------------------------------
    SPI Speed Configuration & Lookup Options
    ------------------------------------------------*/
    static constexpr uint8_t CMD_CFG_SPEED = 0x60;
    static constexpr uint8_t MSK_CFG_SPEED = 0x07;

    static constexpr std::array<uint32_t, 9> sortedSPISpeeds = { SPEED_30kHz, SPEED_125kHz, SPEED_250kHz,
                                                                 SPEED_1MHz,  SPEED_2MHz,   SPEED_2_6MHz,
                                                                 SPEED_4MHz,  SPEED_8MHz,   SPEED_NOT_SUPPORTED };

    using SpeedBimap_t                       = boost::bimap<uint32_t, uint8_t>;
    static const SpeedBimap_t mapSpeedtoBits = boost::assign::list_of<SpeedBimap_t::relation>( SPEED_30kHz, 0x00 )(
        SPEED_125kHz, 0x01 )( SPEED_250kHz, 0x02 )( SPEED_1MHz, 0x03 )( SPEED_2MHz, 0x04 )( SPEED_2_6MHz, 0x05 )(
        SPEED_4MHz, 0x06 )( SPEED_8MHz, 0x07 )( SPEED_NOT_SUPPORTED, 0x00 );

    BinarySPI::BinarySPI( Device &device ) : busPirate( device )
    {
      busPirate.open();
      systemInitialized = false;

      /*------------------------------------------------
      Initialize the virtual registers
      ------------------------------------------------*/
      reg_CS        = 0;
      reg_SPICfg    = 0;
      reg_PeriphCfg = 0;
      reg_SPISpeed  = 0;
    }

    Chimera::Status_t BinarySPI::init( const Chimera::SPI::Setup &setupStruct ) noexcept
    {
      Chimera::Status_t result = SPI::Status::NOT_INITIALIZED;

      if ( busPirate.bbInit() && busPirate.bbEnterSPI() )
      {
        result = cfgPowerSupplies( true );

        result |= cfgSPIPinOut( true );
        result |= cfgPullups( true );
        result |= cfgChipSelect( true );
        result |= setChipSelect( State::HI );

        switch ( setupStruct.clockMode )
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

        auto tmp = setClockFrequency( setupStruct.clockFrequency );
        if (tmp == SPI::Status::CLOCK_SET_EQ || tmp == SPI::Status::CLOCK_SET_LT)
        {
          result |= SPI::Status::OK;
        }
        else
        {
          result |= SPI::Status::FAIL;
        }

        if (result == SPI::Status::OK)
        {
          systemInitialized = true;
        }
      }

      return result;
    }

    Chimera::Status_t BinarySPI::deInit() noexcept
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      if ( busPirate.reset() )
      {
        busPirate.close();
        result = SPI::Status::OK;
        systemInitialized = false;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::setChipSelect( const Chimera::GPIO::State &value ) noexcept
    {
      Chimera::Status_t result = SPI::Status::FAILED_CHIP_SELECT_WRITE;

      uint8_t bitVals = reg_CS;
      if ( static_cast<bool>( value ) )
      {
        bitVals |= SET_CS;
      }
      else
      {
        bitVals &= ~SET_CS;
      }

      uint8_t command = CMD_SET_CS | ( bitVals & MSK_SET_CS );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_CS = bitVals;
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
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      auto iter = std::lower_bound( sortedSPISpeeds.begin(), sortedSPISpeeds.end(), freq );

      /* Reset to the lowest speed if the requested value is not found */
      if (iter == sortedSPISpeeds.end())
      {
        iter = sortedSPISpeeds.begin();
      }

      auto bitVals = mapSpeedtoBits.left.find( *iter )->second;

      uint8_t command = CMD_CFG_SPEED | ( bitVals & MSK_CFG_SPEED );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_SPISpeed = bitVals;

        uint32_t actualClock = mapSpeedtoBits.right.find( reg_SPISpeed )->second;

        result = SPI::Status::CLOCK_SET_EQ;
        if (actualClock < freq)
        {
          result = SPI::Status::CLOCK_SET_LT;
        }
      }

      return result;
    }

    Chimera::Status_t BinarySPI::getClockFrequency( uint32_t *const freq ) noexcept
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      if (freq)
      {
        *freq = mapSpeedtoBits.right.find( reg_SPISpeed )->second;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::cfgPowerSupplies( const bool state )
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      uint8_t bitVals = reg_PeriphCfg;
      if ( state )
      {
        bitVals |= CFG_PERIPH_POWER;
      }
      else
      {
        bitVals &= ~CFG_PERIPH_POWER;
      }

      uint8_t command = CMD_CFG_PERIPH | ( bitVals & MSK_CFG_PERIPH );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_PeriphCfg = bitVals;
        result        = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::cfgPullups( const bool state )
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      uint8_t bitVals = reg_PeriphCfg;
      if ( state )
      {
        bitVals |= CFG_PERIPH_PULLUP;
      }
      else
      {
        bitVals &= ~CFG_PERIPH_PULLUP;
      }

      uint8_t command = CMD_CFG_PERIPH | ( bitVals & MSK_CFG_PERIPH );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_PeriphCfg = bitVals;
        result        = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::cfgAuxPin( const bool state )
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      uint8_t bitVals = reg_PeriphCfg;
      if ( state )
      {
        bitVals |= CFG_PERIPH_AUX_PIN;
      }
      else
      {
        bitVals &= ~CFG_PERIPH_AUX_PIN;
      }

      uint8_t command = CMD_CFG_PERIPH | ( bitVals & MSK_CFG_PERIPH );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_PeriphCfg = bitVals;
        result        = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::cfgChipSelect( const bool state )
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      uint8_t bitVals = reg_PeriphCfg;
      if ( state )
      {
        bitVals |= CFG_PERIPH_CS_PIN;
      }
      else
      {
        bitVals &= ~CFG_PERIPH_CS_PIN;
      }

      uint8_t command = CMD_CFG_PERIPH | ( bitVals & MSK_CFG_PERIPH );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_PeriphCfg = bitVals;
        result        = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::cfgSPIPinOut( const bool state )
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      uint8_t bitVals = reg_SPICfg;
      if ( state )
      {
        bitVals |= CFG_SPI_PIN_3V3;
      }
      else
      {
        bitVals &= CFG_SPI_PIN_HIZ;
      }

      uint8_t command = CMD_CFG_SPI | ( bitVals & MSK_CFG_SPI );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_SPICfg = bitVals;
        result     = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::cfgSPIClkIdle( const bool state )
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      uint8_t bitVals = reg_SPICfg;
      if ( state )
      {
        bitVals |= CFG_SPI_CPOL_1;
      }
      else
      {
        bitVals &= CFG_SPI_CPOL_0;
      }

      uint8_t command = CMD_CFG_SPI | ( bitVals & MSK_CFG_SPI );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_SPICfg = bitVals;
        result     = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::cfgSPIClkEdge( const bool direction )
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      uint8_t bitVals = reg_SPICfg;
      if ( direction )
      {
        bitVals |= CFG_SPI_CPHA_ACT_TO_IDLE;
      }
      else
      {
        bitVals &= CFG_SPI_CPHA_IDLE_TO_ACT;
      }

      uint8_t command = CMD_CFG_SPI | ( bitVals & MSK_CFG_SPI );

      /*------------------------------------------------
      Send and verify
      ------------------------------------------------*/
      std::vector<uint8_t> cmd = { command };
      auto rx                  = busPirate.sendResponsiveCommand( cmd, static_cast<uint32_t>( cmd.size() ) );

      if ( rx.size() && rx[ 0 ] == BitBangCommands::success )
      {
        reg_SPICfg = bitVals;
        result     = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::reserve( const uint32_t &timeout_ms /*= 0u */ )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::release( const uint32_t &timeout_ms /*= 0u */ )
    {
      return SPI::Status::NOT_SUPPORTED;
    }
  }  // namespace BusPirate
}  // namespace HWInterface
