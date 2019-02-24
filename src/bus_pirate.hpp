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


    class MenuCommands
    {
    public:
      static constexpr auto info = "i";
      static constexpr auto reset = "#";
    };

    class Device
    {
    public:
      struct Info
      {
        bool isValid;

        std::string hwVer;
        std::string firmwareVer;
        std::string bootLoaderVer;
        std::string deviceID;
        std::string revID;
        std::string mcuVer;

        uint32_t hwVerNum;
        uint32_t hwVerNumMajor;

        uint32_t firmwareVerNum;
        uint32_t firmwareVerNumMajor;
        uint32_t firmwareVerNumMinor;

        uint32_t bootloaderVerNum;
        uint32_t bootloaderVerNumMajor;
        uint32_t bootloaderVerNumMinor;

        Info()
        {
          isValid = false;

          hwVer         = "N/A";
          bootLoaderVer = "N/A";
          deviceID      = "N/A";
          firmwareVer   = "N/A";
          mcuVer        = "N/A";
          revID         = "N/A";

          hwVerNum      = 0;
          hwVerNumMajor = 0;

          firmwareVerNum      = 0;
          firmwareVerNumMajor = 0;
          firmwareVerNumMinor = 0;

          bootloaderVerNum      = 0;
          bootloaderVerNumMajor = 0;
          bootloaderVerNumMinor = 0;
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
       *	Resets the Bus Pirate according to command '#'
       *	Only firmware versions v2.0+ can do this.
       *
       *	@return True if success, false if not
       */
      bool reset();

      /**
       *  Gets the device information by sending the 'i' command
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

      std::string delimiter = "\r\nHiZ>";       /**< Full terminal end of command character sequence */
      boost::regex delimiter_regex{ "(HiZ>)" }; /**< Matches the BusPirate terminal "end" character sequence */

      static constexpr uint8_t MAX_CONNECT_ATTEMPTS = 3;

      Info deviceInfo;

    private:
    };
  }  // namespace BusPirate

}  // namespace HWInterface


#endif /* !BUS_PIRATE_HPP */
