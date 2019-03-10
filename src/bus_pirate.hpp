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
    static constexpr uint8_t CMD_ENTER_RAW_BIT_BANG = 0x00;
    static constexpr uint8_t CMD_EXIT_RAW_BIT_BANG  = 0x0F;

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
      static const std::string info;    /**< Hardware, firmware, and microcontroller version information */
      static const std::string reset;   /**< Resets the board (firmware V2.0+) */
      static const std::string busMode; /**< Sets the bus mode (1-Wire, SPI, I2C, JTAG, UART, etc). */
      static const std::string ping;    /**< Simulates the user mashing the 'enter' key */
    };

    class BitBangCommands
    {
    public:
      static constexpr uint8_t init    = 0x00; /**< Command that when repeated, enters bit-bang mode */
      static constexpr uint8_t unknown = 0x00; /**< Response when command is ineffective or invalid */
      static constexpr uint8_t success = 0x01; /**< Indicates that a command succeeded */
      static constexpr uint8_t reset   = 0x0F; /**< Resets the Bus Pirate and returns to the user terminal */

      static constexpr uint8_t enterSPI = 0x01; /**< Enter SPI bit bang mode */

      static const std::string initSuccess; /**< Character sequence that indicates transition to Bit Bang root mode */
    };

    class ModeTracker
    {
    public:
      void update();

      void getModeRegex();

    protected:
    private:
    };

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

      BP_MODE_BIT_BANG_ROOT,
      BP_MODE_SPI_BIT_BANG,

      BP_INVALID_MODE,
      BP_NUM_MODES
    };

    enum class InteractionMode : uint8_t
    {
      MODE_TERMINAL,
      MODE_BIT_BANG
    };

    class Device
    {
    public:
      friend class BinarySPI;
      friend class I2C;
      friend class UART;

      Device( std::string &devicePort );
      Device() = default;
      ~Device() = default;

      /**
       *  Opens a connection to the device. By default, clears all settings and
       *  leaves the device in the user terminal in HiZ mode.
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
       *	Resets the Bus Pirate back to terminal mode and clears all settings.
       *	Only firmware versions v2.0+ can do this.
       *
       *	@return True if success, false if not
       */
      bool reset();

      /**
       *	Attempts to reset the board and validate the device info can be received properly
       *
       *	@return bool: True if succeeded, else false
       */
      bool connect();

      /**
       *	Checks if the the serial port has been opened and configured
       *
       *	@return True if connected, false if not
       */
      bool isOpen();

      /**
       *	Clears the terminal interface by sending 'enter' a few times
       *
       *	@return void
       */
      void clearTerminal();

      /**
       *  Gets the device information by sending the 'i' command
       *
       *  @return struct containing the device info
       */
      Info getInfo();

      /**
       *	Sends a command to the device
       *
       *	@param[in]	cmd         Command to be sent that ends with '\n'.
       *	@return void
       */
      void sendCommand( const std::string &cmd ) noexcept;

      /**
       *	Send a series of bytes to the Bus Pirate
       *
       *	@param[in]	cmd         Command to be sent that ends with '\n'.
       *	@return void
       */
      void sendCommand( const std::vector<uint8_t> &cmd ) noexcept;

      /**
       *	Sends a command to the device and returns the response. If a delimiter
       *  is not specified, it internally uses a regex that matches the end sequence
       *  of all modes currently supported.
       *
       *	@param[in]	cmd         Command to be sent that ends with '\n'.
       *  @param[in]  delimiter   Expected sequence that signals the end of the response.
       *	@return std::string
       */
      std::string sendResponsiveCommand( const std::string &cmd, const boost::regex &delimiter = boost::regex{} ) noexcept;

      /**
       *	Send a series of bytes to the Bus Pirate, returning the response to the user.
       *  Most typically this is used for terminal mode that uses a regex to figure out
       *  when the transfer has ended.
       *
       *	@param[in]	cmd         Command to be sent that ends with '\n'.
       *  @param[in]  delimiter   Regex used to decide when reading the response is finished
       *	@return std::vector<uint8_t>
       */
      std::vector<uint8_t> sendResponsiveCommand( const std::vector<uint8_t> &cmd,
                                                  const boost::regex &delimiter = boost::regex{} ) noexcept;

      /**
       *	Send a series of bytes to the Bus Pirate, returning the response to the user.
       *  This version is used primarily in Bit Bang mode where the response length is known.
       *
       *	@param[in]	cmd         Command to be sent (does not need to end in '\n')
       *	@param[in]	length      How many bytes to read before returning
       *	@return std::vector<boost::uint8_t>
       */
      std::vector<uint8_t> sendResponsiveCommand( const std::vector<uint8_t> &cmd, const uint32_t length ) noexcept;

      /**
       *  Resets the board and enters terminal mode
       *
       *	@return bool: true if success, false if not
       */
      bool terminalInit();

      /**
       *	Enters bit bang mode
       *
       *	@return bool: true if success, false if not
       */
      bool bbInit();

      /**
       *  Enters raw SPI mode.
       *  Must have called bbInit() first or else this will fail.
       *
       *	@return bool: true if success, false if not
       */
      bool bbEnterSPI();

      /**
       *  Enters raw I2C mode.
       *  Must have called bbInit() first or else this will fail.
       *
       *	@return bool: true if success, false if not
       */
      bool bbI2C();

      /**
       *  Enters raw UART mode.
       *  Must have called bbInit() first or else this will fail.
       *
       *	@return bool: true if success, false if not
       */
      bool bbUART();

      /**
       *  Enters raw 1Wire mode.
       *  Must have called bbInit() first or else this will fail.
       *
       *	@return bool: true if success, false if not
       */
      bool bb1Wire();

      /**
       *  Enters Raw Wire mode.
       *  Must have called bbInit() first or else this will fail.
       *
       *	@return bool: true if success, false if not
       */
      bool bbRawWire();

      /**
       *  Enters raw JTAG mode.
       *  Must have called bbInit() first or else this will fail.
       *
       *	@return bool: true if success, false if not
       */
      bool bbJTAG();

      /**
       *	Exits from whatever raw mode the user was in and goes back to bit bang mode
       *
       *	@return bool: true if success, false if not
       */
      bool bbExitHWMode();


    protected:
      SerialDriver_sPtr serial;
      boost::regex terminalModeRegex{ "(\r\n).+(>)" };
      static constexpr uint8_t MAX_CONNECT_ATTEMPTS = 3;

      Info deviceInfo;

      bool connectedToSerial; /**< True if the device is connected and configured over serial, false if not */
      OperationalModes currentMode;


      /**
       *  Resets the device under the assumption we are in terminal mode.
       *  The device will be in terminal mode after success.
       *
       *	@return bool: true if success, false if not
       */
      bool resetTerminal();

      /**
       *	Resets the device under the assumption we are in Bit Bang root.
       *  The device will be in terminal mode after success.
       *
       *	@return bool: true if success, false if not
       */
      bool resetBitBangRoot();

      /**
       *	Resets the device under the assumption we are in some Bit Bang HW mode.
       *  The device will be in terminal mode after success.
       *
       *	@return bool: true if success, false if not
       */
      bool resetBitBangHWMode();

    private:
    };
  }  // namespace BusPirate

}  // namespace HWInterface


#endif /* !BUS_PIRATE_HPP */
