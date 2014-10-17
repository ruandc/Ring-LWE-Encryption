/*****************************************************************************
* Copyright 2014 Katholieke Universiteit Leuven
*
* Use of this software is governed by the GNU LGPLv3.0 license
*
* Written by Ruan de Clercq, Sujoy Sinha Roy,
* Frederik Vercauteren, and Ingrid Verbauwhede
*    ______      _____ _____ ______
*   / ____/___  / ___//_  _// ____/
*  / /   / __ \ \__ \  / / / /
* / /___/ /_/ /___/ /_/ /_/ /___
* \____/\____//____//____/\____/
*
* Computer Security and Industrial Cryptography (COSIC)
* K.U.Leuven, Departement Electrical Engineering,
* Celestijnenlaan 200A, B-3001 Leuven, Belgium
****************************************************************************/

#include "stdint.h"
#include "global.h"
#define attribute_fixed_data1 __attribute__((section(".fixed_data1")));
uint32_t fixed_data1[M/2] attribute_fixed_data1;
uint32_t fixed_data2[M/2] attribute_fixed_data1;
uint32_t fixed_data3[M/2] attribute_fixed_data1;
