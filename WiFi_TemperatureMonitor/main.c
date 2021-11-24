#include <msp430.h>


/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    /************************************************************************
     * SEUP UP USCI REGISTERS
     * Data format: 115200N81
     * Low frequency mode
     * BRCLK ~ 1 Mhz
     ***********************************************************************/
    P3SEL |= BIT3 + BIT4;   // Congfigure p3.3, p3.4 as tx,rx
    P3DIR |= BIT3 + BIT4;   // configure ports as outputs

    /* CONFIGURE CONTROL REGISTERS */
    UCA0CTL1 |= UCSWRST;    // enable configuration of control registers
    UCA0CTL0 |= UCSYNC;      // asynchronous mode
    UCA0CTL1 |= UCSSEL_2;       // BRCLK = SMCLK

    /* Set the baud rate */



//  //*************** CONFIGURE PORTS *********************************
//  P6SEL |= BIT6;     // Port 6.6 set to analog function
//  P6DIR &= ~BIT6;     // Configure p6.6 as input
//
//
//  //************* SET CONTROL REGISTERS ***************
//  REFCTL0 &= ~BIT7;     // clear REFMSTR bit to give control of the reference generator to ADC12
//  ADC12CTL0 |= ADC12REF2_5V + ADC12REFON + ADC12ON + ADC12SHT0_2; // SHT = 16 cycles
//  ADC12CTL0 |= ADC12ENC;  // enable conversion
//  ADC12CTL1 |= ADC12SHS0 + ADC12SSEL1 + ADC12SHP;
//                  // select ACLK
//                  // sample hold source is ADC12SC bit
//                  // ample and hold pulse mode
//
//  ADC12MCTL6 |= ADC12INCH_6;      // analog 6 input channel select
//    ADC12IE |= BIT6;        // enable interrupt for ADDC12IFG6
//
//
//  //************ ENABLE INTERRUPTS AND START CONVERSION ********************
//  while(1)
//  {
//      ADC12CTL0 |= ADC12SC;                  // start conversion
//
//      __bis_SR_register(GIE);
//  }


    return 0;
}

//******************** ADC12 ISR *****************************************
//#pragma vector = ADC12_VECTOR
//__interrupt void ADC12_ISR(void)
//{
//
//}
