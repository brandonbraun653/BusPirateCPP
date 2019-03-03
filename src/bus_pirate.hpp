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
      static constexpr auto info    = "i\n"; /**< Hardware, firmware, and microcontroller version information */
      static constexpr auto reset   = "#\n"; /**< Resets the board (firmware V2.0+) */
      static constexpr auto busMode = "m\n"; /**< Sets the bus mode (1-Wire, SPI, I2C, JTAG, UART, etc). */
      static constexpr auto ping    = "\n";  /**< Simulates the user mashing the 'enter' key */
    };

    class BitBangCommands
    {
    public:
      static constexpr uint8_t init  = 0x00; /**< Command that when repeated, enters bit-bang mode */
      static constexpr uint8_t success = 0x01; /**< Indicates that a command succeeded */
      static constexpr uint8_t reset = 0x0F; /**< Resets the Bus Pirate and returns to the user terminal */

      static constexpr uint8_t enterSPI = 0x01; /**< When in Bit Bang mode, this will cause the Bus Pirate to enter SPI bit bang mode */

    };

    // class OperationalModes
    //{
    // public:
    //
    //};

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

      Device( std::string &devicePort );
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
       *	Resets the Bus Pirate according to command '#'
       *	Only firmware versions v2.0+ can do this.
       *
       *	@return True if success, false if not
       */
      bool reset();


      bool connect();

      /**
       *	Checks if the device has been successfully connected
       *
       *	@return True if connected, false if not
       */
      bool isConnected();


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
       *	Figures out which mode the Bus Pirate is currently in
       *
       *	@return ModeTypeBase_sPtr
       */
      ModeBase_sPtr getSystemMode();

      /**
       *	Figures out if the device is in terminal mode or bit bang mode. Will automatically
       *	reset bit bang mode if already in that mode.
       *
       *	@return HWInterface::BusPirate::InteractionMode
       */
       InteractionMode getInteractionMode();

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
       *  Most typically this is used for bit-bang or a raw interface mode
       *
       *	@param[in]	cmd         Command to be sent that ends with '\n'.
       *	@return std::vector<uint8_t>
       */
      std::vector<uint8_t> sendResponsiveCommand( const std::vector<uint8_t> &cmd, const boost::regex &delimiter = boost::regex{} ) noexcept;

      std::vector<uint8_t> sendResponsiveCommand( const std::vector<uint8_t> &cmd, const uint32_t length);

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
       *
       *
       *	@return bool
       */
      bool bbI2C();

      /**
       *
       *
       *	@return bool
       */
      bool bbUART();

      /**
       *
       *
       *	@return bool
       */
      bool bb1Wire();

      /**
       *
       *
       *	@return bool
       */
      bool bbRawWire();

      /**
       *
       *
       *	@return bool
       */
      bool bbJTAG();

      /**
       *  Performs a hardware reset of the Bus Pirate and returns to the standard
       *  input terminal. bbInit() must be called again to re-enter bit bang mode.
       *
       *	@return bool: true if success, false if not
       */
      bool bbReset();

      /**
       *	Manually sets output/input pin configurations.
       *  Input parameter should set (output) or clear (input) the lower 5 bits in this order:
       *  AUX|MOSI|CLK|MISO|CS
       *
       *	@param[in]	cfg         The pin configuration option for the Bus Pirate
       *	@return bool: true if success, false if not
       */
      bool bbCfgPins( const uint8_t &cfg );

      /**
       *	Exits from whatever raw mode the user was in and goes back to bit bang mode
       *
       *	@return bool
       */
      bool bbExit();


    protected:
      SerialDriver_sPtr serial;
      boost::regex boost_modeRegex{ "(\r\n).+(>)" };
      std::regex std_modeRegex{ "(\r\n).+(>)" };
      static constexpr uint8_t MAX_CONNECT_ATTEMPTS = 3;

      Info deviceInfo;

      bool connectedToSerial; /**< True if the device is connected and configured over serial, false if not */

      std::vector<ModeBase_sPtr> supportedModes;
      OperationalModes currentMode;

    private:
    };
  }  // namespace BusPirate

}  // namespace HWInterface


#endif /* !BUS_PIRATE_HPP */
