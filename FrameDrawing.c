#include <p24Fxxxx.h>
#include "FrameDrawing.h"
#include <math.h>
#include "constants.h"

/*------------------------------------Macros----------------------------------*/
#define abs(x)  (x<0)?-1*x:x
/*---------------------------------Macro Functions----------------------------*/
#define stopOC()    T2CONbits.TON = 0;
#define startOC()   TMR3 = 0; T2CONbits.TON = 1;
#define wait10()    Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop()
#define wait100() wait10();wait10();wait10();wait10();wait10();wait10();wait10();wait10();wait10();wait10();
#define waitSPI()   wait10();wait10();wait10();Nop();Nop();Nop();

/*------------------------------------Variables-------------------------------*/
//dropOff = 1;
modes cur_mode = LOBED1;
int angle = 0;
int inc = 1;

/*-----------------------------------Interrupts-------------------------------*/
void __attribute__((__interrupt__,__auto_psv__)) _T3Interrupt(void)
{
    _T3IF = 0;
    //Disable GSCLK
    stopOC();
    BLANK = 1;
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

void sendGSData(){
    swapGSbytes();
    XLAT = 0;
    int i=0;
    for(i=0; i<12; i++){
        SPI1BUF = GS.data16[i];
        waitSPI();
    }
    for(i=0; i<12; i++){
        SPI1BUF = GS.data16[i];
        waitSPI();
    }
    XLAT = 1;
}

void setGS(int chan, int d){
    d = abs(d);
    d = 0x0FFF & d; //Save only the lower 12 bits
    //if(d<0x11){d=0x11;}
    int i;
    if(chan & 1){   //Odd
        i = ((chan - 1) / 2) * 3;
        i += 1;
        GS.data8[i] &= 0xF0;    //Mask MSb to save prev channel's data
        GS.data8[i] |= ((0x0F00 & d) >> 8);
        GS.data8[i+1] = (0x00FF & d);
    }else{  //Even
        i = (chan / 2) * 3;
        GS.data8[i] = ((0x0FF0 & d) >> 4);
        GS.data8[i+1] &= 0x0F;  //Mask LSb to save next channel's data
        GS.data8[i+1] |= ((0x000F & d) << 4);
    }
}

void setAll(int value){
    int i=0;
    for(i=0; i < 12; i++){
        GS.data16[i] = value;
    }
}

void lobed(int angle, int k){
    //angle = [0, 1024]
    double phi = k * PHI_conv * angle;
    int i=0;
    double intensity;
    for(i=0; i < 16; i++){
        intensity = cos(((double)i * k *PIover8) - phi);
        intensity = pow(intensity, 6);
        setGS(i, intensity * 0xFFF);
    }
}

void lobed4(int angle){
    lobed(angle, 2);
}

void lobed8(int angle){
    lobed(angle, 4);
}

void lobed3(int angle){
    lobed(angle, 3);
}

void lobed2(int angle){
    lobed(angle, 1);
}

void lobed1(int angle){
    int i;
    double phi = PHI_conv * angle;
    double intensity;
    for(i=0; i < 16; i++){
        intensity = abs(1 + cos(((double)i * PIover8) - phi));
        intensity /= 2;
        intensity = pow(intensity, 4);
        setGS(i, intensity * 0xFFF);
    }
}

void randFade(){
    
}

void pulseAll(int a){
    
}

void singleRing(int a){
    
}

void swapGSbytes(void){
    int i;
    unsigned char temp;
    for(i=0; i<24; i+=2)
    {
        temp = GS.data8[i];
        GS.data8[i] = GS.data8[i+1];
        GS.data8[i+1] = temp;
    }
}

void nextFrame(void){
    TESTpin = 1;
    if (cur_mode == LOBED1){
        lobed1(angle);
    }else if(cur_mode == LOBED2){
        lobed(angle, 1);
    }else if(cur_mode == LOBED3){
        lobed(angle, 3);
    }else if(cur_mode == LOBED4){
        lobed(angle, 2);
    }else if(cur_mode == LOBED8){
        lobed(angle, 4);
    }else if(cur_mode == RAND_FADE){
        randFade();
    }else if(cur_mode == SINGLE_RING){
        singleRing(angle);
    }else if(cur_mode == PULSE_ALL){
        pulseAll(angle);
    }else{
        //nothing
    }
//    switch (cur_mode)
//    {
//        case LOBED1:
//            lobed1(angle);
//        case LOBED2:
//            lobed(angle, 1);
//        case LOBED3:
//            lobed(angle, 3);
//        case LOBED4:
//            lobed(angle, 2);
//        case LOBED8:
//            lobed(angle, 4);
//        case RAND_FADE:
//            randFade();
//        case SINGLE_RING:
//            singleRing(angle);
//        case PULSE_ALL:
//            pulseAll(angle);
//        default:
//            cur_mode = cur_mode;
//    }
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

void setMode(modes m){
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
