/********************************************************************************
*   File Name:
*       chimeraPort.hpp
*
*   Description:
*       Defines Chimera namespaces and types needed for linking with the HAL
*
*   2019 | Brandon Braun | brandonbraun653@gmail.com
********************************************************************************/

#pragma once
#ifndef CHIMERA_PORT_HPP
#define CHIMERA_PORT_HPP

#include <bp_spi.hpp>

#ifndef SIM
#define SIM
#endif

#define CHIMERA_INHERITED_SPI HWInterface::BusPirate::BinarySPI


#endif  /* !CHIMERA_PORT_HPP */
