#include <msp430.h>


/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    /*************************************************************************
    * SETUP ADC
    * The conversion will be stored in the ADC12MEM0 register.
    * Note: The ADC12IEs and ADC12IFGs corresponds to the MEMx registers, not
    *   the ADC input channel.
    * Using module ADC12 A0
    * Input channel: 6
    * Test: send data to wifi
    *************************************************************************/
    /************************************************************************
     * Configure ports
     ************************************************************************/
    P6SEL |= BIT6;      // Port 6.6 set to analog function
    P6DIR &= ~BIT6;     // Configure p6.6 as input


    //************* SET CONTROL REGISTERS ***************
    REFCTL0 &= ~BIT7;     // clear REFMSTR bit to give control of the reference generator to ADC12
    ADC12CTL0 |= ADC12REF2_5V + ADC12REFON + ADC12ON + ADC12SHT0_2;     // SHT = 16 cycles
    ADC12CTL0 |= ADC12ENC;                                              // enable conversion
    ADC12CTL1 |= ADC12SHS0 + ADC12SSEL1 + ADC12SHP;
        // select ACLK
        // sample hold source is ADC12SC bit
        // ample and hold pulse mode

    ADC12MCTL6 |= ADC12INCH_6;      /* Analog 6 input channel select */
    ADC12IFG &= ~ADC12IFG0;         /* Clear ADC12MEM0 interrupt request flag */
    ADC12IE |= ADC12IE0;            /* Enable interrupt for the ADC12MEM0 register */



    /************************************************************************
     * SETUP UP USCI REGISTERS
     * Data format: 115200N81
     * Low frequency mode
     * BRCLK ~ 1 Mhz
     * TXO: P3.3
     * RXI: P3.4
     * Asynchronous mode
     ***********************************************************************/
    P3SEL |= BIT3 + BIT4;   /* configure p3.3, p3.4 to function as tx,rx */

    /* Configure control registers */
    UCA0CTL1 |= UCSWRST;    /* enable configuration of control registers */
    UCA0CTL1 |= UCSSEL_2;   /* BRCLK = SMCLK */


    /* ********************************************************************
     * Configure the baud rate for UART
     * 115200
     * UCOS16 = 0 (low-frequency mode)
     * UCBRX = 9
     * UCBRSX = 1
     * ********************************************************************/
    UCA0BR0 = 0x9;          /* low-byte BRCLK prescaler */
    UCA0MCTL |= UCBRS_1;    /* sets BRS */


    /*************************************************************************
     * Activate UART
     * Transmit data via the Transmit Buffer Register UCA0TXBUF
     *************************************************************************/
     UCA0CTL1 &= ~UCSWRST;     /* Enable USCI*/

//     UCA0TXBUF = 0x66;

     //************ ENABLE INTERRUPTS AND START CONVERSION ********************
     ADC12CTL0 |= ADC12SC;          // start conversion

     __bis_SR_register(GIE);
     __no_operation();              /* for debugger */

    return 0;
}

/**************************************************************************
 *  ADC12 ISR
 ***************************************************************************/
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    UCA0TXBUF = 0xA;
    ADC12IE &= ~ADC12IFG0;
}
