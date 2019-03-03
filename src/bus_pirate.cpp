/********************************************************************************
 *   File Name:
 *      bus_pirate.cpp
 *
 *   Description:
 *      Implements the interface to the BusPirate test hardware
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* Driver Includes */
#include "bus_pirate.hpp"

/* Library Includes */
#include <strtk/strtk.hpp>

/* Chimera Includes */
#include <Chimera/serial.hpp>

/* C++ Includes */
#include <algorithm>
#include <deque>
#include <string>
#include <regex>

/* Boost Includes */
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

using namespace Chimera::Serial;


namespace HWInterface
{
  namespace BusPirate
  {
    /*------------------------------------------------
    Track which versions of BusPirates are known to work with this software.
    ------------------------------------------------*/
    static std::vector<std::string> knownBoardVer      = { "v3b" };
    static std::vector<std::string> knownFirmwareVer   = { "v5.10" };
    static std::vector<std::string> knownBootloaderVer = { "v4.4" };

    /*------------------------------------------------
    Version limits for various feature support
    ------------------------------------------------*/
    static constexpr uint32_t minResetFirmwareMajorVer = 2; /**< Minimum firmware version needed to support reset '#' command */

    /*------------------------------------------------
    The current supported Bus Pirate operational modes
    ------------------------------------------------*/
    // std::array<ModeTypeBase, 2> supportedModes = { HiZMode(), SPIMode() };
    //{ "HiZ", "1-WIRE", "UART", "I2C", "SPI", "JTAG", "RAW2WIRE", "RAW3WIRE", "PC KEYBOARD", "LCD" };

    Device::Device( std::string &devicePort )
    {
      serial = std::make_shared<SerialDriver>( devicePort );

      supportedModes.push_back( std::make_unique<HiZMode>() );
      supportedModes.push_back( std::make_unique<SPIMode>() );

      connectedToSerial = false;
      currentMode = OperationalModes::BP_INVALID_MODE;
    }

    bool Device::open()
    {
      bool opened = false;

      if ( !serial->isOpen() )
      {
        serial->begin();
        Chimera::Status_t error = serial->configure( 115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE,
                                                     FlowControl::FCTRL_NONE );

        /*------------------------------------------------
        This flag controls whether or not the Bus Pirate will work. All
        functions should be checking this variable before execution.
        ------------------------------------------------*/
        connectedToSerial = ( error == Status::OK );

        /*------------------------------------------------
        Immediately reset the board, which places the board into Terminal mode
        ------------------------------------------------*/
        if ( connectedToSerial && reset() )
        {
          opened = true;
          currentMode = OperationalModes::BP_MODE_HiZ;
        }
      }
      else
      {
        opened = true;
      }

      return opened;
    }

    void Device::close()
    {
      /*------------------------------------------------
      In-case the user is not powering off the board, disconnect in a HiZ state
      ------------------------------------------------*/
      reset();

      /*------------------------------------------------
      Power off low level platform serial driver
      ------------------------------------------------*/
      serial->flush();
      connectedToSerial = !( serial->end() == Status::OK );
    }

    bool Device::reset()
    {
      bool result = false;

      /*------------------------------------------------
      Send the Bit Bang reset command first. There is a weird quirk with the hardware where
      sending the Terminal reset first will cause a lock-up if you are in Bit Bang mode. Should
      this fail to reset the board, we are likely already in terminal mode.
      ------------------------------------------------*/
      serial->flush();
      std::vector<uint8_t> bbCmd = { BitBangCommands::reset };
      std::vector<uint8_t> bbOut = sendResponsiveCommand( bbCmd, 1 );

      if ( std::find( bbOut.begin(), bbOut.end(), BitBangCommands::success ) != bbOut.end() )
      {
        result = true;
      }
      else
      {
        /*------------------------------------------------
        Allow some breathing room for the hardware
        ------------------------------------------------*/
        Chimera::delayMilliseconds( 10 );

        /*------------------------------------------------
        Send the reset command
        ------------------------------------------------*/
        std::string cmd = MenuCommands::reset;
        std::string out = sendResponsiveCommand( cmd, boost::regex{ "(\r\n)" } );

        /*------------------------------------------------
        According to the docs, RESET should be printed if the command was received/executed
        successfully from terminal mode. Otherwise, any data at on the serial port indicates
        reset from Bit Bang mode.
        ------------------------------------------------*/
        if ( out.find( "RESET" ) != std::string::npos )
        {
          result = true;
        }
      }

      /*------------------------------------------------
      Leave the next call in a clean state
      ------------------------------------------------*/
      serial->flush();

      return result;
    }

