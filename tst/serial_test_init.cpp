/********************************************************************************
 *  File Name:
 *    serial_test_init.cpp
 *
 *  Description:
 *    Tests initialization sequences of the serial driver
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include <gtest/gtest.h>
#include "bp_test_fixtures.hpp"

#include "serial_driver.hpp"

using namespace HWInterface;
using namespace Chimera::Serial;

TEST( SerialDriverTests, OpenBadPort )
{
  std::string badPort = "COMabcdefg";
  SerialDriver serial( badPort );

  ASSERT_EQ( Chimera::Serial::Status::FAILED_OPEN, serial.begin() );
}

TEST( SerialDriverTests, OpenGoodPort )
{
  SerialDriver serial( USB_TO_UART_PORT );
  ASSERT_EQ( Chimera::Serial::Status::OK, serial.begin() );
}

TEST( SerialDriverTests, OpenCloseRinseRepeat )
{
  SerialDriver serial( USB_TO_UART_PORT );

  ASSERT_EQ( Chimera::Serial::Status::OK, serial.begin() );
  ASSERT_EQ( Chimera::Serial::Status::OK, serial.end() );
  ASSERT_EQ( Chimera::Serial::Status::OK, serial.begin() );
  ASSERT_EQ( Chimera::Serial::Status::OK, serial.end() );
}

TEST( SerialDriverTests, OpenCloseConfigureRinseRepeat )
{
  SerialDriver serial( USB_TO_UART_PORT );

  ASSERT_EQ( Chimera::Serial::Status::OK, serial.begin() );
  ASSERT_EQ( Chimera::Serial::Status::OK,
             serial.configure( 115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE, FlowControl::FCTRL_NONE ) );
  ASSERT_EQ( Chimera::Serial::Status::OK, serial.end() );

  ASSERT_EQ( Chimera::Serial::Status::OK, serial.begin() );
  ASSERT_EQ( Chimera::Serial::Status::OK,
             serial.configure( 115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE, FlowControl::FCTRL_NONE ) );
  ASSERT_EQ( Chimera::Serial::Status::OK, serial.end() );
}