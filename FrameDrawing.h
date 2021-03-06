#ifndef FRAMEDRAWING_H
#define	FRAMEDRAWING_H

/*----------------------------------Includes----------------------------------*/

/*--------------------------------Macros--------------------------------------*/
#define allOn()     setAll(1)
#define allOff()    setAll(0)
#define TWO_PI      6.283185
#define PI          3.141592
#define PIover2     1.570796
#define PIover4     0.785398
#define PIover8     0.392699
#define maxFrames   2047
#define PHI_conv    (TWO_PI / (maxFrames+1))   //changes [0,1024]->[0,2PI]
#define FPS         70  //Figure out the PRx for this

/*--------------------------------Variables-----------------------------------*/
typedef enum {
    LOBED1,
    LOBED2,
    LOBED3,
    LOBED4,
    LOBED8,
    RAND_FADE,
    SINGLE_RING,
    PULSE_ALL,
    ALL_SAME,
} modes;

typedef union{
    unsigned int data16[12];
    unsigned char data8[24];
} GSdata;

extern GSdata gsL;
extern GSdata gsR;
extern int dropOff;    //How quickly the values drop from full strength
                //  raises a value that's less than one to a power (.5^2=.25)

/*-------------------------------Functions------------------------------------*/
void swapGSbytes(void);
void setBPM(int bpm);
void incBPM(int bpm);
void setMaxI(int i);
void incMaxI(int i);
void setMode(modes m);
void init(void);
void nextFrame(void);

#endif
