/********************************************************************************
*   File Name:
*       main.cpp
*
*   Description:
*       Entry point for the BusPirate test suite
*
*   2019 | Brandon Braun | brandonbraun653@gmail.com
********************************************************************************/
#include <gtest/gtest.h>

using namespace ::testing;

int main(int argc, char **argv)
{
  InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG( filter ) = "BinarySPIFixture.Write*";
  return RUN_ALL_TESTS();
}
