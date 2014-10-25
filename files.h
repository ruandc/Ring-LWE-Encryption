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

#ifndef __FILES_H_
#define __FILES_H_
int read_hex_file(char * file_name, uint32_t * out, int max_size);
int write_hex_file(char * file_name, uint32_t * output, int len);
void read_binary_file(char *name, char * buffer); //unused?
#endif
