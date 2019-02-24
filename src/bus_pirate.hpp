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
#include <regex>

/* Module Includes */
#include "serial_driver.hpp"

namespace HWInterface
{
  namespace BusPirate
  {
    class Device;
    using Device_sPtr = std::shared_ptr<Device>;
    using Device_uPtr = std::unique_ptr<Device>;

    /*------------------------------------------------
    Forward declare for using as friends
    ------------------------------------------------*/
    class BinarySPI;
    class I2C;
    class UART;

    class MenuCommands
    {
    public:
      static constexpr auto info    = "i";
      static constexpr auto reset   = "#";
      static constexpr auto busMode = "m";
      static constexpr auto ping    = "\n";
    };

    enum class OperationalModes : uint8_t
    {
      BP_MODE_HiZ = 0,
      BP_MODE_1_WIRE,
      BP_MODE_UART,
      BP_MODE_I2C,
      BP_MODE_SPI,
      BP_MODE_JTAG,
      BP_MODE_RAW2WIRE,
      BP_MODE_RAW3WIRE,
      BP_MODE_PC_KEYBOARD,
      BP_MODE_LCD,

      BP_INVALID_MODE,
      BP_NUM_MODES
    };

    class ModeBase
    {
    public:
      /**
       *	Returns the expected mode string that, when combined with
       *  the mode switch command, produces the correct mode switch.
       *
       *	@return std::string
       */
      virtual std::string modeString() noexcept = 0;

      /**
       *	Gets the expected delimiter that signals the completion of a command.
       *  It will follow some sort of pattern like:
       *  \r\nHiZ>
       *  \r\nSPI>
       *  \r\nJTAG>
       *  etc
       *
       *	@return std::string
       */
      virtual std::string delimiter() noexcept = 0;

      /**
       *	Returns the pattern used to identify the delimiter in a string
       *
       *	@return std::regex
       */
      virtual std::regex regex() noexcept = 0;

      /**
       *  Returns an enum indicating the mode
       *
       *  @return OperationalModes
       */
      virtual OperationalModes modeType() noexcept = 0;
    };

    using ModeBase_sPtr = std::shared_ptr<ModeBase>;

    class HiZMode : public ModeBase
    {
    public:
      HiZMode()  = default;
      ~HiZMode() = default;

      std::string modeString() noexcept override;

      std::string delimiter() noexcept override;

      std::regex regex() noexcept override;

      OperationalModes modeType() noexcept override;
    };

    class SPIMode : public ModeBase
    {
    public:
      SPIMode()  = default;
      ~SPIMode() = default;

      std::string modeString() noexcept override;

      std::string delimiter() noexcept override;

      std::regex regex() noexcept override;

      OperationalModes modeType() noexcept override;
    };

    class Device
    {
    public:
      friend class BinarySPI;
      friend class I2C;
      friend class UART;

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
       *	Checks if the device has been successfully connected
       *
       *	@return True if connected, false if not
       */
      bool isConnected();

      /**
       *  Gets the device information by sending the 'i' command
       *
       *  @return struct containing the device info
       */
      Info getInfo();

      /**
       *	Figures out which mode the Bus Pirate is currently in
       *
       *	@return ModeTypeBase_sPtr
       */
      ModeBase_sPtr getMode();

      /**
       *	Sends a command to the device
       *
       *	@param[in]	cmd       Command to be sent. Should NOT end with CR or LF.
       *	@return void
       */
      void sendCommand( std::string &cmd ) noexcept;

      /**
       *	Sends a command to the device and returns the response. If a delimiter
       *  is not specified, it internally uses a regex that matches the end sequence
       *  of all modes currently supported.
       *
       *	@param[in]	cmd         Command to be sent. Should NOT end with CR or LF.
       *  @param[in]  delimiter   Expected sequence that signals the end of the response.
       *	@return std::string
       */
      std::string sendResponsiveCommand( std::string &cmd, const boost::regex &delimiter = boost::regex{} ) noexcept;


    protected:
      SerialDriver_sPtr serial;
      boost::regex boost_modeRegex{ "(\r\n).+(>)" };
      std::regex std_modeRegex{ "(\r\n).+(>)" };
      static constexpr uint8_t MAX_CONNECT_ATTEMPTS = 3;

      Info deviceInfo;

      std::vector<ModeBase_sPtr> supportedModes;

    private:
    };
  }  // namespace BusPirate

}  // namespace HWInterface


#endif /* !BUS_PIRATE_HPP */
