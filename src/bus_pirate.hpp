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
  namespace BusPirate
  {
    class Device;
    using Device_sPtr = std::shared_ptr<Device>;
    using Device_uPtr = std::unique_ptr<Device>;


    class Commands
    {
    public:
      static constexpr auto info = "i";
    };

    class Device
    {
    public:
      struct Info
      {
        std::string boardVer;
        std::string firmwareVer;
        std::string bootLoaderVer;
        std::string deviceID;
        std::string revID;
        std::string mcuVer;

        Info()
        {
          boardVer      = "N/A";
          bootLoaderVer = "N/A";
          deviceID      = "N/A";
          firmwareVer   = "N/A";
          mcuVer        = "N/A";
          revID         = "N/A";
        }
      };

      Device( std::string &devicePort );
      ~Device() = default;

      /**
       *  Opens a connection to the device.
       *
       *  @return True if connected, false if not
       */
      bool open();

      /**
       *  Closes the connection to the device.
       *
       *  @return void
       */
      void close();

      /**
       *  Gets the device information
       *
       *  @return Struct containing the device info
       */
      Info getInfo();


      /**
       *	Sends a command to the device and returns the response
       *
       *	@param[in]	cmd     Command to be sent. Should NOT end with CR or LF.
       *	@return std::string
       */
      std::string sendCommand( std::string &cmd );


    protected:
      SerialDriver_sPtr serial;

      boost::regex delimiter_regex{ "(HiZ>)" };   /**< Matches the BusPirate terminal "end" character sequence */

    private:
      bool is_open;
    };
  }    // namespace BusPirate

}    // namespace HWInterface


#endif /* !BUS_PIRATE_HPP */
