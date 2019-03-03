
#include <iostream>
#include "bus_pirate.hpp"
#include "serial_driver.hpp"
#include "bp_spi.hpp"

using namespace Chimera::Serial;

constexpr auto SERIAL_BUFFER_SIZE = 4096;

using namespace boost;
using namespace std;

int main()
{
  std::string device = "COM6";
  HWInterface::BusPirate::Device bp( device );
  HWInterface::BusPirate::BinarySPI spi( bp );

  Chimera::SPI::Setup setup;
  setup.bitOrder = Chimera::SPI::BitOrder::MSB_FIRST;
  setup.mode = Chimera::SPI::Mode::MASTER;
  setup.clockMode = Chimera::SPI::ClockMode::MODE0;
  setup.clockFrequency = 1000000;
  setup.dataSize = Chimera::SPI::DataSize::SZ_8BIT;

  if (spi.init(setup) == Status::OK )
  {
    
  }

  spi.deInit();

  return 0;
}
