// registrar.cpp	A (static) object for maintaining object serial numbers
//			for classes that care about such things.  This particulary
//			useful for random number generators, so they don't always
//			generate the same sequence when declared as automatics
//			The registrar object is instantiated in the '.cpp' file
//			the user just calls the serial_number method:
//
//	int my_id = registrar.serial_number();
//
// registrar.c++

static const char rcsid[] = "@(#)registrar.c++	1.1 13:43:58 10/14/93   EFC";

#include "registrar.h"

unsigned int Registrar::seq_no = 0;

Registrar registrar;

Registrar::Registrar()
{
	++seq_no;
}

unsigned int Registrar::serial_number()
{
	return seq_no++;
}

