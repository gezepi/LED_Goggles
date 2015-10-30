#ifndef CONSTANTS_H
#define	CONSTANTS_H

/*---------------------------------Pins---------------------------------------*/
#define XLAT    LATBbits.LATB9
#define BLANK   LATBbits.LATB10
#define XERR    LATBbits.LATB5
#define DCPRG   LATAbits.LATA4
#define GSCLK   LATBbits.LATB4
#define GSCount LATBbits.LATB3
//  SPI
#define SCLK    LATBbits.LATB8
#define MISO    LATBbits.LATB6
#define MOSI    LATBbits.LATB7
//  Other
#define TESTpin LATBbits.LATB11

/*--------------------------------TRIS----------------------------------------*/
#define TRIS_XERR   TRISBbits.TRISB5
#define TRIS_DCPRG  TRISAbits.TRISA4
#define TRIS_BLANK  TRISBbits.TRISB10
#define TRIS_XLAT   TRISBbits.TRISB9
#define TRIS_Test   TRISBbits.TRISB11


#endif	/* CONSTANTS_H */

