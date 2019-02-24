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

#include <deque>

using namespace Chimera::Serial;


namespace HWInterface
{
  namespace BusPirate
  {
    Device::Device( std::string &devicePort )
    {
      serial = std::make_shared<SerialDriver>( devicePort );
    }

    bool Device::open()
    {
      if ( !is_open )
      {
        serial->begin();
        auto error =
            serial->configure( 115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE, FlowControl::FCTRL_NONE );

        is_open = ( error == Status::OK );
      }

      // TODO: Will need to check the info stuff to make sure we are a valid device.
      // Check multiple times just in case there is data in the serial buffer.

      return is_open;
    }

    void Device::close()
    {
      serial->end();
      is_open = false;
    }

    Device::Info Device::getInfo()
    {
      Info info;

      std::string cmd = Commands::info;
      std::string rawOutput = sendCommand(cmd);
      auto split_opt = strtk::split_options::compress_delimiters;

      /*------------------------------------------------
      Split the string by new lines
      ------------------------------------------------*/
      std::deque<std::string> tokenList;
      strtk::multiple_char_delimiter_predicate predicate("\r\n");
      strtk::split( predicate, rawOutput, strtk::range_to_type_back_inserter( tokenList ), split_opt );
      
      /*------------------------------------------------
      Pull out the Board version
      ------------------------------------------------*/
      std::string line1 = tokenList.front();
      std::deque<std::string> l1Tokens;
      strtk::split(' ', line1, strtk::range_to_type_back_inserter( l1Tokens ), split_opt );

      info.boardVer = l1Tokens[2];
      tokenList.pop_front();

      /*------------------------------------------------
      Pull out the Firmware and Bootloader Versions
      ------------------------------------------------*/
      std::string line2 = tokenList.front();
      std::deque<std::string> l2Tokens;
      strtk::split( ' ', line2, strtk::range_to_type_back_inserter( l2Tokens ), split_opt );

      info.firmwareVer = l2Tokens[1];
      info.bootLoaderVer = l2Tokens[4];

      tokenList.pop_front();

      /*------------------------------------------------
      Pull out the Device ID, Revision ID, and MCU Type
      ------------------------------------------------*/
      std::string line3 = tokenList.front();
      std::deque<std::string> l3Tokens;
      strtk::split(' ', line3, strtk::range_to_type_back_inserter( l3Tokens ), split_opt );
      tokenList.pop_front();

      /*------------------------------------------------
      Handle Device ID separately
      ------------------------------------------------*/
      std::string rawDevID = l3Tokens[ 0 ];
      std::deque<std::string> devIDTokens;
      strtk::split(':', rawDevID, strtk::range_to_type_back_inserter(devIDTokens), split_opt);

      info.deviceID = devIDTokens[1];

      /*------------------------------------------------
      Handle Revision ID separately 
      ------------------------------------------------*/
      std::string rawRevID = l3Tokens[1];
      std::deque<std::string> revIDTokens;
      strtk::split(':', rawRevID, strtk::range_to_type_back_inserter(revIDTokens), split_opt);

      info.revID = revIDTokens[1];

      /*------------------------------------------------
      Handle MCU type separately
      ------------------------------------------------*/
      std::string mcu = l3Tokens[2] + ' ' + l3Tokens[3];
      mcu.erase( std::remove( mcu.begin(), mcu.end(), '(' ), mcu.end() );
      mcu.erase( std::remove( mcu.begin(), mcu.end(), ')' ), mcu.end() );

      info.mcuVer = mcu;

      return info;
    }


    std::string Device::sendCommand( std::string &cmd )
    {
      std::vector<uint8_t> readBuffer;
      size_t original_cmd_len = cmd.length();
      
      cmd += '\n';
      serial->write( reinterpret_cast<const uint8_t*>(cmd.c_str()), cmd.length());
      serial->readUntil( readBuffer, delimiter_regex);

      /*------------------------------------------------
      BusPirate emulates a terminal, which means our command is printed 
      back to us. This is not part of the actual output so it is removed.

      HiZ><our_command>\r\n
      <actual output we want>\r\n
      HiZ>
      ------------------------------------------------*/
      constexpr size_t newline_char_len = 2;
      readBuffer.erase( readBuffer.begin(), readBuffer.begin() + original_cmd_len + newline_char_len );

      return std::string( readBuffer.begin(), readBuffer.end() );
    }
  }    // namespace BusPirate

}    // namespace HWInterface
