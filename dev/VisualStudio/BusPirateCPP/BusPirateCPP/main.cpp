
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

  std::vector<uint8_t> someData = { 0xaa, 0x11, 0x22, 0x33, 0x44 };

  if (spi.init(setup) == Status::OK )
  {
    spi.setChipSelect(Chimera::GPIO::State::LOW);
    spi.setChipSelect(Chimera::GPIO::State::HI);

    spi.writeBytes( someData.data(), someData.size() );
  }

  spi.deInit();

  return 0;
}
