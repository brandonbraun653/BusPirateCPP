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

      std::cout << rawOutput << std::endl;

      return info;
    }


    std::string Device::sendCommand( std::string &cmd )
    {
      std::vector<uint8_t> readBuffer;

      serial->write( reinterpret_cast<const uint8_t*>(cmd.c_str()), cmd.length());
      serial->readUntil( readBuffer, delimiter_regex);

      return std::string( readBuffer.begin(), readBuffer.end() );
    }
  }    // namespace BusPirate

}    // namespace HWInterface
