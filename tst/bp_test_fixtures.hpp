
#pragma once
#ifndef BP_TEST_FIXTURES_HPP
#define BP_TEST_FIXTURES_HPP

#include <string>

#include <gtest/gtest.h>
#include "serial_driver.hpp"

/*------------------------------------------------
Defines the port that some generic USB to UART adapter is connected on
------------------------------------------------*/
extern std::string USB_TO_UART_PORT;


class SerialFixture : public ::testing::Test
{
protected:
  virtual ~SerialFixture() = default;

  void SetUp() override;
  void TearDown() override;

  HWInterface::SerialDriver *serial;
};


#endif /* BP_TEST_FIXTURES_HPP */