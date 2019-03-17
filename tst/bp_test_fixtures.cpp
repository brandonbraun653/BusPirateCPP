

#include "bp_test_fixtures.hpp"
         
std::string USB_TO_UART_PORT = "COM7";

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