#include <p24Fxxxx.h>
#include <xc.h>
#include <PPS.h>
#include <math.h>
#include "constants.h"
#include "FrameDrawing.h"

/*---------------------------------Variables----------------------------------*/
GSdata gsR;
GSdata gsL;
int dropOff = 1;

/*---------------------------------Macros-------------------------------------*/
#define newCycle    IFS0bits.T1IF

/*----------------------------------Config------------------------------------*/
#if 1
// CONFIG2
#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled)
#pragma config I2C1SEL = PRI            // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF            // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSECME           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL           // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))
#pragma config SOSCSEL = SOSC           // Sec Oscillator Select (Default Secondary Oscillator (SOSC))
#pragma config WUTSEL = LEG             // Wake-up timer Select (Legacy Wake-up Timer)
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx1               // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)
#endif

/*--------------------------------Functions-----------------------------------*/
void delayms(long int ms){
    //Relatively accurate
    ms *= 1130;
    while(ms--);
}

/*--------------------------------Interrupts----------------------------------*/
void __attribute__((__interrupt__,__auto_psv__)) _T1Interrupt(void)
{
//    _T1IF = 0;
//    TESTpin = 1;
//    startOC();
//    sendGSData();
//    TESTpin = 0;
}

void __attribute__((__interrupt__,__auto_psv__)) _SPI1Interrupt(void)
{
    IFS0bits.SPI1IF = 0;
    
}

/*------------------------------------Setup-----------------------------------*/
void setupPins(void){
    CLKDIVbits.RCDIV = 0; // make 16MHz
    AD1PCFG = 0x9fff;   //All digital
    TRISB = 0xFFFF;     //RB0-RB15 input

    //Pin directions
    TRIS_XLAT   = 0;
    TRIS_XERR   = 1;
    TRIS_DCPRG  = 0;
    TRIS_BLANK  = 0;
    TRIS_Test   = 0;
}

void setupT1(void){
    //T1-GSCLK
    T1CON = 0x0;
    //25ms  PRE=64  PR=6250     40Hz
    //12.5msPRE=64  PR=3100     80Hz
    T1CONbits.TCKPS = 0b10;   //11=256, 10=64, 01=8, 00=1
    PR1 = 2500;
    TMR1 = 0;
    _T1IF = 0;
    _T1IE = 0;
    T1CONbits.TON = 1;
}

void setupOC(void){
    //Output compare 62.5kHz
    PPSUnLock;
    PPSOutput(PPS_RP4,PPS_OC1);
    PPSLock;
    T2CON = 0;
    T2CONbits.TCKPS = 0;
    PR2 = 47;  //16us
    TMR2 = 0;
    _T2IF = 0;
    _T2IE = 0;
    OC1CON = 0;
    OC1R = 0;
    OC1RS = 20;
    _OCM = 0b101;   //Start low, continuous
    _OCTSEL = 0;
    T2CONbits.TON = 0;

    //TMR3 GSCLK Input
    PPSUnLock;
    PPSInput(PPS_T3CK, PPS_RP3);
    PPSLock;
    T3CON = 0;
    T3CONbits.TCS = 1;  //0=Internal    1=External
    T3CONbits.TCKPS = 00;   //11=256, 10=64, 01=8, 00=1
    PR3 = 4095;    //4096/256=16
    TMR3 = 0;
    _T3IF = 1;
    _T3IE = 1;
    T3CONbits.TON = 1;
}

void setupSPI(void){
    PPSUnLock;
    PPSOutput(PPS_RP7, PPS_SDO1);
    PPSOutput(PPS_RP8, PPS_SCK1OUT);
    PPSInput(PPS_SDI1, PPS_RP6);
    PPSLock;
    SPI1CON1 = 0;
    SPI1CON2 = 0;
    SPI1STAT = 0;
    SPI1STATbits.SISEL = 0b110; // IF TX buffer is empty
    _SPI1IF = 0;
    SPI1CON1bits.MSTEN = 1;  // master mode
    SPI1CON1bits.MODE16 = 1; // sixteen bits
    SPI1CON1bits.CKE = 1;
    SPI1CON1bits.CKP = 0;
    SPI1CON1bits.SPRE = 0b110; //111=1:1, 110=1:2, 000=1:8
    SPI1CON1bits.PPRE = 0b11;  //11=1:1, 10=4:1, 01=16:1, 00=64:1
    _SPI1IE = 0;
}

/*---------------------------------Main---------------------------------------*/
void setup(){
    setupPins();
    setupT1();
    setupOC();
    setupSPI();
}

int main(void)
{
    setup();
    T2CONbits.TON = 1;
    T1CONbits.TON = 1;
    SPI1STATbits.SPIEN = 1;

    //Set all to default levels
    //GSCLK   = 0;
    SCLK    = 0;
    GSCount = 0;
    XLAT    = 0;
    BLANK   = 1;

    delayms(55);
    
    //allOff();

    setMode(LOBED1);
    setBPM(120);
    
    init();
    int speedChange = 0;

    while(1){
        while(!newCycle);
        newCycle = 0;
        nextFrame();
        if(!speedChange){setBPM(60);}
        else if(speedChange > 768){if(speedChange & 1){incBPM(2);}}
        else if(speedChange > 512){if(speedChange & 1){incBPM(-2);}}
        speedChange++;
        speedChange = speedChange & 0x3FF;//1023
    }
    return 0;
}