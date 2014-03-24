/*********************************************
 *********************************************
 *
 *	RC robot
 *   RX - main onBoardM2.
 *********************************************
 *********************************************
 */
#include <avr/io.h>
#include "m_imu.h"
#include "m_bus.h"
#include "m_usb.h"
#include "m_general.h"
#include "m_rf.h"
#include <stdlib.h>
#include "m_wireless.h"

#define		ADCpins		2
#define 	p_length 	12
#define 	chan		1
#define	 	RX_add 		0x25
#define 	TX_add 		0x17
#define 	ADC_DELAY 	2000
#define 	min_motor 	200
#define 	hyster		55
#define 	tank_smooth	0.85
#define 	MK_drive	900
#define 	MK_reverse	600

#define 	Single_Joy	1
#define 	Double_Joy	11
#define 	Mario_Kart 	64
#define 	Tank_Mode	100

void setupUSB(void);
void TX_comm(void);
void debug_rf(void);

void setup_pins(void);
void setup_timer_1(void);
void setup_timer_3(void);
void set_motors( int , int );

void single_joystick(void);
void double_joystick(void);
void tank_driving(void);
void Mario_Drive(void);

volatile char 	new	 		= 	0;
char 		receive_buffer[p_length]=	{0};
char 		send_buffer[p_length]	=	{0};
/* receive_buffer[0] = leftB	 receive_buffer[1] = rightB
   receive_buffer[2&3] = VLadc	 receive_buffer[4&5] = VRadc
   receive_buffer[6&7] = HLadc	 receive_buffer[8&9] = HRadc
   receive_buffer[11]   	= playMode ( mk = 64 , sj = 1 , dj = 11 , key = 3 )

	if 	( L_bump && R_bump ) 	{ Mario 	= true;}
	else if (!L_bump && R_bump ) 	{ tank_mode 	= true;}
	else if ( L_bump && !R_bump ) 	{ double_joy 	= true;}
	else if ( !L_bump && !R_bump ) 	{ single_joy 	= true;}
*/
bool RF_debug = true ;

float motor_smooth = .25;
int lastRight=0;
int lastLeft =0;

int left = 0 ;
int right = 0 ;


int main(void) {
	m_clockdivide(0);
	setup_pins();
	if ( RF_debug) {setupUSB();}
	setup_timer_1();
	setup_timer_3();
	m_bus_init();
	m_rf_open(chan,RX_add,p_length);

	int timer_3_cnt = 0;

	//sei();
	set_motors(0,0);
	while (1){
		if (check(TIFR3,OCF3A)){
			set(TIFR3, OCF3A);
			timer_3_cnt++;
			if ( timer_3_cnt % 10 ==0 ) {
				timer_3_cnt=0;
				m_green(TOGGLE);
			}
			if ( timer_3_cnt == 1000){ timer_3_cnt=0;}
		}
		if(new){
			switch ( receive_buffer[11] ) {
				case Single_Joy: single_joystick(); break;
				case Double_Joy: double_joystick(); break;
				case Tank_Mode: tank_driving(); break;
				case Mario_Kart: Mario_Drive(); break;
				default : set_motors(0,0); m_green(2);
			}
			if(RF_debug){ debug_rf(); }
		}
	}
}

void single_joystick(){
	int left  = 0;
	int right = 0;

	int VLeft = *(int*)(&receive_buffer[2]);
	int HLeft = ((*(int*)(&receive_buffer[6])) * 3)/2;

	if 	(VLeft >  500 )	{ VLeft  =  999;}
	else if (VLeft < -500 )	{ VLeft  = -999;}
	else 		 	{ VLeft  = 2*VLeft;}


	if ( VLeft > 0){	// going forward
		if (HLeft > 0 ){ 	// joystick right
			right = VLeft - HLeft;
			left = VLeft;
		}else {			// joystick left (HLeft < 0 )
			left = VLeft + HLeft;
			right = VLeft;
		}
	}else if (VLeft < -50) { 	// going back (VLeft < 0)
		if ( HLeft > 0){ 	// joystick right ()
			right =VLeft + HLeft;
			left  = VLeft;
		}else{			// joystick left (HLeft < 0 )
			left = VLeft - HLeft;
			right = VLeft;
		}
	}else {
		if ( HLeft > 0){ 	// joystick right ()
			right =VLeft ;
			left  = VLeft + HLeft;
		}else{			// joystick left (HLeft < 0 )
			right = VLeft - HLeft;
			left = VLeft;
		}
	}
	set_motors(left, right);
}

