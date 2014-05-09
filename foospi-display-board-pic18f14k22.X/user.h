/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

#define BUTTON_POWER 0xD827 // i.e. 0x10EFD827
#define BUTTON_A 0xF807
#define BUTTON_B 0x7887
#define BUTTON_C 0x58A7
#define BUTTON_UP 0xA05F
#define BUTTON_DOWN 0x00FF
#define BUTTON_LEFT 0x10EF
#define BUTTON_RIGHT 0x807F
#define BUTTON_CIRCLE 0x20DF

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

void InitApp(void);         /* I/O and Peripheral Initialization */
void printToBoard(char* BoardNumbers, uint16_t number);
uint32_t getNumberFromIRResultArray(char* IRBitsTimings, char IRBitsTimingsCounter);
void IRIncrementCounter();
void onIRResultTaken(char* IRBitsTimings, char IRBitsTimingsCounter);
void InitIR();
void EndIR();
