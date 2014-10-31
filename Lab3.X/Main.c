// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"
//#include "keypad.h"

#define XTFREQ          7372800         	  // On-board Crystal frequency
#define PLLMODE         4               	  // On-chip PLL setting (Fosc)
#define FCY             (XTFREQ*PLLMODE)/2    // Instruction Cycle Frequency (Fosc/2)

#define BAUDRATE         115200
#define BRGVAL          ((FCY/BAUDRATE)/16)-1
// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings.
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF &
		 BKBUG_ON & COE_OFF & ICS_PGx1 &
		 FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

// ******************************************************************************************* //
// Configuration bits for CONFIG2 settings.
// Make sure "Configuration Bits set in code." option is checked in MPLAB.

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
		 IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

// ******************************************************************************************* //

// Varible used to indicate that the current configuration of the keypad has been changed,
// and the KeypadScan() function needs to be called.


// ******************************************************************************************* //
volatile int state = 0;
volatile int count = 0;

int main(void)
{

    TRISBbits.TRISB5 = 1; // setting up the push button
    CNEN2bits.CN27IE = 1;
    IFS1bits.CNIF = 0; // set flag low
    IEC1bits.CNIE = 1; // enable interrupt

        // UART1 Setup
        RPINR18bits.U1RXR = 9;
	RPOR4bits.RP8R = 3;
        U1BRG  = BRGVAL;
	U1MODE = 0x8000;
        U1STA  = 0x0440;
	IFS0bits.U1RXIF = 0;

        IFS1bits.CNIF = 0;
        IEC1bits.CNIE = 1;


        PR3 = 1032; // 0.1 second delay
	TMR3 = 0;
        T3CON = 0x8000;
	IFS0bits.T3IF = 0;
//	IEC0bits.T3IE = 1;

        /// ADC

        int ADC_value;
        char value[8];
        double AD_value;

        AD1PCFG &= 0xFFDF;
        AD1CON2 = 0; // reference voltage
        AD1CON3 = 0x0101;  // sample conversion
        AD1CON1 = 0x20E4;  // sample conversion
        AD1CHS = 5; // AN5 pin for reference
        AD1CSSL = 0;

        IFS0bits.AD1IF = 0; // set flag low
        AD1CON1bits.ADON = 1; // turn on ADC

        /// PWM
        int PWM_Period = 1023;
        OC1CON = 0x000E;
        OC1CONbits.OCTSEL = 1;
        OC1R = PWM_Period;
        OC1RS = PWM_Period/2;
        RPOR1bits.RP2R = 18;
        RPOR5bits.RP10R = 18;
//        TRISBbits.TRISB10 = 0;

        OC2CON = 0x000E;
        OC2CONbits.OCTSEL = 1;
        OC2R = PWM_Period;
        OC2RS = PWM_Period/2;
        RPOR4bits.RP8R = 19;
        RPOR4bits.RP9R = 19;
//        TRISBbits.TRISB9 = 0;

        OC3CON = 0x000E;
        OC3CONbits.OCTSEL = 1;
        OC3R = PWM_Period;
        OC3RS = 0;

	LCDInitialize();

        int num_temp = 0;
        
        printf("Working\n");

	while(1)
	{
            while(IFS0bits.AD1IF == 0);
            IFS0bits.AD1IF = 0;
            ADC_value = ADC1BUF0; // digital value

            sprintf(value, "%6d", ADC_value); // convert digital value to string for LCD
            LCDMoveCursor(0,0);
            LCDPrintString(value);
            printf("%d\n", ADC_value);
            
///////////////////////////////////////////////////////////////////////////////
            switch(state){
                case 0: // forward state
                    if(ADC_value <= PWM_Period/2){
                        OC1RS = PWM_Period;
                     }
                    else if(ADC_value > PWM_Period/2){
                        OC1RS = PWM_Period - ((ADC_value * 2) - PWM_Period);
                    }

                    if(ADC_value >= PWM_Period/2){
                        OC2RS = PWM_Period;
                    }
                    else if(ADC_value < PWM_Period/2){
                        OC2RS = (ADC_value * 2);
                    }
                    RPOR1bits.RP2R = 18;
                    RPOR4bits.RP8R = 19;
                    RPOR5bits.RP10R = 20;
                    RPOR4bits.RP9R = 20;
                    break;                    
                case 1:  // IDLE state
                    OC1RS = 0;
                    OC2RS = 0;
                    break;
                case 2: // reverse state
                   if(ADC_value <= PWM_Period/2){
                        OC1RS = PWM_Period;
                     }
                    else if(ADC_value > PWM_Period/2){
                        OC1RS = PWM_Period - ((ADC_value * 2) - PWM_Period);
                    }

                    if(ADC_value >= PWM_Period/2){
                        OC2RS = PWM_Period;
                    }
                    else if(ADC_value < PWM_Period/2){
                        OC2RS = (ADC_value * 2);
                    }
                    RPOR5bits.RP10R = 18;
                    RPOR4bits.RP9R = 19;
                    RPOR1bits.RP2R = 20;
                    RPOR4bits.RP8R = 20;
//                    LATBbits.LATB2 = 0;
//                    LATBbits.LATB8 = 0;
                    
                    break;
                case 3:  // IDLE state
                    OC1RS = 0;
                    OC2RS = 0;
                    break;
            }
///////////////////////////////////////////////////////////////////////////////

            // TO DO  print the duty cycle of both PWM channels onto the LCD   convert a float to a string...
            LCDMoveCursor(1,0);
            num_temp = (int)(((float)OC1RS / (float)1023) * 100);
            sprintf(value, "%2d%%", num_temp);
            LCDPrintString(value);
//            LCDPrintString("%");
            LCDMoveCursor(1,7);
            num_temp = (int)(((float)OC2RS / (float)1023) * 100);
            sprintf(value, "%2d%%", num_temp);
            LCDPrintString(value);
//            LCDPrintString("%");

//            AD_value = (ADC_value*3.3)/1024;
//            sprintf(value, "%6.2f", AD_value); // convert to string
//            LCDMoveCursor(1,0);
//            LCDPrintString(value);


        }
		
	return 0;
}


void __attribute__((interrupt)) _CNInterrupt(void)
{
	IFS1bits.CNIF = 0;

       if(PORTBbits.RB5 == 0){
           while(PORTBbits.RB5 == 0);
           state++;
           if(state == 4){
               state = 0;
           }
       }
}

