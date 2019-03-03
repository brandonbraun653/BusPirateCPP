
#include <iostream>
#include "bus_pirate.hpp"
#include "serial_driver.hpp"

using namespace Chimera::Serial;

constexpr auto SERIAL_BUFFER_SIZE = 4096;

using namespace boost;
using namespace std;

int main()
{
  std::string device = "COM7";
  HWInterface::BusPirate::Device bp( device );

  if( bp.open())
  {
    bp.bbInit();
    bp.bbEnterSPI();
    bp.close();
  }

  return 0;
}
