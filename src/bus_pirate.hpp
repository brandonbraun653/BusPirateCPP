/********************************************************************************
 *   File Name:
 *      bus_pirate.hpp
 *
 *   Description:
 *      Defines an interface to the ever popular BusPirate hardware. The goal here
 *      is to provide software portable to both Windows and Linux in support of
 *      developing hardware drivers for systems that operate on SPI, I2C and UART
 *      interfaces.
 *
 *      In general, I needed something like this to use a modern development
 *      environment to write embedded systems firmware.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef BUS_PIRATE_HPP
#define BUS_PIRATE_HPP

/* C++ Includes */
#include <string>
#include <memory>

/* Module Includes */
#include "serial_driver.hpp"

namespace HWInterface
{
  class BusPirate;
  typedef std::shared_ptr<BusPirate> BusPirate_sPtr;
  typedef std::unique_ptr<BusPirate> BusPirate_uPtr;

  class BusPirate
  {
  public:
    BusPirate(std::string &devicePort);
    ~BusPirate() = default;




  protected:
    SerialDriver_sPtr serial;
  
  private:

  };

}


#endif /* !BUS_PIRATE_HPP */