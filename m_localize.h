#ifndef LOCALIZE__
#define LOCALIZE__

#include "m_general.h"
#include "m_wii.h"
#include <stdlib.h>
#include <math.h>

///////////////////////////////////////////////////////////////
////		localization stuff
///////////////////////////////////////////////////////////////
 // vars
	char scale = 1; 
	
 // funcitons
	void localize(unsigned int*, float*);
	void matlab_output( float* , unsigned int*);


#endif
