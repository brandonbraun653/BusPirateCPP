/********************************************************************************
 *  File Name:
 *    bp_test_fixtures.cpp
 *
 *  Description:
 *    Implements test fixtures for the Bus Pirate
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include "bp_test_fixtures.hpp"
         
std::string USB_TO_UART_PORT = "COM7";
std::string BUS_PIRATE_PORT  = "COM6";

using namespace HWInterface;
using namespace Chimera::Serial;

void SerialFixture::SetUp()
{
  serial = new SerialDriver( USB_TO_UART_PORT );
  if (serial->begin() == Status::OK )
  {
    serial->configure( 115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE, FlowControl::FCTRL_NONE );
  }

  ASSERT_EQ( true, serial->isOpen() );
}

void SerialFixture::TearDown()
{
  serial->end();
  delete serial;
}

void BusPirateFixture::SetUp()
{
  busPirate = new HWInterface::BusPirate::Device( BUS_PIRATE_PORT );
  ASSERT_EQ( true, busPirate->open() );
}

void BusPirateFixture::TearDown()
{
  busPirate->close();
  delete busPirate;
}

void BinarySPIFixture::SetUp()
{
  spi = new HWInterface::BusPirate::BinarySPI( busPirate );

  Chimera::SPI::Setup setup;
  spi->init( setup );
}

void BinarySPIFixture::TearDown()
{
  spi->deInit();
  delete spi;
}