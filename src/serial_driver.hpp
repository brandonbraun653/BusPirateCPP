/********************************************************************************
 *   File Name:
 *     serial_driver.hpp
 *
 *   Description:
 *     Provides an interface to the host computer's serial port.
 *
 *   2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/
#pragma once
#ifndef BUS_PIRATE_CPP_SERIAL_DRIVER_HPP
#define BUS_PIRATE_CPP_SERIAL_DRIVER_HPP

/* C++ Includes */
#include <string>

/* Boost Includes */
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/serial_port.hpp>

/* Chimera Includes */
#include <Chimera/interface.hpp>

class SerialDriver : public Chimera::Serial::Interface
{
public:
  SerialDriver(std::string &comPort);
  ~SerialDriver() = default;

  Chimera::Serial::Status begin( const uint32_t baud, const Chimera::Serial::Modes txMode,
                                 const Chimera::Serial::Modes rxMode ) override;

  Chimera::Serial::Status setBaud( const uint32_t buad ) override;

  Chimera::Serial::Status setMode( const Chimera::Serial::SubPeripheral periph, const Chimera::Serial::Modes mode ) override;

  Chimera::Serial::Status write( const uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) override;

  Chimera::Serial::Status read( uint8_t *const buffer, const size_t length, const uint32_t timeout_mS = 500 ) override;

  bool reserve( const uint32_t timeout_mS ) override;

  bool release( const uint32_t timeout_mS ) override;
};


#endif /* !BUS_PIRATE_CPP_SERIAL_DRIVER_HPP */