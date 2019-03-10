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
    static constexpr uint8_t CMD_ENTER_RAW_SPI = 0x01;

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

    // clang-format off
    using SpeedBimap_t = boost::bimap<uint32_t, uint8_t>;
    static const SpeedBimap_t mapSpeedtoBits = boost::assign::list_of<SpeedBimap_t::relation>
      ( SPEED_30kHz,  0x00 )
      ( SPEED_125kHz, 0x01 )
      ( SPEED_250kHz, 0x02 )
      ( SPEED_1MHz,   0x03 )
      ( SPEED_2MHz,   0x04 )
      ( SPEED_2_6MHz, 0x05 )
      ( SPEED_4MHz,   0x06 )
      ( SPEED_8MHz,   0x07 )
      ( SPEED_NOT_SUPPORTED, 0x00 );
    // clang-format on

    /*------------------------------------------------
    SPI Write/Read Commands
    ------------------------------------------------*/
    static constexpr uint8_t CMD_BULK_SPI_TXFR       = 0x10;
    static constexpr uint8_t MSK_BULK_SPI_TXFR_BYTES = 0x0F;
    static constexpr uint8_t CMD_TX_THEN_RX_MAN_CS   = 0x05;
    static constexpr uint8_t CMD_TX_THEN_RX_AUTO_CS  = 0x04;


    BinarySPI::BinarySPI( Device &device ) : busPirate( device )
    {
      busPirate.open();
      systemInitialized = false;
      csMode            = Chimera::SPI::ChipSelectMode::AUTO_AFTER_TRANSFER;

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
        /*------------------------------------------------
        Default power off the supplies as we don't want to accidentally damage something
        ------------------------------------------------*/
        result = cfgPowerSupplies( true );

        /*------------------------------------------------
        Enable all the pins as outputs
        ------------------------------------------------*/
        result |= cfgSPIPinOut( true );

        /*------------------------------------------------
        Disable the pullups to false, otherwise signals will propagate from MOSI
        into MISO when there is no load, falsely indicating a response. This software
        was developed on v3.6 hardware and the schematics seem to indicate that
        enabling the pullups (IC3) requires an external voltage source (VEXTERN).
        Without this, it will tie together the SPI signals via 10k resistors and
        therefore generate a signal on MISO with nothing connected.
        ------------------------------------------------*/
        result |= cfgPullups( false );

        /*------------------------------------------------
        The chip select line follows whatever configuration is set by cfgSPIPinOut.
        ------------------------------------------------*/
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

        /*------------------------------------------------
        Attempts to get the closest supported match to what the user inputted.
        ------------------------------------------------*/
        auto tmp = setClockFrequency( setupStruct.clockFrequency, 0 );
        if ( tmp == SPI::Status::CLOCK_SET_EQ || tmp == SPI::Status::CLOCK_SET_LT )
        {
          result |= SPI::Status::OK;
          spdlog::info( "SPI initialized" );
        }
        else
        {
          result |= SPI::Status::FAIL;
          spdlog::error( "Failed SPI initialization" );
        }

        if ( result == SPI::Status::OK )
        {
          systemInitialized = true;
        }
      }

      return result;
    }

    Chimera::Status_t BinarySPI::deInit() noexcept
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      busPirate.close();
      result            = SPI::Status::OK;
      systemInitialized = false;

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
      csMode = mode;
      return SPI::Status::OK;
    }

    Chimera::Status_t BinarySPI::writeBytes( const uint8_t *const txBuffer, const size_t length,
                                             const uint32_t timeoutMS ) noexcept
    {
      if ( !txBuffer || !length )
      {
        return SPI::Status::INVAL_FUNC_PARAM;
      }

      TXRXPacket_t transfer;

      transfer.command       = CMD_BULK_SPI_TXFR;
      transfer.numWriteBytes = static_cast<uint16_t>( length );
      transfer.numReadBytes  = static_cast<uint16_t>( length );
      transfer.writeData     = std::vector<uint8_t>( txBuffer, txBuffer + length );

      return bulkTransfer( transfer );
    }

    Chimera::Status_t BinarySPI::readBytes( uint8_t *const rxBuffer, const size_t length, const uint32_t timeoutMS ) noexcept
    {
      if ( !rxBuffer || !length )
      {
        return SPI::Status::INVAL_FUNC_PARAM;
      }

      Chimera::Status_t result = SPI::Status::FAIL;
      TXRXPacket_t transfer;

      transfer.command       = CMD_BULK_SPI_TXFR;
      transfer.numWriteBytes = static_cast<uint16_t>( length );
      transfer.numReadBytes  = static_cast<uint16_t>( length );
      transfer.writeData     = std::vector<uint8_t>( length );

      transfer.writeData.assign( length, 0 );

      if ( bulkTransfer( transfer ) == SPI::Status::OK )
      {
        auto rxLen = std::min( length, transfer.readData.size() );
        memcpy( rxBuffer, transfer.readData.data(), rxLen );
        result = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::readWriteBytes( const uint8_t *const txBuffer, uint8_t *const rxBuffer, const size_t length,
                                                 const uint32_t timeoutMS ) noexcept
    {
      if ( !txBuffer || !rxBuffer || !length )
      {
        return SPI::Status::INVAL_FUNC_PARAM;
      }

      Chimera::Status_t result = SPI::Status::FAIL;
      TXRXPacket_t transfer;

      transfer.command       = CMD_BULK_SPI_TXFR;
      transfer.numWriteBytes = static_cast<uint16_t>( length );
      transfer.numReadBytes  = static_cast<uint16_t>( length );
      transfer.writeData     = std::vector<uint8_t>( txBuffer, txBuffer + length );

      if ( bulkTransfer( transfer ) == SPI::Status::OK )
      {
        auto rxLen = std::min( length, transfer.readData.size() );
        memcpy( rxBuffer, transfer.readData.data(), rxLen );
        result = SPI::Status::OK;
      }

      return result;
    }

    Chimera::Status_t BinarySPI::setPeripheralMode( const Chimera::SPI::SubPeripheral periph,
                                                    const Chimera::SPI::SubPeripheralMode mode ) noexcept
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::setClockFrequency( const uint32_t freq, const uint32_t tolerance ) noexcept
    {
      Chimera::Status_t result = SPI::Status::FAIL;

      /*------------------------------------------------
      Build the command
      ------------------------------------------------*/
      auto iter = std::lower_bound( sortedSPISpeeds.begin(), sortedSPISpeeds.end(), freq );

      /* Reset to the lowest speed if the requested value is not found */
      if ( iter == sortedSPISpeeds.end() )
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
        if ( actualClock < freq )
        {
          result = SPI::Status::CLOCK_SET_LT;
        }
      }

      return result;
    }

    Chimera::Status_t BinarySPI::getClockFrequency( uint32_t &freq ) noexcept
    {
      Chimera::Status_t result = SPI::Status::OK;

      freq = mapSpeedtoBits.right.find( reg_SPISpeed )->second;

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

    Chimera::Status_t BinarySPI::reserve( const uint32_t timeout_ms )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::release( const uint32_t timeout_ms )
    {
      return SPI::Status::NOT_SUPPORTED;
    }

    Chimera::Status_t BinarySPI::bulkTransfer( TXRXPacket_t &transfer )
    {
      Chimera::Status_t result = SPI::Status::FAIL;
      std::vector<uint8_t> data;
      std::vector<uint8_t> output;

      if ( csMode != ChipSelectMode::MANUAL )
      {
        setChipSelect( Chimera::GPIO::State::LOW );
      }

      /*------------------------------------------------
      Let the Bus Pirate know how many bytes we want to transfer
      //TODO: Support "unlimited" transfers by breaking it into pieces
      //TODO: Add the option for AUTO_BETWEEN_TRANSFER
      ------------------------------------------------*/
      uint8_t command = transfer.command | ( ( transfer.numWriteBytes - 1 ) & MSK_BULK_SPI_TXFR_BYTES );
      data            = std::vector<uint8_t>{ command };
      auto out        = busPirate.sendResponsiveCommand( data, 1 );

      /*------------------------------------------------
      If BP responds with a success, it's prepared to do the transfer. Send the data.
      ------------------------------------------------*/
      if ( out.size() && out[ 0 ] == BitBangCommands::success )
      {
        output = busPirate.sendResponsiveCommand( transfer.writeData, static_cast<uint32_t>( transfer.writeData.size() ) );

        if ( output.size() )
        {
          transfer.readData = output;
          result            = SPI::Status::OK;
        }
      }

      /*------------------------------------------------
      Any state besides manual must disable the chip select line after the transfer
      ------------------------------------------------*/
      if ( csMode != ChipSelectMode::MANUAL )
      {
        setChipSelect( Chimera::GPIO::State::HIGH );
      }

      return result;
    }

    Chimera::Status_t BinarySPI::writeThenRead( TXRXPacket_t &transfer )
    {
      Chimera::Status_t result = SPI::Status::FAIL;
      std::vector<uint8_t> data;

      /*------------------------------------------------
      Send the command preamble
      ------------------------------------------------*/
      data = std::vector<uint8_t>{ transfer.command, static_cast<uint8_t>( ( transfer.numWriteBytes >> 8 ) & 0xFF ),
                                   static_cast<uint8_t>( ( transfer.numWriteBytes & 0xFF ) ),
                                   static_cast<uint8_t>( ( transfer.numReadBytes >> 8 ) & 0xFF ),
                                   static_cast<uint8_t>( ( transfer.numReadBytes & 0xFF ) ) };
      busPirate.sendCommand( data );

      /*------------------------------------------------
      Send the actual data
      ------------------------------------------------*/
      auto out = busPirate.sendResponsiveCommand( transfer.writeData, transfer.numReadBytes + 1 );

      if ( out.size() && out[ 0 ] == BitBangCommands::success )
      {
        /* Remove the success byte from the returned data (0x01) */
        transfer.readData = std::vector<uint8_t>( out.begin() + 1, out.end() );
      }

      return result;
    }

  }  // namespace BusPirate
}  // namespace HWInterface
