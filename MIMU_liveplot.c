 
/********************** ADC Live Plotting & Logging Code ****************************
	Sends ADC value from pin F0 in hex from the M2 microcontroller [http://medesign.seas.upenn.edu/index.php/Guides/MaEvArM].

	By Nick McGill [nmcgill.com]
************************************************************************************/

// header files
#include "m_general.h"
#include "m_usb.h"
#include "m_imu.h"
#include "m_bus.h"

// subroutines
void set_ADC(void);
void update_ADC(void);

// global variables
#define DEBUG 1
#define CLOCK 0
int data[9]={0};

int main(void){

	m_red(ON);
	m_green(ON);

	m_bus_init();
	m_imu_init(0 ,2 );
	m_usb_init(); // connect usb
	while(!m_usb_isconnected()){};  //wait for connection

	m_red(OFF);
	m_green(OFF);

	char rx_buffer; //computer interactions
  
	int i;
	while(1){
	
		m_red(ON);
		m_green(OFF);
		m_imu_raw(data);
		m_red(OFF);
		m_green(ON);

		while(!m_usb_rx_available());  	//wait for an indication from the computer
		rx_buffer = m_usb_rx_char();  	//grab the computer packet

		m_usb_rx_flush();  				//clear buffer		

		if(rx_buffer == 1) {  			//computer wants ir data
			//write ir data as concatenated hex:  i.e. f0f1f4f5
			for (i = 0 ; i < 9 ; i++){
				m_usb_tx_int(data[i]);
				m_usb_tx_char('\t');

			}
			m_usb_tx_char('\n');  //MATLAB serial command reads 1 line at a time
		}
	}
}
