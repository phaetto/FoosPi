/******************************************************************************/
/*Files to Include                                                            */
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
#include "user.h"

#endif

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

#define COMMUNICATION_RESET 0x0
#define COMMUNICATION_RED 0x1
#define COMMUNICATION_BLUE 0x2
#define COMMUNICATION_ZERO 0x3
#define COMMUNICATION_RESET 0x4

char CommunicationCounter = 0;
char CommunicationBitsReceived[4] = { 0, 0, 0, 0 };
char CommunicationActionHeader = 0x0;
int8_t CommunicationScoreChangeRequest = 0;
bool CommunicationHasSynced = false;

char BoardCounter = 0;
char BoardNumbers[4] = { 0, 0, 0, 0 }; /* 0,1: Red, 2,3: Blue */

bool IRCanInit = false;
#define IRBitsTimingsSize 33
uint8_t IRBitsTimings[IRBitsTimingsSize];
uint8_t IRBitsTimingsSizeCounter = 0;
bool IRIsOn = false;

char Timer3Counter = 0;

/* High-priority service */

#if defined(__XC) || defined(HI_TECH_C)
void interrupt high_isr(void)
#elif defined (__18CXX)
#pragma code high_isr=0x08
#pragma interrupt high_isr
void high_isr(void)
#else
#error "Invalid compiler selection for implemented ISR routines"
#endif
{
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        // Must print output to board, using PORTC
        
        PORTBbits.RB5 = 0; // Demultiplexer
        PORTBbits.RB4 = 1; // Decoder bcd->7-segment

        ++BoardCounter;
        if (BoardCounter > 3) {
            BoardCounter = 0;
        }

        PORTC = (BoardCounter << 6) |
                (BoardNumbers[3 - BoardCounter] << 2) |
                0b00; // Unused bits

        // TMR0 register has overflowed (must be cleared by software)
        INTCONbits.TMR0IF = 0;

        // Maybe reset communication if an error happened? CommunicationCounter = 0
    }
    else if (INTCONbits.INT0F && INTCONbits.INT0E) {
        // RA0 clock raised

        // Synchronize first with host signal
        if (!CommunicationHasSynced)
        {
            TRISAbits.TRISA1 = 1;

            if (PORTAbits.RA1)
            {
                // Host sends a clock and input equals to 1, thinks we are synced
                CommunicationHasSynced = true;

                CommunicationBitsReceived[0] =
                    CommunicationBitsReceived[1] =
                    CommunicationBitsReceived[2] =
                    CommunicationBitsReceived[3] = 0;

                CommunicationCounter = 0;
            }
        }
        else
        {
            // Send 8bits, 1 ack send, read 4 x 4bit bytes, 1 ack receive = total 26 bits (2 of them ack)

            // Push 8bit changes/events requested (maybe taken from IR)
            if (CommunicationCounter < 8) {
                // Set RA1 to output
                TRISAbits.TRISA1 = 0;

                if (CommunicationCounter < 3)
                {
                    // 3-bit change number
                    uint8_t po2 = (1 << CommunicationCounter);
                    PORTAbits.RA1 = ((CommunicationActionHeader & po2) == po2) ? 1 : 0;
                }
                else if (CommunicationCounter == 3)
                {
                    // Positive or negative change
                    PORTAbits.RA1 = (CommunicationScoreChangeRequest < 0 ? 1 : 0);
                }
                else if (CommunicationCounter > 3)
                {
                    // 4-bit change number
                    uint8_t po2 = (1 << (CommunicationCounter - 4));
                    uint8_t scoreNumber = (CommunicationScoreChangeRequest > 0 ? CommunicationScoreChangeRequest : -CommunicationScoreChangeRequest);

                    PORTAbits.RA1 = ((scoreNumber & po2) == po2) ? 1 : 0;
                }
                else
                {
                    PORTAbits.RA1 = 0;
                }
            }

            if (CommunicationCounter == 8)
            {
                // Send ack
                TRISAbits.TRISA1 = 0;
                PORTAbits.RA1 = 1;
            }

            // Check if we have a signal, otherwise zero is the default
            if (CommunicationCounter > 8 && CommunicationCounter < 25) {
                // Set RA1 to input
                TRISAbits.TRISA1 = 1;

                if (PORTAbits.RA1) {
                    if (CommunicationCounter < 13) {
                        CommunicationBitsReceived[0] |= (1 << (CommunicationCounter - 9));
                    }
                    else if (CommunicationCounter < 17) {
                        CommunicationBitsReceived[1] |= (1 << (CommunicationCounter - 13));
                    }
                    else if (CommunicationCounter < 21) {
                        CommunicationBitsReceived[2] |= (1 << (CommunicationCounter - 17));
                    }
                    else if (CommunicationCounter < 25) {
                        CommunicationBitsReceived[3] |= (1 << (CommunicationCounter - 21));
                    }
                }
            }

            // Get Ack from PI
            if (CommunicationCounter >= 25) {
                TRISAbits.TRISA1 = 1;

                if (PORTAbits.RA1) {
                    BoardNumbers[0] = CommunicationBitsReceived[0];
                    BoardNumbers[1] = CommunicationBitsReceived[1];
                    BoardNumbers[2] = CommunicationBitsReceived[2];
                    BoardNumbers[3] = CommunicationBitsReceived[3];

                    CommunicationScoreChangeRequest = 0;
                    CommunicationActionHeader = COMMUNICATION_RESET;
                }

                CommunicationHasSynced = false;
            }
            else {
                ++CommunicationCounter;
            }
        }

        INTCONbits.INT0F = 0;
    } else if (INTCON3bits.INT2IF) {
        // RA2 edge-fell - IR receiver input

        if (IRCanInit) {
            T2CONbits.TMR2ON = 0;

            bool isStartDigit = (TMR2 > 150);

            if (isStartDigit) {
                InitIR();
            }

            if (IRIsOn) {
                IRBitsTimings[IRBitsTimingsSizeCounter] = TMR2;

                TMR2 = 0;
                ++IRBitsTimingsSizeCounter;

                if (IRBitsTimingsSizeCounter >= IRBitsTimingsSize) {
                    // More than have been anticipated; stop the communication
                    EndIR();
                    onIRResultTaken(IRBitsTimings, IRBitsTimingsSizeCounter);
                }
                else {
                    T2CONbits.TMR2ON = 1;
                }
            }
            else {
                InitIR();
            }
        }
        else {
            // The first edge fall mark is initialization
            IRCanInit = true;
        }
        
        INTCON3bits.INT2IF = 0;
    } else if (PIR1bits.TMR2IF) {
        // Timer 2 interrupt overflow

        // Stop the transmission check for the IR result
        EndIR();
        onIRResultTaken(IRBitsTimings, IRBitsTimingsSizeCounter);

        /*if (IRIsOn) {
            // Stop the transmission check for the IR result
            EndIR();
            onIRResultTaken(IRBitsTimings, IRBitsTimingsSizeCounter);
        } else {
            InitIR();
        }*/
        
        PIR1bits.TMR2IF = 0;
    } else if (PIR2bits.TMR3IF) {
        // Timer 3 interrupt overflow - stop the selection for up/down
        ++Timer3Counter;

        if (Timer3Counter > 4) {
            PORTBbits.RB7 = 0;
            PORTBbits.RB6 = 0;
            Timer3Counter = 0;
            T3CONbits.TMR3ON = 0;
            CommunicationActionHeader = COMMUNICATION_RESET;
        }
        
        PIR2bits.TMR3IF = 0;
    }
}

