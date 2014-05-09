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

    // Initialize latch bits
    PORTBbits.RB5 = 0; // Demultiplexer
    PORTBbits.RB4 = 1; // Decoder bcd->7-segment
    PORTBbits.RB7 = 0; // IR selection #A
    PORTBbits.RB6 = 0; // IR selection #B

    TRISA = 0b111; // Inputs RA0-2
    TRISB = TRISC = 0x0; // Outputs

    ANSEL = 0; // No analog inputs
    ANSELH = 0;

    /* Initialize peripherals */
    T0CONbits.TMR0ON = 1; // Timer 0 on
    T0CONbits.T08BIT = 1; // 8bit timer
    T0CONbits.T0CS = 0; // Follow instruction clock
    T0CONbits.PSA = 1; // No prescaler for this clock

    T1CONbits.RD16 = 0; // 8bit timer
    T1CONbits.TMR1ON = 0; // Timer 1 is off
    T1CONbits.T1CKPS = 0b11; // 1:8 prescaled
    T1CONbits.TMR1CS = 0; // Internal clock
    T1CONbits.T1OSCEN = 0; // Oscillator

    T2CONbits.TMR2ON = 0; // Timer 1 is off
    T2CONbits.T2CKPS = 0b11; // 1:16 prescaled
    T2CONbits.T2OUTPS = 0b0000; // 1:12 postscaler

    T3CONbits.RD16 = 1; // 16bit timer
    T3CONbits.TMR3ON = 0; // Timer 1 is off
    T3CONbits.T3CKPS = 0b11; // 1:8 prescaled
    T3CONbits.TMR3CS = 0; // Internal clock

    /* Configure the IPEN bit (1=on) in RCON to turn on/off int priorities */
    RCONbits.IPEN = 0; // No interrupt priorities

    /* Enable interrupts */
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.INT0E = 1; // Enable the INT0 interrupt
    INTCON3bits.INT2E = 1; // Enable the INT2 interrupt
    INTCON3bits.INT2IP = 1; // INT2 interrupt is high priority
    INTCONbits.TMR0IE = 1; // Timer 0 interrupt
    PIE1bits.TMR1IE = 1; // Timer 1 interrupt
    PIE1bits.TMR2IE = 1; // Timer 2 interrupt
    PIE2bits.TMR3IE = 1; // Timer 3 interrupt
    
    INTCON2bits.INTEDG0 = 1; // Interrupt on raising edge for RA0
    INTCON2bits.INTEDG2 = 0; // Interrupt on falling edge for RA2
    
    INTCON2bits.TMR0IP = 1; // High priority for timer0
    INTCONbits.GIE = 1; // Enables all unmasked interrupts
}

void printToBoard(char* BoardNumbers, uint16_t number) {
    BoardNumbers[3] = number % 10;
    BoardNumbers[2] = (number / 10) % 10;
    BoardNumbers[1] = (number / 100) % 10;
    BoardNumbers[0] = (number / 1000) % 10;
}

uint32_t getNumberFromIRResultArray(char* IRBitsTimings, char IRBitsTimingsCounter) {
    if (IRBitsTimingsCounter < 33) {
        return 0;
    }

    uint32_t address = 0;
    uint32_t command = 0;
    uint8_t startIndex = 1;
    uint8_t min = 255, max = 0, mean = 0;

    // 32-bits expected
    for(uint8_t n = startIndex; n < startIndex + 32; ++n)
    {
        if (max < IRBitsTimings[n])
        {
            max = IRBitsTimings[n];
        }

        if (min > IRBitsTimings[n])
        {
            min = IRBitsTimings[n];
        }
    }

    mean = (max + min) / 2;

    // Address / LSB first
    for(uint8_t n = startIndex; n < startIndex + 16; ++n)
    {
        bool isOne = IRBitsTimings[n] > mean;
        if (isOne) {
            address |= (1 << (startIndex + 15 - n));
        }
    }
    
    // Command / LSB first
    for(uint8_t n = startIndex + 16; n < startIndex + 32; ++n)
    {
        bool isOne = IRBitsTimings[n] > mean;
        if (isOne) {
            command |= (1 << (startIndex + 31 - n));
        }
    }

    return (address << 16) | command;
}