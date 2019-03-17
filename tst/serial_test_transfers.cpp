/********************************************************************************
 *  File Name:
 *    serial_test_transfers.cpp
 *
 *  Description:
 *    Tests transfers of the serial driver. Expects the user to have some kind of
 *    USB to UART adapter connected in loopback mode (TX/RX shorted together).
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include <gtest/gtest.h>
#include "bp_test_fixtures.hpp"

#include "serial_driver.hpp"

#include <random>
#include <algorithm>
#include <functional>

#include <boost/regex.hpp>


using namespace Chimera::Serial;

TEST_F( SerialFixture, FixedLenWriteRead )
{
  static constexpr size_t len = 4;

  std::array<uint8_t, len> writeData = { 0x55, 0x33, 0x23, 0x99 };
  std::array<uint8_t, len> readData  = { 0x00, 0x00, 0x00, 0x00 };

  EXPECT_EQ( Status::OK, serial->write( writeData.data(), len ) );
  EXPECT_EQ( Status::OK, serial->read( readData.data(), len ) );
  EXPECT_EQ( 0, memcmp( writeData.data(), readData.data(), len ) );
}

TEST_F( SerialFixture, LargeWriteRead )
{
  using namespace std;

  random_device rnd_device;
  mt19937 mersenne_engine{ rnd_device() };
  uniform_int_distribution<unsigned short> dist{ 0x00, 0xFF };

  auto gen = [&dist, &mersenne_engine]() { return dist( mersenne_engine ); };

  constexpr int len = 1000;
  vector<uint8_t> writeData( len );
  generate( writeData.begin(), writeData.end(), gen );

  std::array<uint8_t, len> readData;
  readData.fill(0);


  EXPECT_EQ( Status::OK, serial->write( writeData.data(), len ) );
  EXPECT_EQ( Status::OK, serial->read( readData.data(), len ) );
  EXPECT_EQ( 0, memcmp( writeData.data(), readData.data(), len ) );
}

TEST_F( SerialFixture, UnknownLength )
{
  std::string writeData = "I have no clue when this will 758ryt end";
  std::vector<uint8_t> readData;
  boost::regex regex{ "(758ryt)" };

  serial->write( reinterpret_cast<const uint8_t *>(writeData.c_str()), writeData.length());
  EXPECT_EQ( Status::OK, serial->readUntil(readData, regex));

  /*------------------------------------------------
  While this string is past the regex, it should still show up
  in the output due to how ASIO will return "bonus" data.
  ------------------------------------------------*/
  std::string nonExistantString = "end";
  std::string output = std::string( readData.begin(), readData.end() );

  ASSERT_GT( output.find( nonExistantString ), 35 );
}

TEST_F( SerialFixture, RegexReadTimeout )
{
  std::vector<uint8_t> readData;
  boost::regex regex{ "(758ryt)" };

  EXPECT_EQ( Status::EMPTY, serial->readUntil( readData, regex ) );
}

TEST_F( SerialFixture, FixedLenReadTimeout )
{
  std::array<uint8_t, 5> readData;
  readData.fill(0);

  EXPECT_EQ( Status::EMPTY, serial->read( readData.data(), readData.size() ) );
}

TEST_F( SerialFixture, FailedReadThenAttemptToWrite )
{
  /*------------------------------------------------
  Fail a fixed length read and then immediately try to write more
  ------------------------------------------------*/
  static constexpr size_t len = 4;

  std::array<uint8_t, len> writeData = { 0x55, 0x33, 0x23, 0x99 };
  std::array<uint8_t, len> readData;
  readData.fill( 0 );
  
  EXPECT_EQ( Status::EMPTY, serial->read( readData.data(), readData.size() ) );
  EXPECT_EQ( true, std::all_of( readData.begin(), readData.end(), []( auto i ) { return i == 0; } ) );

  EXPECT_EQ( Status::OK, serial->write( writeData.data(), len ) );
  EXPECT_EQ( Status::OK, serial->read( readData.data(), len ) );
  EXPECT_EQ( 0, memcmp( writeData.data(), readData.data(), len ) );

  /*------------------------------------------------
  Fail a regex read then immediately try to write more
  ------------------------------------------------*/
  std::vector<uint8_t> readDataPt2;
  boost::regex regex{ "(758ryt)" };

  EXPECT_EQ( Status::EMPTY, serial->readUntil( readDataPt2, regex ) );

  EXPECT_EQ( Status::OK, serial->write( writeData.data(), len ) );
  EXPECT_EQ( Status::OK, serial->read( readData.data(), len ) );
  EXPECT_EQ( 0, memcmp( writeData.data(), readData.data(), len ) );
}

TEST_F( SerialFixture, FlushTheToilet )
{
  static constexpr size_t len = 4;

  std::array<uint8_t, len> writeData = { 0x55, 0x33, 0x23, 0x99 };
  std::array<uint8_t, len> readData;
  readData.fill( 0 );

  EXPECT_EQ( Status::OK, serial->write( writeData.data(), len ) );
  EXPECT_EQ( true, serial->flush() );
  EXPECT_EQ( Status::EMPTY, serial->read( readData.data(), len ) );
  EXPECT_EQ( true, std::all_of( readData.begin(), readData.end(), []( auto i ) { return i == 0; } ) );
}

TEST_F( SerialFixture, ResetAllTheThings )
{
  static constexpr size_t len = 4;

  std::array<uint8_t, len> writeData = { 0x55, 0x33, 0x23, 0x99 };
  std::array<uint8_t, len> readData  = { 0x00, 0x00, 0x00, 0x00 };

  EXPECT_EQ( Status::OK, serial->write( writeData.data(), len ) );
  EXPECT_EQ( Status::OK, serial->read( readData.data(), len ) );
  EXPECT_EQ( 0, memcmp( writeData.data(), readData.data(), len ) );

  EXPECT_EQ( true, serial->reset() );
  serial->configure( 115200, CharWid::CW_8BIT, Parity::PAR_NONE, StopBits::SBITS_ONE, FlowControl::FCTRL_NONE );

  EXPECT_EQ( Status::OK, serial->write( writeData.data(), len ) );
  EXPECT_EQ( Status::OK, serial->read( readData.data(), len ) );
  EXPECT_EQ( 0, memcmp( writeData.data(), readData.data(), len ) );
}