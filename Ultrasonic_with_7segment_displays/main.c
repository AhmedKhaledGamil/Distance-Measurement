
#include "stdint.h"
#include "tm4c123gh6pm.h"


/* Bit Specific Addressing */
#define displayed_number       			(*((volatile uint32_t *)0x4000703C))  // PD0 --> PD3
#define Enable_7_Segment_Displays       (*((volatile uint32_t *)0x40005070))  // PB2 --> PB4 // 3 7-segment dispaly
#define TRIG       						(*((volatile uint32_t *)0x40005008))  // Trig Pin --> PB1
/**/


void SystemInit(){};
	
double distance;
const double _16MHz_1clock = 62.5e-9;
uint32_t counts;
	
	
void PORTD_Init()
{
	volatile uint32_t delay;		// dummy variable	   
	SYSCTL_RCGCGPIO_R |= 0x8;     	// enable clock for PORT D
	delay = 20;           			// delay   
	GPIO_PORTD_LOCK_R = 0x4C4F434B; // unlock PORTD  
	GPIO_PORTD_CR_R = 0x0F;         // allow changes to PD0-3       
	GPIO_PORTD_AMSEL_R = 0;         // disable analog function
	GPIO_PORTD_PCTL_R = 0;   		// GPIO clear bit PCTL  
	GPIO_PORTD_DIR_R = 0xF;         // PD0->PD3 output   
	GPIO_PORTD_AFSEL_R = 0;         // no alternate function       
	GPIO_PORTD_DEN_R = 0xF;         // enable digital pins PD0-PD3        
}	

		
void PORTB_Init () 
{
	volatile uint32_t delay ;		// dummy variable
 	SYSCTL_RCGCGPIO_R |= 0x2;		// enable clock for PORT B
	delay = 20;						// delay
	SYSCTL_RCGCTIMER_R |= 0x4;		// enable clock for T2CCP0
	delay=20;						// delay
	GPIO_PORTB_LOCK_R = 0x4C4F434B ;
	GPIO_PORTB_CR_R = 0x1F;			// allow changes to PB0-4
	GPIO_PORTB_DIR_R = 0x1E;		// PB0 -> input , PB1-4 -> output
	GPIO_PORTB_AFSEL_R = 0x01;		// enable alternative function for PB0 only
	GPIO_PORTB_PCTL_R &= ~0x000fffff; 
	GPIO_PORTB_PCTL_R = 0x7;		// choose timer T2CCP0
	GPIO_PORTB_DEN_R = 0x1F  ;		// enable digital pins PB0-4
	GPIO_PORTB_AMSEL_R = 0   ;		// no alternative function
}

uint32_t Timer_T2CCP0_Init ()
{
	uint32_t time1=0 ,time2=0,difference=0 ;
	TIMER2_CTL_R &= ~0x01;						// disable timer
	TIMER2_CFG_R |= 0x04;						// Write the GPTM Configuration (GPTMCFG) register with a value of 0x0000.0004
	TIMER2_TAMR_R |= 0x17;						// configure for : count up - capture mode - input time mode
	TIMER2_CTL_R |= 0x0C;						// GPTM Timer A Event Mode : both edges
  	TIMER2_CTL_R |= 1;							// enable timer
	TIMER2_ICR_R |= 0x04;						// clear the flag

	while((TIMER2_RIS_R&0x04)==0){};			// wait until positive edge of the echo signal is arrived
	time1 = TIMER2_TAR_R;						// restore value of time
	TIMER2_ICR_R |= 0x04;						// clear the flag

	while((TIMER2_RIS_R&0x04)==0){};			// wait until the negative edge of the echo signal is arrived
	time2 = TIMER2_TAR_R;						// restore value of time

	difference= (time2-time1) & 0x00ffffff ;	// calculate the difference in time between both edges
	return difference;		
}	

void delay_systick()                    //  delay 10 microseconds
{
	NVIC_ST_CTRL_R = 0 ;       			// disable systick timer
	NVIC_ST_RELOAD_R  = 160 ;  			// start counting from 10 microseconds  // for 16 MHZ clock // number of counts = 160   
	NVIC_ST_CURRENT_R  = 0 ;       		// clear value in the current_r and clear the count flag
	NVIC_ST_CTRL_R = 5 ;				// enable systick timer
	while(!(NVIC_ST_CTRL_R&0x10000))	// wait until the flag in CTRL_R is set
	{};
}

void delay_times_systick(int times)     // delay times*10 microseconds
{
	uint32_t i = 0 ;
	for(i=0;i<times;i++)
	{
		delay_systick();
	}
}
	
void Print_distance_to_7segment()       // print distance(cm) by multiplixing between the three 7-segment displays
{
	int ones,tens,hundreds;
		
	hundreds = (int)distance / 100;		// calculate the third digit (hundreds)
	Enable_7_Segment_Displays = 0;   	// disable all displays
	Enable_7_Segment_Displays = 0x4;    // enable third display     
	displayed_number = hundreds;		// display the third digit (hundreds)
	delay_times_systick(1000);			// delay 10 milliseconds

		
	tens = distance / 10;
	tens = tens % 10;
	Enable_7_Segment_Displays = 0;
	Enable_7_Segment_Displays = 0x8;             
	displayed_number = tens;
	delay_times_systick(1000);
		
	ones = (int)distance % 10;
	Enable_7_Segment_Displays = 0;
	Enable_7_Segment_Displays = 0x10;             
	displayed_number = ones;
	delay_times_systick(1000);
}


int main ()
{
	PORTD_Init();
	PORTB_Init();
	while(1)
	{
		TRIG = 0;                // TRIG Low
		delay_times_systick(1);	 // 10 microseconds delay
		TRIG = 2;				 // TRIG High
		delay_times_systick(1);  // 10 microseconds delay
		TRIG = 0;				 // TRIG Low
		 
		distance =  _16MHz_1clock * Timer_T2CCP0_Init () * ( 34300.0 / 2.0 ) ;    // distance in centimeters 
	
		Print_distance_to_7segment();             // display the distance with accuracy of 1 cm

	}
}
	
	
	
	
	
	