void double_joystick(){
	int left  = 0;
	int right = 0;

	int VLeft  =( *(int*)(&receive_buffer[2]));
	int HRight = ((*(int*)(&receive_buffer[8])) * 3 )/ 2;
//	int  = (*(int*)(&receive_buffer[8]));

	if 	(VLeft >  500 )	{ left  =  999;}
	else if (VLeft < -500 )	{ left  = -999;}
	else 		 	{ left = 2*VLeft;}

	right = left;

	if ( left >  0 ){ 	// going forward
		if (HRight> 0){ 	// joystic is to the right
			right -= HRight;
		}else {			// joystic is to the left
			left  += HRight;
		}
	}else if (left< -50) {		 	// going backword  ( left < 0)
		if (HRight> 0){		// joystic is to the right
			right += HRight;
		}else {			// joystic is to the left
			left  -= HRight;
		}
	}else {
		if ( HRight > 0){ 	// joystick right ()
			right =VLeft ;
			left  = VLeft + HRight;
		}else{			// joystick left (HLeft < 0 )
			right = VLeft - HRight;
			left = VLeft;
		}
	}

	set_motors(left, right);
}

void tank_driving(){
	int left  = 0;
	int right = 0;

	int VLeft  =( *(int*)(&receive_buffer[2]));
	int VRight = (*(int*)(&receive_buffer[4]));

	if 	(VLeft >  500 )	{ left  =  999;}
	else if (VLeft < -500 )	{ left  = -999;}
	else 		 	{ left = 2*VLeft;}

	if 	( VRight>  500 ){ right  =  999;}
	else if ( VRight < -500 ){ right  = -999;}
	else 		 	{ right = 2*VRight;}

	int Tleft = left ;
	left = (int) ( (tank_smooth * (float)left) + ((1.0-tank_smooth) * (float)right) );
	right   =  (int) ( (tank_smooth * (float)right) + ((1.0-tank_smooth) * (float)Tleft) ) ;
	set_motors(left, right);
}

void Mario_Drive(){
	int HLeft =  ((*(int*)(&receive_buffer[6])) * 3 )/ 2;
	if (receive_buffer[1]==1) // right
		{ left = -MK_reverse ; right = -MK_reverse;}
	else if (receive_buffer[0]==1 ) // left
		{ left = MK_drive ; right = MK_drive;}
	else {left = 0; right =0; set_motors(0,0); return;}


	if ( left > -60 ){ 	// going forward
		if (HLeft > 0){ 	// joystic is to the right
			right -= HLeft;
		}else {			// joystic is to the left
			left  += HLeft;
		}
	}else {		 	// going backword  ( left < 0)
		if (HLeft > 0){		// joystic is to the right
			right += HLeft;
		}else {			// joystic is to the left
			left  -= HLeft;
		}
	}
	set_motors(left,right);

}
void set_motors( int left, int  right ){
	int sentL = (int)((float) left * motor_smooth ) +  (int)((float) lastLeft *(1.0 -  motor_smooth) );
	int sentR = (int)((float) right * motor_smooth ) +  (int)((float) lastRight *(1.0 -  motor_smooth) );

	lastLeft = sentL;
	lastRight = sentR;
	if	( sentR < -hyster )	{ OCR1B = -sentR + min_motor; }
	else if ( sentR > hyster  )	{ OCR1B  = sentR +  min_motor; }
	else {OCR1B = 0 ;}

	if 	( sentL < -hyster ) { OCR1C = -sentL + min_motor; }
	else if ( sentL >  hyster ) { OCR1C  = sentL + min_motor; }
	else {OCR1C = 0 ;}

	if 	(sentL >= hyster)	{   set(PORTC,6); }
	else if (sentL < -hyster)	{ clear(PORTC, 6); }

	if 	(sentR >= hyster)	{   set(PORTC,7); }
	else if (sentR < -hyster)	{ clear(PORTC, 7); }


}

