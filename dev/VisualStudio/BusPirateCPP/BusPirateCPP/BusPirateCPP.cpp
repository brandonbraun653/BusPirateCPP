
#include <iostream>
#include "serial_driver.hpp"

using namespace Chimera::Serial;

int main()
{
  uint8_t data[ 100 ];
  memset( data, 0, 100 );

  std::cout << "Hello World!\n";

  std::string device = "COM6";
  HWInterface::SerialDriver serial( device );

  const char *wData = "?\n";

  serial.begin(Modes::BLOCKING, Modes::BLOCKING);
  serial.configure(115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE, FlowControl::FCTRL_NONE);
  serial.write(reinterpret_cast<const uint8_t*>(wData), 2);
  serial.read(data, 100);
  serial.end();

  std::cout << data << std::endl;

  std::cout << "finished" << std::endl;


  return 0;
}