    bool Device::connect()
    {
      /*------------------------------------------------
      Make sure that we can talk to the device correctly. Occasionally there will be old data in the system serial buffer that
      hasn't been cleared out yet. Boost does not provide a way to flush this, so the simple fix is just to try and read
      things out again.
      ------------------------------------------------*/
      if ( connectedToSerial && reset() )
      {
        for ( auto x = 0; x < MAX_CONNECT_ATTEMPTS; x++ )
        {
          if ( getInfo().isValid )
          {
            connectedToSerial = true;
            break;
          }
          else
          {
            std::cout << "Retrying connection..." << std::endl;
            boost::this_thread::sleep_for( boost::chrono::milliseconds( 500 ) );
          }
        }
      }

      return false;
    }

    bool Device::isConnected()
    {
      return connectedToSerial;
    }

    void Device::clearTerminal()
    {
      const std::string enter = MenuCommands::ping;
      for ( auto x = 0; x < 3; x++ )
      {
        sendCommand( enter );
      }
    }

    Info Device::getInfo()
    {
      Info info;

      if ( isConnected() && serial->flush() )
      {
        std::regex numberOnlyRegex = std::regex( R"([\D])" );
        std::string cmd            = MenuCommands::info;
        std::string rawOutput      = sendResponsiveCommand( cmd );
        auto split_opt             = strtk::split_options::compress_delimiters;

        /*------------------------------------------------
        Split the string by new lines
        ------------------------------------------------*/
        std::deque<std::string> tokenList;
        strtk::multiple_char_delimiter_predicate predicate( "\r\n" );
        strtk::split( predicate, rawOutput, strtk::range_to_type_back_inserter( tokenList ), split_opt );

        if ( tokenList.size() )
        {
          /*------------------------------------------------
          Pull out the Board version
          ------------------------------------------------*/
          /* Get the version as a string */
          std::string line1 = tokenList.front();
          std::deque<std::string> l1Tokens;
          strtk::split( ' ', line1, strtk::range_to_type_back_inserter( l1Tokens ), split_opt );

          info.hwVer = l1Tokens[ 2 ];
          tokenList.pop_front();

          /* Get the version as a number, expected format: vXXX */
          std::string boardVerStr = std::regex_replace( info.hwVer, numberOnlyRegex, "" );
          info.hwVerNum           = static_cast<uint32_t>( std::stoi( boardVerStr ) );
          info.hwVerNumMajor      = static_cast<uint32_t>( std::stoi( info.hwVer.substr( 1, 1 ) ) );

          /*------------------------------------------------
          Pull out the Firmware and Bootloader Versions
          ------------------------------------------------*/
          /* Get the values as strings */
          std::string line2 = tokenList.front();
          std::deque<std::string> l2Tokens;
          strtk::split( ' ', line2, strtk::range_to_type_back_inserter( l2Tokens ), split_opt );

          info.firmwareVer   = l2Tokens[ 1 ];
          info.bootLoaderVer = l2Tokens[ 4 ];
          tokenList.pop_front();

          /* Get the values as numbers: Convert to representation expecting vX.X */
          std::string firmwareVerStr = std::regex_replace( info.firmwareVer, numberOnlyRegex, "" );
          info.firmwareVerNum        = static_cast<uint32_t>( std::stoi( firmwareVerStr ) );
          info.firmwareVerNumMajor   = static_cast<uint32_t>( std::stoi( info.firmwareVer.substr( 1, 1 ) ) );
          info.firmwareVerNumMinor   = static_cast<uint32_t>( std::stoi( info.firmwareVer.substr( 3, 1 ) ) );

          std::string bootloaderVerStr = std::regex_replace( info.bootLoaderVer, numberOnlyRegex, "" );
          info.bootloaderVerNum        = static_cast<uint32_t>( std::stoi( bootloaderVerStr ) );
          info.bootloaderVerNumMajor   = static_cast<uint32_t>( std::stoi( info.bootLoaderVer.substr( 1, 1 ) ) );
          info.bootloaderVerNumMinor   = static_cast<uint32_t>( std::stoi( info.bootLoaderVer.substr( 3, 1 ) ) );

          /*------------------------------------------------
          Pull out the Device ID, Revision ID, and MCU Type
          ------------------------------------------------*/
          std::string line3 = tokenList.front();
          std::deque<std::string> l3Tokens;
          strtk::split( ' ', line3, strtk::range_to_type_back_inserter( l3Tokens ), split_opt );
          tokenList.pop_front();

          /*------------------------------------------------
          Handle Device ID separately
          ------------------------------------------------*/
          std::string rawDevID = l3Tokens[ 0 ];
          std::deque<std::string> devIDTokens;
          strtk::split( ':', rawDevID, strtk::range_to_type_back_inserter( devIDTokens ), split_opt );

          info.deviceID = devIDTokens[ 1 ];

          /*------------------------------------------------
          Handle Revision ID separately
          ------------------------------------------------*/
          std::string rawRevID = l3Tokens[ 1 ];
          std::deque<std::string> revIDTokens;
          strtk::split( ':', rawRevID, strtk::range_to_type_back_inserter( revIDTokens ), split_opt );

          info.revID = revIDTokens[ 1 ];

          /*------------------------------------------------
          Handle MCU type separately
          ------------------------------------------------*/
          std::string mcu = l3Tokens[ 2 ] + ' ' + l3Tokens[ 3 ];
          mcu.erase( std::remove( mcu.begin(), mcu.end(), '(' ), mcu.end() );
          mcu.erase( std::remove( mcu.begin(), mcu.end(), ')' ), mcu.end() );

          info.mcuVer = mcu;

          /*------------------------------------------------
          Validate the Board Version
          ------------------------------------------------*/
          /* Assume everything is OK and then negate if not. */
          info.isValid = true;

          if ( !std::any_of( knownBoardVer.begin(), knownBoardVer.end(),
                             [info]( std::string &i ) { return i == info.hwVer; } ) )
          {
            std::cout << "Unknown board version: " << info.hwVer << std::endl;
            info.isValid = false;
          }

          /*------------------------------------------------
          Validate the Firmware Version
          ------------------------------------------------*/
          if ( !std::any_of( knownFirmwareVer.begin(), knownFirmwareVer.end(),
                             [info]( std::string &f ) { return f == info.firmwareVer; } ) )
          {
            std::cout << "Unknown firmware version: " << info.firmwareVer << std::endl;
            info.isValid = false;
          }

          /*------------------------------------------------
          Validate the Bootloader Version
          ------------------------------------------------*/
          if ( !std::any_of( knownBootloaderVer.begin(), knownBootloaderVer.end(),
                             [info]( std::string &b ) { return b == info.bootLoaderVer; } ) )
          {
            std::cout << "Unknown bootloader version: " << info.bootLoaderVer << std::endl;
            info.isValid = false;
          }
        }
      }
      else
      {
        std::cout << "Could not send command. Bus Pirate not connected." << std::endl;
      }

      this->deviceInfo = info;
      return info;
    }

