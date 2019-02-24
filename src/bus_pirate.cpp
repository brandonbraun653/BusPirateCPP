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

    static constexpr uint32_t minResetFirmwareMajorVer = 2; /**< Minimum firmware version needed to support reset '#' command */


    Device::Device( std::string &devicePort )
    {
      serial = std::make_shared<SerialDriver>( devicePort );
    }

    bool Device::open()
    {
      bool connected = true;

      if ( !serial->isOpen() )
      {
        connected = false;

        serial->begin();
        auto error = serial->configure( 115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE,
                                        FlowControl::FCTRL_NONE );

        /*------------------------------------------------
        Make sure that we can talk to the device correctly. Occasionally there will be old data in the system serial buffer that
        hasn't been cleared out yet. Boost does not provide a way to flush this, so the simple fix is just to try and read
        things out again.
        ------------------------------------------------*/
        if ( error == Status::OK )
        {
          for ( auto x = 0; x < MAX_CONNECT_ATTEMPTS; x++ )
          {
            if ( getInfo().isValid )
            {
              connected = true;
              break;
            }
            else
            {
              std::cout << "Retrying connection..." << std::endl;
              boost::this_thread::sleep_for( boost::chrono::milliseconds( 500 ) );
            }
          }
        }
      }

      return connected;
    }

    void Device::close()
    {
      serial->end();
    }

    bool Device::reset()
    {
      bool result = false;

      if ( deviceInfo.isValid && ( deviceInfo.firmwareVerNumMajor > minResetFirmwareMajorVer ) )
      {
        std::string cmd = MenuCommands::reset;
        std::string out = sendCommand(cmd);

        /*------------------------------------------------
        According to the docs, RESET should be printed if the
        command was received/executed successfully.
        ------------------------------------------------*/
        if ( out.find( "RESET" ) != std::string::npos )
        {
          result = true;
        }

        std::cout << out << std::endl;
      }
      return result;
    }

    Device::Info Device::getInfo()
    {
      Info info;

      std::regex numberOnlyRegex = std::regex( R"([\D])" );
      std::string cmd            = MenuCommands::info;
      std::string rawOutput      = sendCommand( cmd );
      auto split_opt             = strtk::split_options::compress_delimiters;

      /*------------------------------------------------
      Split the string by new lines
      ------------------------------------------------*/
      std::deque<std::string> tokenList;
      strtk::multiple_char_delimiter_predicate predicate( "\r\n" );
      strtk::split( predicate, rawOutput, strtk::range_to_type_back_inserter( tokenList ), split_opt );

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

      if ( !std::any_of( knownBoardVer.begin(), knownBoardVer.end(), [info]( std::string &i ) { return i == info.hwVer; } ) )
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

      this->deviceInfo = info;
      return info;
    }

    std::string Device::sendCommand( std::string &cmd )
    {
      std::vector<uint8_t> readBuffer;
      size_t original_cmd_len = cmd.length();

      cmd += '\n';
      serial->write( reinterpret_cast<const uint8_t *>( cmd.c_str() ), cmd.length() );
      serial->readUntil( readBuffer, delimiter_regex );

      /*------------------------------------------------
      BusPirate emulates a terminal, which means our command is printed
      back to us + that little 'HiZ>' string that we all know means
      'enter more things here'. (What's the proper name for that anyways?)
      Neither are part of the actual output so they are removed.

      HiZ><our_command>\r\n
      <actual output we want>\r\n
      HiZ>
      ------------------------------------------------*/
      constexpr size_t newline_char_len = 2;

      /* Remove our command from the front */
      readBuffer.erase( readBuffer.begin(), readBuffer.begin() + original_cmd_len + newline_char_len );

      /* Remove the delimiter from the back */
      readBuffer.erase( readBuffer.end() - delimiter.size(), readBuffer.end() );

      return std::string( readBuffer.begin(), readBuffer.end() );
    }
  }  // namespace BusPirate

}  // namespace HWInterface
