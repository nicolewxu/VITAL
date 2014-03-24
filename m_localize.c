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




/////////////////////////////////////////////////////////////////////////////////////////////////
//					localization   
//////////////////////////////////////////////////////////////////////////////////////////////////

void localize(unsigned int* mWii_packet, float* localizationData){
	// SUBROUTINE VARIABLES
	int X[STARS] = {0}; 	int Y[STARS] = {0}; 
	long Dists[6]= {1};  	//  dists[0] = s0s1 [1] = s0s2  [2] = s0s3 [3] = s1s2 [4] = s1s3 [5] = s2s3 

	int s_map[STARS] = {0};	// star map...will probably get rid of in future versions 
	int avail_stars = 0;

	// fills X[] and Y[] with all of the good star readings
	int cnt;
	m_red(OFF);
	for (cnt = 0 ; cnt< STARS;cnt++){
		if (mWii_packet[cnt*3] == 1023){  /* m_red(ON);*/  }
		//else {
		X[avail_stars]=mWii_packet[ 3*cnt ]     -  X_BOT;
		Y[avail_stars]=mWii_packet[ 3*cnt + 1]  -  Y_BOT;
		avail_stars++;
		//}
	}	

	// computes distances between all available stars 
	int distIndex=0; 	int index;	 int other;

	for (index = 0; index< avail_stars ; index++){
		for(other = index+1; other < avail_stars; other ++){
			Dists[distIndex] = pow((X[index] - X[other]),2) + pow(( Y[index] - Y[other] ),2);
			distIndex++;
		}
	}
// finds largest inter star distance 
	int minIndex=-1;	 int maxIndex=-1;
	long  max = 0;		 long  min = 90000; 
	for (index = 0 ; index < 6 ; index++){
		if (Dists[index] < min)  { min  = Dists[index]; 	minIndex = index; }
		if ( Dists[index] > max ){ max  = Dists[index]; 	maxIndex  = index; }
	}

	/* 0 : 1	 0 : 2	 0 : 3	 1 : 2	 1 : 3	 2 : 3 */
	switch (maxIndex){
		case 0: switch (minIndex) {
				case 1: case 2:		s_map[1] = 0; s_map[3] = 1; break; 
				case 3: case 4: s_map[1] = 1; s_map[3] = 0; break; 
				default: m_red(ON); } break;
		case 1: switch (minIndex) {
				case 0: case 2: s_map[1] = 0; s_map[3] = 2; break;
				case 3: case 5:		s_map[1] = 2; s_map[3] = 0; break; 
				default: m_red(ON); } break;
		case 2: switch (minIndex) {
				case 0: case 1: s_map[1] = 0; s_map[3] = 3; break; 
				case 4: case 5: s_map[1] = 3; s_map[3] = 0; break;
				default: m_red(ON); } break;
		case 3: switch (minIndex) {
				case 0: case 4: s_map[1] = 1; s_map[3] = 2; break; 
				case 1: case 5: s_map[1] = 2; s_map[3] = 1; break;
				default: m_red(ON); }	break;
		case 4: switch (minIndex) {
				case 0: case 3: s_map[1] = 1; s_map[3] = 3; break;
				case 2: case 5: s_map[1] = 3; s_map[3] = 1; break;
				default: m_red(ON) } break;
		case 5: switch (minIndex){
				case 1: case 3: s_map[1]=2; s_map[3]=3; break;
				case 2: case 4: s_map[1] = 3; s_map[3] = 2; break;
				default: m_red(ON); } break;
		default: m_red(ON);	
	}

	float s_Vert = sqrt(max);  		// find the pixel dist b/w stars 2 & 4
	float scale  = s_Vert/29.0;		// find the ration of pixel/cm

	// find the location of global center in local coordinates.
	long x_cent, y_cent;
	x_cent = ( X[ s_map[1] ] + X[s_map[3]] ) / 2.0 ;		// define the local position of the origin
	y_cent = ( Y[ s_map[1] ] + Y[s_map[3]] ) / 2.0 ;		// define the local position of the origin

	// find the dist from bot to global center
	long r = (long) sqrt( (double)x_cent*x_cent +(double)y_cent*y_cent)/scale;

	// find the angle between the local frame and the global frame
	float theta = - atan2(  X[ s_map[1] ] - X[s_map[3]]  , Y[ s_map[1] ] - Y[s_map[3]] );

	// find the angle between the local x-axis and the vector to the global origin
	float alpha = atan2( y_cent , x_cent );

	// find the angle between the global x-axis and the vector to the origin
	float phi = -theta +alpha;

	float XBOT = r*cos(phi);		// determine the bot's global position
	float YBOT = -r*sin(phi);		// ^

	localizationData[0] =XBOT;// x_cent/scale;
	localizationData[1] =YBOT; //y_cent/scale;
	localizationData[2] = theta - PI/2;
}

void matlab_output( float* position, unsigned int* wii_data){
	if( m_usb_rx_available() ){
		m_usb_tx_int( ( int)position[0] );
		m_usb_tx_char('\n');
		m_usb_tx_int( ( int)position[1] );
		m_usb_tx_char('\n');
		m_usb_tx_int( ( int )(1000*position[2]) );
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[0]);
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[1]);
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[3]);
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[4]);
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[6]);
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[7]);
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[9]);
		m_usb_tx_char('\n');
		m_usb_tx_uint(wii_data[10]);
		m_usb_tx_char('\n');
		m_usb_rx_flush();
	}
}
