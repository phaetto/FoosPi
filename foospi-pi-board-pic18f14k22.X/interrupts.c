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

#endif

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

#define NO_CHANNEL 0x00
#define MAX_WAIT 0xFFFFF

uint8_t CommunicationCounter = 0;
uint8_t CommunicationChannelTriggered = NO_CHANNEL; // RA0 or RA1 has been triggered?
uint32_t CommunicationSpeedClock;
bool CommunicationHasSynced = false;

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
    if (INTCONbits.INT0F) {
        // Channel RA0 has been activated

        if (CommunicationChannelTriggered == NO_CHANNEL && !T0CONbits.TMR0ON)
        {
            // Close interrupts and communication while at it
            INTCONbits.GIE = 0;
            TRISCbits.TRISC0 = 0;
            PORTCbits.RC0 = 0;
            CommunicationHasSynced = false;

            uint32_t timer = 0x00000000;
            while(timer < MAX_WAIT && PORTAbits.RA0 == 1)
            {
                ++timer;
            }

            CommunicationChannelTriggered = 0x01;
            CommunicationSpeedClock = timer;
            TMR0 = 0x0000;
            T0CONbits.TMR0ON = 1;

            INTCONbits.GIE = 1;
        }

        INTCONbits.INT0F = 0;
    }
    else if (INTCON3bits.INT1F) {
        // Channel RA1 has been activated

        if (CommunicationChannelTriggered == NO_CHANNEL && !T0CONbits.TMR0ON)
        {
            // Close interrupts and communication while at it
            INTCONbits.GIE = 0;
            TRISCbits.TRISC0 = 0;
            PORTCbits.RC0 = 0;
            CommunicationHasSynced = false;

            uint32_t timer = 0x00000000;
            while(timer < MAX_WAIT && PORTAbits.RA1 == 1)
            {
                ++timer;
            }

            CommunicationChannelTriggered = 0x02;
            CommunicationSpeedClock = timer;
            TMR0 = 0x0000;
            T0CONbits.TMR0ON = 1;

            INTCONbits.GIE = 1;
        }
        
        INTCON3bits.INT1F = 0;
    }
    else if (INTCONbits.TMR0IF) {
        // When timer goes off, then the input can be enabled again
        T0CONbits.TMR0ON = 0;
        INTCONbits.TMR0IF = 0;
    }
    else if (INTCON3bits.INT2F) {
        // RA2 clock raised

        // Synchronize first with host signal
        if (!CommunicationHasSynced)
        {
            TRISCbits.TRISC0 = 1;

            if (PORTCbits.RC0)
            {
                // Host sends a clock and input equals to 1, thinks we are synced
                CommunicationHasSynced = true;

                CommunicationCounter = 0;
            }
        }
        else
        {
            // Send 8bits (Channel enabled), 1 ack send, 3x8bits, 1 ack receive = total 34 bits (2 of them ack)
            TRISCbits.TRISC0 = 0;

            if (CommunicationCounter < 8) {
                // Set RA1 to output
                uint8_t po2 = (1 << CommunicationCounter);
                PORTCbits.RC0 = ((CommunicationChannelTriggered & po2) == po2) ? 1 : 0;
            }

            if (CommunicationCounter == 8){
                // Send ack to pi
                if (CommunicationChannelTriggered != NO_CHANNEL)
                {
                    PORTCbits.RC0 = 1;
                }
                else
                {
                    PORTCbits.RC0 = 0;
                    CommunicationHasSynced = false;
                }
            }

            if (CommunicationCounter > 8 && CommunicationCounter < 33) {
                uint32_t po2 = 1;
                po2 = (po2 << (CommunicationCounter - 9));
                PORTCbits.RC0 = ((CommunicationSpeedClock & po2) == po2) ? 1 : 0;
            }

            // Get Ack from PI
            if (CommunicationCounter >= 33) {
                TRISCbits.TRISC0 = 1;

                if (PORTCbits.RC0) {
                    // Ack
                    CommunicationChannelTriggered = NO_CHANNEL;
                }

                CommunicationHasSynced = false;
            }
            else {
                ++CommunicationCounter;
            }
        }

        INTCON3bits.INT2F = 0;
    }
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
