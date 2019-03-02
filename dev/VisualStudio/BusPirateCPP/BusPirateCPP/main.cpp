
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

  std::array<uint8_t, 5> writeData = { 0xaa, 0x11, 0x22, 0x33, 0x44 };
  std::array<uint8_t, 5> readData;

  if (spi.init(setup) == Status::OK )
  {
    readData.fill(0);

    spi.writeBytes( writeData.data(), writeData.size() );
    spi.readBytes(readData.data(), readData.size() );
    spi.readWriteBytes(writeData.data(), readData.data(), writeData.size() );
  }

  spi.deInit();

  return 0;
}
