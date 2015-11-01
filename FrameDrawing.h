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
#define LOBED1		1
#define LOBED2		2
#define LOBED3		3
#define LOBED4		4
#define LOBED6		5
#define RAND_FADE	6
#define SINGLE_RING	7
#define PULSE_ALL	8
#define ALL_SAME	9
#define FADE		10
#define ALL_ON		11
#define ALL_OFF		12


typedef union{
    unsigned int data16[12];
    unsigned char data8[24];
} GSdata;

extern GSdata gsL;
extern GSdata gsR;
extern unsigned int eyeL[16];
extern unsigned int eyeR[16];
extern int dropOff;    //How quickly the values drop from full strength
                //  raises a value that's less than one to a power (.5^2=.25)

/*-------------------------------Functions------------------------------------*/
void setBPM(int bpm);
void incBPM(int bpm);
void setMaxI(int i);
void incMaxI(int i);
void setMode(int m);
void init(void);
void nextFrame(void);

#endif
