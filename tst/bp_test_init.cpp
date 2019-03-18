/********************************************************************************
 *  File Name:
 *    bp_test_init.cpp
 *  
 *  Description:
 *    Tests init/de-init basic 
 *  
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include "bp_test_fixtures.hpp"


TEST( BPInit, WillItOpen )
{
  auto bp = HWInterface::BusPirate::Device( BUS_PIRATE_PORT );
  EXPECT_EQ( true, bp.open() );
}

TEST( BPInit, WillItOpenAndClose )
{
  auto bp = HWInterface::BusPirate::Device( BUS_PIRATE_PORT );

  EXPECT_EQ( true, bp.open() );
  bp.close();
  EXPECT_EQ( true, bp.open() );
  bp.close();
}