void debug_rf(void){
	new = 0;
	int i; int a;
	switch ( receive_buffer[11] ){
		case Single_Joy :
			 m_usb_tx_string("Single Joystick Driving\t");
			break;
		case Double_Joy :
			m_usb_tx_string("Double Joystick Driving\t");
			break;
		case Mario_Kart :
			m_usb_tx_string("Mario Kart RACING!\t");
			break;
		case Tank_Mode:
			m_usb_tx_string("TANK MODE! \t");
			break;
		default:
			m_usb_tx_string("No mode...? \t");
	}

	m_usb_tx_string("Button R:\t");
	if (receive_buffer[0]==1)	{m_usb_tx_string(" on") ;}
	else 				{m_usb_tx_string("off");}
	m_usb_tx_string("\t");

	m_usb_tx_string("Button L:\t");
	if (receive_buffer[1]==1)	{m_usb_tx_string(" on"); }
	else 				{m_usb_tx_string("off");}
//	m_usb_tx_string("\t");

	for (i=0; i < 4 ; i++){
		m_usb_tx_string("\tpot: ");
		m_usb_tx_int(i*2+1);
		m_usb_tx_string("\t");
		a = *(int*)&receive_buffer[i*2+2];
		m_usb_tx_int(a);
	}
	m_usb_tx_string("\tL Motor =: ");
	m_usb_tx_int((int)OCR1B);
	m_usb_tx_string("\tR Motor =: ");
	m_usb_tx_int((int)OCR1C);
	m_usb_tx_string("\r\n");
}


void setupUSB(){
	m_usb_init();
	m_green(ON);
	while (!m_usb_isconnected()){m_wait(1);}
	m_green(OFF);
}


void  setup_timer_1(){
	OCR1A = 1015;		OCR1B=0; 		OCR1C=0;
	// Setting up clock divider
	clear(TCCR1B,CS12); 	set(TCCR1B,CS11); 	set(TCCR1B, CS10);
	// Set mode15 on timer 1
	set(TCCR1B,WGM13); 	set(TCCR1B,WGM12); 	set(TCCR1A,WGM11);	 set(TCCR1A,WGM10);
	// Match b/w TCNT1 and OCR1x clears, set at rollover
	set(TCCR1A,COM1B1); 	clear(TCCR1A,COM1B0);
	set(TCCR1A,COM1C1); 	clear(TCCR1A,COM1C0);
}

void  setup_timer_3(){

	OCR3A =625; 				//1600 /625/ 256  >> 100Hz clock

	set(TCCR3B, CS32); 	clear(TCCR3B, CS31); clear(TCCR3B, CS31);	// <-- clock Prescaller 16MHz/1024
	clear(TCCR3B, WGM33);	set(TCCR3B, WGM32);
	clear(TCCR3A, WGM31);	clear(TCCR3A, WGM30);	//(mode 4) UP to OCR3A

}


void setup_pins(){
	set(DDRC,6);		set(DDRC,7);		//setup motor direction lines as outputs
	clear(PORTC,6);		clear(PORTC,7);		//set default motor dir to be forward
	set(DDRB,6); 		set(DDRB,7); 		// Controll B6 (motors) with the timer
}

ISR(INT2_vect){
	m_red(2);
	m_rf_read(receive_buffer , p_length);
	new = 1;
}