    ModeBase_sPtr Device::getSystemMode()
    {
      ModeBase_sPtr resultMode;

      if ( isConnected() )
      {
        ///*------------------------------------------------
        // Ping the device a few times so the returned string contains the
        // current mode the device is in.
        //------------------------------------------------*/
        // std::string reportedMode;
        // std::string cmd = MenuCommands::ping;

        // for ( auto x = 0; x < 3; x++ )
        //{
        //  reportedMode = sendResponsiveCommand( cmd );
        //}

        ///*------------------------------------------------
        // Parse the mode
        //------------------------------------------------*/
        // for ( ModeBase_sPtr &x : supportedModes )
        //{
        //  if ( reportedMode.find( x->modeString() ) != std::string::npos )
        //  {
        //    resultMode = x;
        //  }
        //}
      }
      return resultMode;
    }

    void Device::sendCommand( const std::string &cmd ) noexcept
    {
      sendResponsiveCommand( cmd );
    }

    void Device::sendCommand( const std::vector<uint8_t> &cmd ) noexcept
    {
      sendResponsiveCommand( cmd );
    }

    std::string Device::sendResponsiveCommand( const std::string &cmd, const boost::regex &delimiter ) noexcept
    {
      std::vector<uint8_t> readBuffer;
      std::string response;

      /*------------------------------------------------
      Flush the serial port as we don't need data from the
      previous command straying into the response from this one.
      ------------------------------------------------*/
      if ( isConnected() )
      {
        serial->write( reinterpret_cast<const uint8_t *>( cmd.c_str() ), cmd.length() );

        if ( delimiter.empty() )
        {
          serial->readUntil( readBuffer, boost_modeRegex );
        }
        else
        {
          serial->readUntil( readBuffer, delimiter );
        }

        /*------------------------------------------------
        BusPirate emulates a terminal, which means our command is printed
        back to us + that little 'HiZ>' string that we all know means
        'enter more things here'. (What's the proper name for that anyways?)
        Neither are part of the actual output so they are removed.


        HiZ><our_command>\r\n
        <actual output we want>\r\n
        HiZ>
        ------------------------------------------------*/
        if ( readBuffer.size() )
        {
          constexpr size_t newline_char_len = 2;

          /* Remove our command from the front. Expects it to have a '\n' appended on. */
          readBuffer.erase( readBuffer.begin(), ( readBuffer.begin() + cmd.length() - 1 ) + newline_char_len );

          response = std::string( readBuffer.begin(), readBuffer.end() );
        }
      }
      else
      {
        std::cout << "Could not send command. There was a problem with the serial port." << std::endl;
      }

      return response;
    }

