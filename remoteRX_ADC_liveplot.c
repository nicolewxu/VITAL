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
void usb_tx(void);

void setup_pins(void);
void setup_timer_1(void);
void setup_timer_3(void);


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

int left = 0 ;
int right = 0 ;


int main(void) {
	m_clockdivide(0);
	setup_pins();
	setupUSB();
	setup_timer_1();
	setup_timer_3();
	m_bus_init();
	m_rf_open(chan,RX_add,p_length);
	sei();
	int timer_3_cnt = 0;

	//sei();
	while (1){
		if (check(TIFR3,OCF3A)){
			set(TIFR3, OCF3A);
			timer_3_cnt++;
			if ( timer_3_cnt % 10 ==0 ) {
				timer_3_cnt=0;
				// m_green(TOGGLE);
			}
			if ( timer_3_cnt == 1000){ timer_3_cnt=0;}
		}
		if(new & m_usb_rx_available()){
			char rx_buffer = m_usb_rx_char();  	//grab the computer packet
			m_usb_rx_flush();  				//clear buffer
			new = 0;
			usb_tx();
		}
	}
}

void usb_tx(void){
	int i; int a;
	m_green(2)
	// m_usb_tx_string("Button R:\t");
	// if (receive_buffer[0]==1)	{m_usb_tx_string(" on") ;}
	// else 				{m_usb_tx_string("off");}
	// m_usb_tx_string("\t");

	// m_usb_tx_string("Button L:\t");
	// if (receive_buffer[1]==1)	{m_usb_tx_string(" on"); }
	// else 				{m_usb_tx_string("off");}
//	m_usb_tx_string("\t");

	for (i=0; i < 4 ; i++){
		// m_usb_tx_string("\tpot: ");
		// m_usb_tx_int(i*2+1);
		a = *(int*)&receive_buffer[i*2+2];
		m_usb_tx_int(a);
		m_usb_tx_char('\t');
	}
	m_usb_tx_char('\n');
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
