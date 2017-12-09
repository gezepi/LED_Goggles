#include <p24Fxxxx.h>
#include "FrameDrawing.h"
#include <math.h>
#include "constants.h"
#include <stdlib.h>

/*------------------------------------Macros----------------------------------*/
#define abs(x)  (x<0)?-1*x:x
#define constrain(x, l, h)  ((x)<(l))?(l):(((x)>(h))?(h):x)
#define sign(x) ((x)<0)?-1:1
/*---------------------------------Macro Functions----------------------------*/
#define stopOC()    T2CONbits.TON = 0;
#define startOC()   TMR3 = 0; T2CONbits.TON = 1;
#define wait10()    Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop()
#define wait100() wait10();wait10();wait10();wait10();wait10();wait10();wait10();wait10();wait10();wait10();
#define waitSPI()   wait10();wait10();wait10();Nop();Nop();Nop();

/*------------------------------------Variables-------------------------------*/
int cur_mode = LOBED1;
int angle = 1024;
int inc = 1;
int maxIntensity = 0xFFF;

/*-----------------------------------Interrupts-------------------------------*/
void __attribute__((__interrupt__,__auto_psv__)) _T3Interrupt(void)
{
    //4096 pulses on GSCLK have occurred
    _T3IF = 0;
    //Disable GSCLK
    stopOC();
    BLANK = 1;
}

/*-----------------------------------GS Functions-----------------------------*/
void setGS(GSdata* gs, int chan, int d){
    d = abs(d);
    d = 0x0FFF & d; //Save only the lower 12 bits
    chan = chan & 0xF;
    int i;
    if(chan & 1){   //Odd
        i = ((chan - 1) / 2) * 3;
        i += 1;
        gs->data8[i] &= 0xF0;    //Mask MSb to save prev channel's data
        gs->data8[i] |= ((0x0F00 & d) >> 8);
        gs->data8[i+1] = (0x00FF & d);
    }else{  //Even
        i = (chan / 2) * 3;
        gs->data8[i] = ((0x0FF0 & d) >> 4);
        gs->data8[i+1] &= 0x0F;  //Mask LSb to save next channel's data
        gs->data8[i+1] |= ((0x000F & d) << 4);
    }
}

void swapGSbytes(void){
    int i;
    unsigned char temp;
    for(i=0; i<24; i+=2)
    {
        //Left
        temp = gsL.data8[i];
        gsL.data8[i] = gsL.data8[i+1];
        gsL.data8[i+1] = temp;
        //Right
        temp = gsR.data8[i];
        gsR.data8[i] = gsR.data8[i+1];
        gsR.data8[i+1] = temp;
    }
}

void sendGSData(){
    int i=0;
    for(i=0; i<16; i++){
        //Transfer data from normal arrays to GS arrays
        setGS(&gsR, i, eyeR[i]);
        setGS(&gsL, i, eyeL[i]);
    }
    swapGSbytes();
    XLAT = 0;
    for(i=0; i<12; i++){
        SPI1BUF = gsL.data16[i];
        waitSPI();
    }
    for(i=0; i<12; i++){
        SPI1BUF = gsR.data16[i];
        waitSPI();
    }
    XLAT = 1;
}

void setLED(unsigned int* eye, int l, int val){
    eye[l] = val;
}

void incLED(unsigned int* eye, int l, int inc){
    eye[l] = eye[l] + inc;
}

/*--------------------------------Drawing Functions---------------------------*/
void setAll(int value){
    int i=0;
    for(i=0; i < 16; i++){
        setLED(eyeR, i, value);
        setLED(eyeL, 15-i, value);
    }
}

void incAll(int inc){
    int i=0;
    for(i=0; i < 16; i++){
        incLED(eyeR, i, inc);
        incLED(eyeL, i, inc);
    }
}

void lobed(int angle, double k){
    //angle = [0, 1024]
    double phi = PHI_conv * angle;
    int i;
    double intensity;
    for(i=0; i < 16; i++){
        intensity = abs(cos(k * (((double)i * PIover8) - phi)));
        intensity = pow(intensity, 7 - (int)k);
        setLED(eyeR, i, intensity * maxIntensity);
        setLED(eyeL, 15-i, intensity * maxIntensity);
    }
}

void lobed1(int angle){
    int i;
    double phi = PHI_conv * angle;
    double intensity;
    for(i=0; i < 16; i++){
        intensity = abs(1 + cos(((double)i * PIover8) - phi));
        intensity /= 2;
        intensity = pow(intensity, 4);
        setLED(eyeR, i, intensity * maxIntensity);
        setLED(eyeL, 15-i, intensity * maxIntensity);
    }
}

void randFade(){
    incAll(-1);
    int led = rand() % 0xF;
    led = constrain(led, 0, 15);
    setLED(eyeR, (led), maxIntensity);
    led = rand() % 0xF;
    setLED(eyeL, (led), maxIntensity);
}

void pulseAll(int a){
    
}

void singleRing(int a){
    int led = 15 * (a / maxFrames);
    //setAll(0);
    setLED(eyeR, led, maxIntensity);
    setLED(eyeL, 15-led, maxIntensity);
}

/*--------------------------------Private Functions---------------------------*/
int SPI_Blocking(int d){
    XLAT = 0;
    SPI1BUF = d;
    //while(IFS0bits.SPI1IF == 0);
    waitSPI();
    XLAT = 1;
    return SPI1BUF;
}

void nextFrame(void){
    TESTpin = 1;
    if(cur_mode == ALL_ON){
        setAll(maxIntensity);
    }else if(cur_mode == LOBED1){
        lobed1(angle);
    }else if(cur_mode == LOBED2){
        lobed(angle, 1);
    }else if(cur_mode == LOBED3){
        lobed(angle, 1.5);
    }else if(cur_mode == LOBED4){
        lobed(angle, 2);
    }else if(cur_mode == LOBED6){
        lobed(angle, 3);
    }else if(cur_mode == RAND_FADE){
        randFade();
    }else if(cur_mode == FADE){
        incAll(-1);
    }else if(cur_mode == SINGLE_RING){
        singleRing(angle);
    }else if(cur_mode == PULSE_ALL){
        pulseAll(angle);
    }else if(cur_mode == ALL_SAME){
        setAll((angle/maxFrames) * maxIntensity);
    }else if(cur_mode == ALL_OFF){
        setAll(0);
    }else{
        lobed1(angle);
    }
    sendGSData();   //Send frame
    BLANK = 0;
    startOC();      //Start GSCLK
    angle += inc;
    if(angle > maxFrames){angle = 0;}
    else if(angle < 0){angle = maxFrames;}
    TESTpin = 0;
}

/*--------------------------------Shared Functions----------------------------*/
void setBPM(int bpm){
    int bps = bpm * 60;
    //set the timer to something that makes sense
    //inc = (maxFrames + 1) / (FPS * bps);
    inc = bps / FPS;
}

void incBPM(int bpm){
    int bps = bpm * 60;
    //set the timer to something that makes sense
    //inc = (maxFrames + 1) / (FPS * bps);
    inc += bps / FPS;
}

void setMaxI(int i){
    i = constrain(i, 0, 0xFFF);
    maxIntensity = i;
}

void incMaxI(int i){
    maxIntensity += i;
    maxIntensity = constrain(maxIntensity, 0, 0xFFF);
}

void setMode(int m){
    cur_mode = m;
}

void init(void){
    //Send first frame
    allOff();
    sendGSData();
    GSCLK ^= 1;
    wait10();
    GSCLK ^= 1;
}