void onIRResultTaken(char* IRBitsTimings, char IRBitsTimingsSizeCounter) {
    uint32_t output = getNumberFromIRResultArray(IRBitsTimings, IRBitsTimingsSizeCounter);

    if (output == 0) {
        return;
    }

    Timer3Counter = 0;

    switch(output & 0xFFFF) {
        case BUTTON_LEFT:
        case BUTTON_A:
            PORTBbits.RB6 = 0;
            PORTBbits.RB7 = 1;
            T3CONbits.TMR3ON = 1;
            CommunicationScoreChangeRequest = 0;
            break;

        case BUTTON_RIGHT:
        case BUTTON_B:
            PORTBbits.RB6 = 1;
            PORTBbits.RB7 = 0;
            T3CONbits.TMR3ON = 1;
            CommunicationScoreChangeRequest = 0;
            break;

        case BUTTON_UP:
            if (PORTBbits.RB6) {
                CommunicationActionHeader = COMMUNICATION_BLUE;
                ++CommunicationScoreChangeRequest;
            } else if (PORTBbits.RB7) {
                CommunicationActionHeader = COMMUNICATION_RED;
                ++CommunicationScoreChangeRequest;
            }
            break;

        case BUTTON_DOWN:
            if (PORTBbits.RB6) {
                CommunicationActionHeader = COMMUNICATION_BLUE;
                --CommunicationScoreChangeRequest;
            } else if (PORTBbits.RB7) {
                CommunicationActionHeader = COMMUNICATION_RED;
                --CommunicationScoreChangeRequest;
            }
            break;

        case BUTTON_CIRCLE:
            CommunicationActionHeader = COMMUNICATION_ZERO;
            CommunicationScoreChangeRequest = 0;
            T3CONbits.TMR3ON = 0;
            PORTBbits.RB7 = 0;
            PORTBbits.RB6 = 0;
            break;
            
        case BUTTON_POWER:
            CommunicationActionHeader = COMMUNICATION_RESET;
            CommunicationScoreChangeRequest = 0;
            T3CONbits.TMR3ON = 0;
            PORTBbits.RB7 = 0;
            PORTBbits.RB6 = 0;
            break;
    }
}

void InitIR() {
    // Close communication channel
    CommunicationHasSynced = false;
    INTCONbits.INT0E = 0;
    // Port A1 should be always false now.
    TRISAbits.TRISA1 = 0;
    PORTAbits.RA1 = 0;

    // Initialize timer
    IRBitsTimingsSizeCounter = 0;
    TMR2 = 0;
    IRIsOn = true;
    T2CONbits.TMR2ON = 1;
}

void EndIR() {
    T2CONbits.TMR2ON = 0;
    TMR2 = 0;
    IRIsOn = false;
    // Reopen communication channel
    INTCONbits.INT0F = 0;
    INTCONbits.INT0E = 1;
}

/* Low-priority interrupt routine */
#if defined(__XC) || defined(HI_TECH_C)
void low_priority interrupt low_isr(void)
#elif defined (__18CXX)
#pragma code low_isr=0x18
#pragma interruptlow low_isr
void low_isr(void)
#else
#error "Invalid compiler selection for implemented ISR routines"
#endif
{
}
