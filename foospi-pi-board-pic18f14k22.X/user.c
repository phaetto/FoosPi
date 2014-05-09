/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#elif defined(__18CXX)
    #include <p18cxxx.h>    /* C18 General Include File */
#endif

#if defined(__XC) || defined(HI_TECH_C)

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#endif

#include "user.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    /* Setup analog functionality and port direction */
    PORTA = PORTB = PORTC = 0x0; // Set to false

    // RA0-1 : Phototrans input
    // RA2: Clock sync input

    TRISA = 0b111; // Inputs RA0-2
    TRISC = 0b110; // Inputs RC1-2; RC0 is pi-protocol output
    TRISB = 0x0; // Outputs

    ANSEL = 0x0; // No analog inputs
    ANSELH = 0x0;

    /* Configure the IPEN bit (1=on) in RCON to turn on/off int priorities */
    RCONbits.IPEN = 0; // No interrupt priorities

    T0CONbits.TMR0ON = 0; // Timer 0 off
    T0CONbits.T08BIT = 0; // 16bit timer
    T0CONbits.T0CS = 0; // Follow instruction clock
    T0CONbits.PSA = 1; // No prescaler for this clock

    /* Enable interrupts */
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.INT0F = 0;
    INTCONbits.INT0E = 1; // Enable the INT0 interrupt
    INTCON3bits.INT1F = 0;
    INTCON3bits.INT1E = 1; // Enable the INT1 interrupt
    INTCON3bits.INT2F = 0;
    INTCON3bits.INT2E = 1; // Enable the INT2 interrupt
    INTCONbits.TMR0IE = 1; // Enable timer-0 interrupt

    INTCON2bits.INTEDG0 = 1; // Interrupt on raising edge for RA0
    INTCON2bits.INTEDG1 = 1; // Interrupt on raising edge for RA1
    INTCON2bits.INTEDG2 = 1; // Interrupt on raising edge for RA2

    INTCONbits.GIE = 1; // Enables all unmasked interrupts
}

