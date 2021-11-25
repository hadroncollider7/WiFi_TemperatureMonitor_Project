#include <msp430.h>
#include <math.h>

/**
 * main.c
 */

volatile unsigned int i = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    /*************************************************************************
    * SETUP ADC
    * The conversion will be stored in the ADC12MEM0 register.
    * Note: The ADC12IEs and ADC12IFGs corresponds to the MEMx (we're using
    *   MEM0) registers, not the ADC input channel.
    * Module ADC12_A
    * Analog channel A6
    * Test: send data to WiFi
    * Clock select: ADC12OSC (MODCLK), 5 Mhz
    * Resolution of the conversion result: 8-bits
    *************************************************************************/
    /************************************************************************
     * Configure ports
     ************************************************************************/
    P6SEL |= BIT6;      // Port 6.6 set to analog function

    //************* SET CONTROL REGISTERS ***************
    REFCTL0 &= ~REFMSTR;     // clear REFMSTR bit to give control of the reference generator to ADC12
    ADC12CTL0 |= ADC12REF2_5V + ADC12REFON + ADC12ON + ADC12SHT0_2;     // SHT = 16 cycles
    ADC12CTL1 |= ADC12SHS_0 + ADC12SHP;
        // sample hold source is ADC12SC bit
        // Sample and hold pulse mode

    ADC12MCTL0 |= ADC12INCH_6;      /* Analog 6 input channel select */
    ADC12MCTL0 |= ADC12SREF_1;      /* V+ = Vref, V- = AVSS */
    ADC12IFG &= ~ADC12IFG0;         /* Clear ADC12MEM0 interrupt request flag */
    ADC12IE |= ADC12IE0;            /* Enable interrupt for the ADC12MEM0 register */

    volatile unsigned int j;
    j = 0x30;
    do j--;
    while (j != 0);                 /* Delay for reference start-up */

    ADC12CTL0 |= ADC12ENC;          /* Enable ADC conversion */

    /************************************************************************
     * SETUP UP USCI REGISTERS
     * Data format: 115200N81
     * Low frequency mode
     * BRCLK = 1.048576 Mhz
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

    /**************************************************************************
     * TIMER A0 MODULE
     * Description: Set up a timer so that the microcontroller monitors
     *  the temperature every ten minutes.
     * Clock: ACLK, 32768 Hz.
     *  ACLK will be divided by 8 to get a clock frequency of 4096 Hz.
     **************************************************************************/
    TA0CTL |= TASSEL_1+ID_3+TAIE;
    TA0CTL |= TACLR;        /* Clears TAR and divider logic (divider settings
                                unchanged. Automatically resets */
    TA0CTL &= ~TAIFG;       /* Clear interrupt request flag */

    __bis_SR_register(GIE);
    __no_operation();

    /* TAKE INITIAL MEASUREMENT BEFORE STARTING TIMER */

    /*************************************************************************
     * ACTIVATE UART
     * Transmit data via the Transmit Buffer Register UCA0TXBUF
     *************************************************************************/
    UCA0CTL1 &= ~UCSWRST;     /* Enable USCI*/

    //********************** START CONVERSION ********************
    ADC12CTL0 |= ADC12SC;           /* Start conversion */
        j = 0x30;
        do j--;
        while (j != 0);             /* Time delay */
//    while (!(ADC12IFG & BIT0));     /* Wait for conversion to finish */
    __no_operation();

    /* BEGIN TIMER AND ENTER LOW POWER MODE 0 */
    TA0CTL |= MC_2;
//    __bis_SR_register(LPM0);
//    __no_operation();
    while(1);

}

/******************************************************************************
 * TIMER A0 INTERRUPT SERVICE ROUTINE
 ******************************************************************************/
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
    if (i >= 36)     // i >= 36 for normal operation
    {
        /* Let 8 seconds pass to reach 10 minutes
         * Note: There is not enough bits to count to 8 seconds.
         * The following lines of code need to be fixed.*/
        volatile unsigned int j;
        j = 0xFFFF;
        do j--;
        while(j != 0);
        i = 0;      /* Reset i */

        /* Take and measurement, convert to digital, and send to WiFi module */


        //************ ENABLE INTERRUPTS AND START CONVERSION ********************

        ADC12CTL0 |= ADC12SC;           /* Start conversion */
            j = 0x30;
            do j--;
            while (j != 0);             /* Time delay */
//        while (!(ADC12IFG & BIT0));     /* Wait for conversion to finish */
        __no_operation();
        TA0CTL &= ~TAIFG;   /* Clear TA0 interrupt request flag */
    }
    else
    {
        i++;
        TA0CTL &= ~TAIFG;   /* Clear TA0 interrupt request flag */
    }
}


/**************************************************************************
 *  ADC12 INTERRUPT SERVICE ROUTINE
 ***************************************************************************/
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    /* Convert ADC to Celsius */
    const double R0 = 9750;       /* Resistance at reference temperature */
    const double B = 3936;        /* Datasheet coefficient */
    const double T0 = 298.15;     /* Reference temperature */
    double R;
    double T_Kelvin;
    double T_Celcius;
    double T_Fah;

    R = (3.3*10000*pow(2,12))/(ADC12MEM0*2.5) - 10000;
    T_Kelvin = (T0*B)/(T0*log(R/R0)+B);
    T_Celcius = T_Kelvin - 273.15;
    T_Fah = T_Celcius*9/5 + 32;

    /**********************************************************
     * Convert number into ASCII
     * Two digits, decimals dropped.
     * Temperature range: 0 - 99 degrees Fahrenheit
     * Error: +/- 1 degree
     *********************************************************/
    unsigned int digit_2;
    unsigned int digit_1;
    digit_2 = T_Fah/10;
    digit_1 = T_Fah - digit_2*10;

    switch (digit_2)
    {
        case 0:
            digit_2 = 0x30;
            break;
        case 1:
            digit_2 = 0x31;
            break;
        case 2:
            digit_2 = 0x32;
            break;
        case 3:
            digit_2 = 0x33;
            break;
        case 4:
            digit_2 = 0x34;
            break;
        case 5:
            digit_2 = 0x35;
            break;
        case 6:
            digit_2 = 0x36;
            break;
        case 7:
            digit_2 = 0x37;
            break;
        case 8:
            digit_2 = 0x38;
            break;
        case 9:
            digit_2 = 0x39;
            break;
        default:
            digit_2 = 0x36;
    }

    switch (digit_1)
        {
        case 0:
            digit_1 = 0x30;
            break;
        case 1:
            digit_1 = 0x31;
            break;
        case 2:
            digit_1 = 0x32;
            break;
        case 3:
            digit_1 = 0x33;
            break;
        case 4:
            digit_1 = 0x34;
            break;
        case 5:
            digit_1 = 0x35;
            break;
        case 6:
            digit_1 = 0x36;
            break;
        case 7:
            digit_1 = 0x37;
            break;
        case 8:
            digit_1 = 0x38;
            break;
        case 9:
            digit_1 = 0x39;
            break;
        default:
            digit_1 = 0x36;
        }

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = digit_2;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = digit_1;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0x3B;
    while (!(UCA0IFG & UCTXIFG));
    ADC12IFG &= ~ADC12IFG0;
}
