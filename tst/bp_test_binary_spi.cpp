/********************************************************************************
 *  File Name:
 *    bp_test_binary_spi.cpp
 *
 *  Description:
 *    Tests the binary SPI interface to ensure that it works properly
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include "bp_test_fixtures.hpp"

TEST_F( BusPirateFixture, EnterBinarySPI )
{
  EXPECT_EQ( true, busPirate->bbEnterSPI() );
}

TEST_F( BusPirateFixture, ResetFromBinarySPI )
{
  ASSERT_EQ( true, busPirate->bbEnterSPI() );
  EXPECT_EQ( true, busPirate->reset() );
}

TEST( BinarySPITest, initialization )
{
  HWInterface::BusPirate::Device busPirate( BUS_PIRATE_PORT );
  HWInterface::BusPirate::BinarySPI spi( busPirate );

  Chimera::SPI::Setup setup;

  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi.init( setup ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi.deInit() );
}

TEST_F( BinarySPIFixture, ConfigPWR )
{
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgPowerSupplies( true ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgPowerSupplies( false ) );
}

TEST_F( BinarySPIFixture, ConfigAUX )
{
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgAuxPin( true ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgAuxPin( false ) );
}

TEST_F( BinarySPIFixture, ConfigPULLUPS )
{
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgPullups( true ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgPullups( false ) );
}

TEST_F( BinarySPIFixture, ConfigCS )
{
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgChipSelect( true ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgChipSelect( false ) );
}

TEST_F( BinarySPIFixture, ConfigPINOUT )
{
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgSPIPinOut( true ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgSPIPinOut( false ) );
}

TEST_F( BinarySPIFixture, ConfigCLK_IDLE_STATE )
{
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgSPIClkIdle( true ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgSPIClkIdle( false ) );
}

TEST_F( BinarySPIFixture, ConfigCLK_EDGE )
{
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgSPIClkEdge( true ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->cfgSPIClkEdge( false ) );
}

TEST_F( BinarySPIFixture, ClockSetGetExact )
{
  using namespace HWInterface::BusPirate;

  /*------------------------------------------------
  Set exactly 30KHz clock
  ------------------------------------------------*/
  uint32_t desired_clock = SPEED_30kHz;
  uint32_t desired_tolerance = 0;
  uint32_t actual_clock =  0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );

  /*------------------------------------------------
  Set exactly 125KHz clock
  ------------------------------------------------*/
  desired_clock     = SPEED_125kHz;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );

  /*------------------------------------------------
  Set exactly 250KHz clock
  ------------------------------------------------*/
  desired_clock     = SPEED_250kHz;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );

  /*------------------------------------------------
  Set exactly 1MHz clock
  ------------------------------------------------*/
  desired_clock     = SPEED_1MHz;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );

  /*------------------------------------------------
  Set exactly 2MHz clock
  ------------------------------------------------*/
  desired_clock     = SPEED_2MHz;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );

  /*------------------------------------------------
  Set exactly 2.6MHz clock
  ------------------------------------------------*/
  desired_clock     = SPEED_2_6MHz;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );

  /*------------------------------------------------
  Set exactly 4MHz clock
  ------------------------------------------------*/
  desired_clock     = SPEED_4MHz;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );

  /*------------------------------------------------
  Set exactly 8MHz clock
  ------------------------------------------------*/
  desired_clock     = SPEED_8MHz;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( desired_clock, actual_clock );
}

TEST_F( BinarySPIFixture, ClockSetGetImprecise )
{
  using namespace HWInterface::BusPirate;

  /*------------------------------------------------
  Set clock values just around 30kHz
  ------------------------------------------------*/
  uint32_t desired_clock     = 10000;
  uint32_t desired_tolerance = 0;
  uint32_t actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_GT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_30kHz, actual_clock );

  desired_clock     = 35000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_LT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_30kHz, actual_clock );

  /*------------------------------------------------
  Set clock values just around 125KHz
  ------------------------------------------------*/
  desired_clock     = 100000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_GT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_125kHz, actual_clock );

  desired_clock     = 125000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_125kHz, actual_clock );

  desired_clock     = 127000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_LT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_125kHz, actual_clock );

  /*------------------------------------------------
  Set clock values just around 1MHz
  ------------------------------------------------*/
  desired_clock     = 900000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_GT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_1MHz, actual_clock );

  desired_clock     = 1000000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_1MHz, actual_clock );

  desired_clock     = 1100000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_LT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_1MHz, actual_clock );

  /*------------------------------------------------
  Set clock values just around 8MHz
  ------------------------------------------------*/
  desired_clock     = 7500000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_GT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_8MHz, actual_clock );

  desired_clock     = 8000000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_EQ, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_8MHz, actual_clock );

  desired_clock     = 8100000;
  desired_tolerance = 0;
  actual_clock      = 0;

  EXPECT_EQ( Chimera::SPI::Status::CLOCK_SET_LT, spi->setClockFrequency( desired_clock, desired_tolerance ) );
  EXPECT_EQ( Chimera::CommonStatusCodes::OK, spi->getClockFrequency( actual_clock ) );
  EXPECT_EQ( SPEED_8MHz, actual_clock );
}