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


namespace HWInterface
{
  BusPirate::BusPirate( std::string &devicePort )
  {
    serial = std::make_shared<SerialDriver>(devicePort);
  }
}    // namespace HWInterface