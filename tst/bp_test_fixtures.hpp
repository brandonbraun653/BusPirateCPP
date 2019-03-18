
#pragma once
#ifndef BP_TEST_FIXTURES_HPP
#define BP_TEST_FIXTURES_HPP

#include <string>

#include <gtest/gtest.h>
#include "serial_driver.hpp"
#include "bus_pirate.hpp"
#include "bp_spi.hpp"

/*------------------------------------------------
Defines the port that some generic USB to UART adapter is connected on
------------------------------------------------*/
extern std::string USB_TO_UART_PORT;

/*------------------------------------------------
Port that the Bus Pirate is connected on
------------------------------------------------*/
extern std::string BUS_PIRATE_PORT;


class SerialFixture : public ::testing::Test
{
protected:
  virtual ~SerialFixture() = default;

  void SetUp() override;
  void TearDown() override;

  HWInterface::SerialDriver *serial;
};

class BusPirateFixture : public ::testing::Test
{
protected:
  virtual ~BusPirateFixture() = default;

  void SetUp() override;
  void TearDown() override;

  HWInterface::BusPirate::Device *busPirate;
};

class BinarySPIFixture : public ::testing::Test
{
protected:
  virtual ~BinarySPIFixture() = default;

  void SetUp() override;
  void TearDown() override;

  HWInterface::BusPirate::Device busPirate = HWInterface::BusPirate::Device( BUS_PIRATE_PORT );
  HWInterface::BusPirate::BinarySPI *spi;
};


#endif /* BP_TEST_FIXTURES_HPP */