    std::vector<uint8_t> Device::sendResponsiveCommand( const std::vector<uint8_t> &cmd,
                                                        const boost::regex &delimiter ) noexcept
    {
      std::vector<uint8_t> readBuffer;

      if ( isConnected() )
      {
        serial->write( cmd.data(), cmd.size() );

        if ( delimiter.empty() )
        {
          serial->readUntil( readBuffer, boost_modeRegex );
        }
        else
        {
          serial->readUntil( readBuffer, delimiter );
        }
      }
      else
      {
        std::cout << "Could not send command. There was a problem with the serial port." << std::endl;
      }

      return readBuffer;
    }

    std::vector<boost::uint8_t> Device::sendResponsiveCommand( const std::vector<uint8_t> &cmd, const uint32_t length )
    {
      std::vector<uint8_t> readBuffer(length);

      if ( isConnected() )
      {
        serial->write( cmd.data(), cmd.size() );
        serial->read(readBuffer.data(), length);
      }
      else
      {
        std::cout << "Could not send command. There was a problem with the serial port." << std::endl;
      }

      return readBuffer;

    }

    bool Device::bbInit()
    {
      bool result                        = false;
      const std::string expectedResponse = "BBIO1";

      if ( reset() )
      {
        std::vector<uint8_t> output;
        std::vector<uint8_t> initCmd;

        initCmd.assign( 20, BitBangCommands::init );
        output = sendResponsiveCommand( initCmd, boost::regex{ "BBIO1" } );
        std::string actualResponse( output.begin(), output.end() );

        if ( actualResponse.find( expectedResponse ) != std::string::npos )
        {
          result = true;
          currentMode = OperationalModes::BP_MODE_BIT_BANG_ROOT;
        }
      }

      return result;
    }

    bool Device::bbEnterSPI()
    {
      bool modeEntered = false;

      /*------------------------------------------------
      Automatically transition the device to BitBang mode if not there
      ------------------------------------------------*/
      if (currentMode != OperationalModes::BP_MODE_BIT_BANG_ROOT )
      {
        bbInit();
      }

      /*------------------------------------------------
      Transition into raw BitBang SPI mode. Success is indicated by the 
      Bus Pirate returning "SPIx" where x is the current SPI version number.
      ------------------------------------------------*/
      if (currentMode == OperationalModes::BP_MODE_BIT_BANG_ROOT)
      {
        std::vector<uint8_t> cmd = { BitBangCommands::enterSPI };
        auto response = sendResponsiveCommand(cmd, boost::regex{"(SPI)."});

        std::string strOut( response.begin(), response.end() );
        std::string substr = "SPI";

        if (strOut.find(substr) != std::string::npos )
        {
          modeEntered = true;
        }
      }
      
      return modeEntered;
    }

    bool Device::bbI2C()
    {
      return false;
    }

    bool Device::bbUART()
    {
      return false;
    }

    bool Device::bb1Wire()
    {
      return false;
    }

    bool Device::bbRawWire()
    {
      return false;
    }

    bool Device::bbJTAG()
    {
      return false;
    }

    bool Device::bbReset()
    {
      return false;
    }

    bool Device::bbCfgPins( const uint8_t &cfg )
    {
      return false;
    }

    bool Device::bbExit()
    {
      return false;
    }

    std::string HiZMode::modeString() noexcept
    {
      return std::string( "HiZ" );
    }

    std::string HiZMode::delimiter() noexcept
    {
      return std::string( "\r\nHiZ>" );
    }

    std::regex HiZMode::regex() noexcept
    {
      return std::regex( "(HiZ>)" );
    }

    HWInterface::BusPirate::OperationalModes HiZMode::modeType() noexcept
    {
      return OperationalModes::BP_MODE_HiZ;
    }

    std::string SPIMode::modeString() noexcept
    {
      return std::string( "SPI" );
    }

    std::string SPIMode::delimiter() noexcept
    {
      return std::string( "\r\nSPI>" );
    }

    std::regex SPIMode::regex() noexcept
    {
      return std::regex( "(SPI>)" );
    }

    HWInterface::BusPirate::OperationalModes SPIMode::modeType() noexcept
    {
      return OperationalModes::BP_MODE_SPI;
    }

  }  // namespace BusPirate

}  // namespace HWInterface
