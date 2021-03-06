
#include <Wire.h>
#include "wiring_private.h"
#include "Adafruit_ZeroTimer.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"

#include "samd51_adc.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)
#define RANGE(min,v,max)	  MIN(MAX(v,min),max) 
#define MAP_RANGE(v,a,b,mi,ma) RANGE(mi,map(v,a,b,mi,ma),ma)

  
// const frequency = Math.pow(2, (m - 69) / 12) * 440;
// incrementer unipolar = 1/SR * f
// incrementer bipolar = 2/SR * f 


#include "s0.h"
#include "s1.h"
#include "s2.h"
#include "s3.h"
#include "s4.h"
#include "s5.h"
#include "s6.h"
#include "s7.h"
#include "s8.h"
#include "s9.h"
#include "s10.h"
#include "s11.h"
#include "s12.h"
#include "s13.h"
#include "s14.h"
#include "s15.h"

#include "s16.h"
#include "s17.h"
#include "s18.h"
#include "s19.h"
#include "s20.h"
#include "s21.h"
#include "s22.h"
#include "s23.h"
#include "s24.h"
#include "s25.h"
#include "s26.h"
#include "s27.h"
#include "s28.h"
#include "s29.h"
#include "s30.h"
#include "s31.h"
/*

PA22 RandomOut    PWM Out ?
PA02 SQROut       DAC0
PA05 SampleOut    DAC1

PA19 SQ1 OUT  PA18 FilterCap1
PA12 SQ2 OUT  PA17 FilterCap2
PB15 SQ3 OUT  PA16 FilterCap3
PB14 SQ4 OUT  PA15 FilterCap4
PB13 SQ5 OUT  PA14 FilterCap5
PB12 SQ6 OUT  PA13 FilterCap6

PB08 SQR Tune ADC
PB09 SQR Spread ADC
PB07 Ratchet ADC
PB03 ClockSpeed ADC
PB06 Sample TUNE ADC
PB05 Sample Change ADC 

PA20 Taster A/B 
PA21 Taster LPF 
PB17 Taster BIT 
PB16 Taster Trigger

PB01 TriggerInput SamplePlay
PB02 TriggerInput Clk IN
PB04 Clk Input gesteckt 

PB00 LED CLK 
PB31 LED SampleChange 

PA23 LED SMD Inline (BootLoader)

*/



 // need to Test .... TODO: call this from setup function and uncomment return in Line 164
 void fixBOD33(){
    // A magic number in the unused area of the user page indicates that
    // the device is already updated with the current configuration.
    static const uint32_t magic = 0xa5f12945;
    const uint32_t *page        = (uint32_t *)NVMCTRL_USER;
    if (page[8] == magic)
      return;

    union {
      uint32_t data[128];
      uint8_t bytes[512];
    };

    // void V2Memory::Flash::UserPage::read(uint32_t data[128]) {
    memcpy(data, (const void *)NVMCTRL_USER, 512);

    Serial.println("current FUSES");
    Serial.println( data[0],HEX);
    Serial.println( data[1],HEX);
    Serial.println( data[2],HEX);
    Serial.println( data[3],HEX);
    Serial.println( data[4],HEX);
    Serial.println( data[5],HEX);
    Serial.println( data[6],HEX);
    Serial.println( data[7],HEX);
    Serial.println( data[8],HEX);



    // Ignore all current values; fix the fallout caused by the broken uf2
    // bootloader, which has erased the devices's factory calibration. Try to
    // restore it with known values
    //
    // User Page Dump (Intel Hex) of pristine SAMD51G19A:
    // :0200000400807A
    // :1040000039929AFE80FFECAEFFFFFFFFFFFFFFFF3C
    // :1040100010408000FFFFFFFFFFFFFFFFFFFFFFFFDC
    if (data[4] == 0xffffffff) {
      memset(bytes, 0xff, 512);
      data[0] = 0xfe9a9239; // 0xF69A9239
      data[1] = 0xaeecff80;
      data[4] = 0x00804010;
    }


    // Protect the bootloader area.
    data[0] = (data[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | NVMCTRL_FUSES_BOOTPROT(13);

    // Enable the Brown-Out Detector
    data[0] &= ~FUSES_BOD33_DIS_Msk;

    // Set EEPROM size (4 Kb)
    data[1] = (data[1] & ~NVMCTRL_FUSES_SEESBLK_Msk) | NVMCTRL_FUSES_SEESBLK(1);
    data[1] = (data[1] & ~NVMCTRL_FUSES_SEEPSZ_Msk) | NVMCTRL_FUSES_SEEPSZ(3);

    // Add our magic, it will skip this configuration at startup.
    data[8] = magic;

    Serial.println("New FUSES");
    Serial.println( data[0],HEX);
    Serial.println( data[1],HEX);
    Serial.println( data[2],HEX);
    Serial.println( data[3],HEX);
    Serial.println( data[4],HEX);
    Serial.println( data[5],HEX);
    Serial.println( data[6],HEX);
    Serial.println( data[7],HEX);
    Serial.println( data[8],HEX);
   
    
    // uncomment to Write Fuses
    // return;


    // V2Memory::Flash::UserPage::write(data);
    while (NVMCTRL->STATUS.bit.READY == 0)
      ;

    // Manual write
    NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

    // Erase page
    NVMCTRL->ADDR.reg  = (uint32_t)NVMCTRL_USER;
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EP;
    while (NVMCTRL->STATUS.bit.READY == 0)
      ;

    // Page buffer clear
    NVMCTRL->ADDR.reg  = (uint32_t)NVMCTRL_USER;
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;
    while (NVMCTRL->STATUS.bit.READY == 0)
      ;

    // Write page
    uint32_t *addr = (uint32_t *)NVMCTRL_USER;
    for (uint8_t i = 0; i < 128; i += 4) {
      addr[i + 0] = data[i + 0];
      addr[i + 1] = data[i + 1];
      addr[i + 2] = data[i + 2];
      addr[i + 3] = data[i + 3];

      // Write quad word (128 bits)
      NVMCTRL->ADDR.reg  = (uint32_t)(addr + i);
      NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WQW;
      while (NVMCTRL->STATUS.bit.READY == 0)
        ;
    }
    // Reboot to enable the new settings.
    delay(100);
    NVIC_SystemReset();

  }


Adafruit_ZeroTimer zt4 = Adafruit_ZeroTimer(4);

void TC4_Handler(){
  Adafruit_ZeroTimer::timerHandler(4);
}

/*
Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS, PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
Adafruit_SPIFlash flash(&flashTransport);

void dump_sector(uint32_t sector) {
  uint8_t buf[512];
  flash.readBuffer(sector*512, buf, 512);
  for(uint32_t row=0; row<32; row++)  {
    if ( row == 0 ) Serial.print("0");
    if ( row < 16 ) Serial.print("0");
    Serial.print(row*16, HEX);
    Serial.print(" : ");
    for(uint32_t col=0; col<16; col++)    {
      uint8_t val = buf[row*16 + col];
      if ( val < 16 ) Serial.print("0");
      Serial.print(val, HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}
*/




SAMD51_ADC adc51;
// SAMPLE RATE = 48MHz TC / 4 TC_CLOCK_PRESCALER_DIV4 / setPeriodMatch=125  = 96Khz 
const float samplerate = 48000000.0 / 4.0 / 125.0;
const float reciprocal_sr = 1.0 / samplerate;




float pitches[1024];
uint8_t sines[256];

void setup() {
  //put your setup code here, to run once:

  fixBOD33();
  // First try remove DC from Ouputs  
  dacInit();
  DAC->DATA[0].reg = 2048;   // ca 1.5V
  DAC->DATA[1].reg = 2048;
  while (DAC->SYNCBUSY.bit.DATA0);

  // gen sin Table
  for(uint16_t i = 0 ; i < 256 ; i ++){
    float phase = (float)i / 256.0f * PI * 2.0f;
    sines[i] = (sin(phase) + 1.0) * 127.0f; 
  }

  // gen Table
  for(uint16_t i = 0 ; i < 1024 ; i ++){
    float clk_tempo_f = (float)i / 256.0f - 6.0f;
    pitches[i] = pow(12,clk_tempo_f);
  }

  Serial.begin(115200);
  // while ( !Serial ) delay(10);   // wait for native usb
  Serial.println("Himalia");

  /*
  flash.begin();
  Serial.println("Adafruit Serial Flash Info example");
  Serial.print("JEDEC ID: "); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.println(flash.size());
  */

  pinMode(PA13,INPUT); digitalWrite(PA13,false);
  pinMode(PA14,INPUT); digitalWrite(PA14,false);
  pinMode(PA15,INPUT); digitalWrite(PA15,false);
  pinMode(PA16,INPUT); digitalWrite(PA16,false);
  pinMode(PA17,INPUT); digitalWrite(PA17,false);
  pinMode(PA18,INPUT); digitalWrite(PA18,false);


  adc51.createADCMap();

  pinMode(PA23,OUTPUT); // LED
  pinMode(PB00,OUTPUT); // LED
  pinMode(PB31,OUTPUT); // LED

  pinMode(PA19,OUTPUT); // SQR1
  pinMode(PA12,OUTPUT); // SQR2
  pinMode(PB15,OUTPUT); // SQR1
  pinMode(PB14,OUTPUT); // SQR1
  pinMode(PB13,OUTPUT); // SQR1
  pinMode(PB12,OUTPUT); // SQR1

  pinMode(PA22,OUTPUT); // PWM OUT

  pinMode(PB01,INPUT_PULLUP); // SQR1
  pinMode(PA21,INPUT_PULLUP); // SQR1
  pinMode(PB17,INPUT_PULLUP); // 8Bit or sample Button
  pinMode(PB16,INPUT_PULLUP); // Manual Trigger
  pinMode(PB23,INPUT_PULLUP); // Manual Trigger
  

  // create Time for AudioSamples
  zt4.configure(TC_CLOCK_PRESCALER_DIV8, // prescaler
                TC_COUNTER_SIZE_8BIT,   // bit width of timer/counter
                TC_WAVE_GENERATION_MATCH_PWM  // match style
                );

  zt4.setPeriodMatch(125, 250, 0); // 1 match, channel 0
  zt4.setCallback(true, TC_CALLBACK_CC_CHANNEL0, renderAudio);  // set DAC in the callback
  zt4.enable(true);
}

//
//  NoiseGenerator
//
class LFSR {
  private:
  uint16_t reg;
  public:
  LFSR(uint16_t seed) : reg(seed) {}
  uint16_t next() {
    uint8_t b = ((reg >> 0) ^ (reg >> 1) ^ (reg >> 3) ^ (reg >> 12)) & 1;
    reg = (reg >> 1) | (b << 15);
    return reg;
  }
};
LFSR lsfr1(0xA1e);


// 8-Bit OSC Stuff
#define OSC8BIT_PRG_COUNT 32
typedef uint16_t (*zm8BitPrg) (uint16_t t);
// https://git.metanohi.name/bytebeat.git/raw/c2f559b6efac4b03a0233e5797437af30601c170/clive.c
static uint16_t render_prg0(uint16_t t){ return  t; }
static uint16_t render_prg1(uint16_t t){ return  sines[(t>>8)]; }
static uint16_t render_prg2(uint16_t t){ return  t*(42&t>>10); }     // viznut
static uint16_t render_prg3(uint16_t t){ return  (t*5&t>>7)|(t*3&t>>10); }

static uint16_t render_prg4(uint16_t t){ return  t|t%255|t%257; } 
static uint16_t render_prg5(uint16_t t){ return  t>>6&1?t>>5:-t>>4; }
static uint16_t render_prg6(uint16_t t){ return  t*(t>>9|t>>13)&16; }
static uint16_t render_prg7(uint16_t t){ return  t*(((t>>9)^((t>>9)-1)^1)%13); }

static uint16_t render_prg8(uint16_t t){  return  t*(t>>8*((t>>15)|(t>>8))&(20|(t>>19)*5>>t|(t>>3))); }
static uint16_t render_prg9(uint16_t t){  return  t*(t>>((t>>9)|(t>>8))&(63&(t>>4))); }
static uint16_t render_prg10(uint16_t t){ return (t>>6|t|t>>(t>>16))*10+((t>>11)&7); }
static uint16_t render_prg11(uint16_t t){ return (t|(t>>9|t>>7))*t&(t>>11|t>>9); }

static uint16_t render_prg12(uint16_t t){ return t*(((t>>12)|(t>>8))&(63&(t>>4))); }
static uint16_t render_prg13(uint16_t t){ return t*(t^t+(t>>15|1)^(t-1280^t)>>10); }
static uint16_t render_prg14(uint16_t t){ return (t&t%255)-(t*3&t>>13&t>>6); }
static uint16_t render_prg15(uint16_t t){ return (t+(t>>2)|(t>>5))+(t>>3)|((t>>13)|(t>>7)|(t>>11)); }

const zm8BitPrg prgList[OSC8BIT_PRG_COUNT] = {  render_prg0, render_prg1, render_prg2, render_prg3,
                                                render_prg4, render_prg5, render_prg6, render_prg7,
                                                render_prg8, render_prg9, render_prg10,render_prg11,
                                                render_prg12,render_prg13,render_prg14,render_prg15,

                                                render_prg0, render_prg1, render_prg2, render_prg3,
                                                render_prg4, render_prg5, render_prg6, render_prg7,
                                                render_prg8, render_prg9, render_prg10,render_prg11,
                                                render_prg12,render_prg13,render_prg14,render_prg15
                                                };




//
//  AUDIO RENDERER Parms
//
volatile float thea[6]     = { 0.0  , 0.0    , 0.0    , 0.0    , 0.0    , 0.0   };
volatile float thea_inc[6] = { 0.001, 0.00101, 0.00102, 0.00105, 0.00107, 0.00109};
volatile float sq_TRS[6]   = { 0.0  ,0.0     ,0.0     ,0.0     ,0.0     ,0.0};
volatile float flt_TRS[6]   = { 0.0  ,0.0     ,0.0     ,0.0     ,0.0     ,0.0};
volatile float spreads[6]  = { 0.0001f, 0.0001002f, 0.0001003f,0.0001007f,0.0001011f,0.0001017f};

float thea_noise  = 0.0f;
volatile float inc_noise   = 0.001f;
volatile float inc_8bit   = 0.001f;

float thea_sample  = 0.0f;
volatile float inc_sample   = 0.001f;

volatile int prg8=0;
volatile uint16_t samplePrg;
// volatile bool is_8bitchipmode = false;

volatile uint32_t ratchet_counts=1;
//
//  AUDIO RENDERER callback for a single sample
//
void renderAudio() {
  //  PORT->Group[PORTA].OUTSET.reg = 1ul << 22;

  // Noise / S/H Output
  thea_noise+=inc_noise;
  if(thea_noise>1.0f){
    thea_noise-=2.0f;
    // if(!DAC->SYNCBUSY.bit.DATA0)    
    DAC->DATA[0].reg = lsfr1.next();

    static uint16_t subdiv=0;
    subdiv++;

    if(!(subdiv % 4)){
      static bool noise_led=false;
      noise_led=!noise_led;
      if(noise_led)  PORT->Group[PORTB].OUTCLR.reg = 1ul << 0;  else   PORT->Group[PORTB].OUTSET.reg = 1ul << 0;    // LED
      if(noise_led)  PORT->Group[PORTA].OUTCLR.reg = 1ul << 22; else   PORT->Group[PORTA].OUTSET.reg = 1ul << 22;   // SQUARE OUT NOISE
    }
  }




  // SuperSQUARE Ramps generate binary pattern
  bool sqr_pins[6];
  bool flt_pins[6];
  for(int i = 0 ; i < 6 ; i++){
    thea[i]+=thea_inc[i];
    if(thea[i]>1.0f) thea[i]-=2.0f;
    sqr_pins[i] = (thea[i]>sq_TRS[i]); 
    flt_pins[i] = (thea[i]>flt_TRS[i]); 
  }
  // Pin assign signals as fast as possible for generated binary pattern
  if(sqr_pins[0])  PORT->Group[PORTA].OUTCLR.reg = 1ul << 19; else   PORT->Group[PORTA].OUTSET.reg = 1ul << 19;
  if(sqr_pins[1])  PORT->Group[PORTA].OUTCLR.reg = 1ul << 12; else   PORT->Group[PORTA].OUTSET.reg = 1ul << 12;
  if(sqr_pins[2])  PORT->Group[PORTB].OUTCLR.reg = 1ul << 15; else   PORT->Group[PORTB].OUTSET.reg = 1ul << 15;
  if(sqr_pins[3])  PORT->Group[PORTB].OUTCLR.reg = 1ul << 14; else   PORT->Group[PORTB].OUTSET.reg = 1ul << 14;
  if(sqr_pins[4])  PORT->Group[PORTB].OUTCLR.reg = 1ul << 13; else   PORT->Group[PORTB].OUTSET.reg = 1ul << 13;
  if(sqr_pins[5])  PORT->Group[PORTB].OUTCLR.reg = 1ul << 12; else   PORT->Group[PORTB].OUTSET.reg = 1ul << 12;

  if(flt_pins[0])  PORT->Group[PORTA].DIRSET.reg = 1ul << 13; else   PORT->Group[PORTA].DIRCLR.reg = 1ul << 13;
  if(flt_pins[1])  PORT->Group[PORTA].DIRSET.reg = 1ul << 14; else   PORT->Group[PORTA].DIRCLR.reg = 1ul << 14;
  if(flt_pins[2])  PORT->Group[PORTA].DIRSET.reg = 1ul << 15; else   PORT->Group[PORTA].DIRCLR.reg = 1ul << 15;
  if(flt_pins[3])  PORT->Group[PORTA].DIRSET.reg = 1ul << 16; else   PORT->Group[PORTA].DIRCLR.reg = 1ul << 16;
  if(flt_pins[4])  PORT->Group[PORTA].DIRSET.reg = 1ul << 17; else   PORT->Group[PORTA].DIRCLR.reg = 1ul << 17;
  if(flt_pins[5])  PORT->Group[PORTA].DIRSET.reg = 1ul << 18; else   PORT->Group[PORTA].DIRCLR.reg = 1ul << 18;



  // Forward Button + Trigger to LED
  static uint32_t leftSamples = 0;
  static bool     previous_trigger = false;
  if (  (PORT->Group[PORTB].IN.reg & (1ul << 16))  &&      // Trigger from Button
        (PORT->Group[PORTB].IN.reg & (1ul << 23))     ) {  // Trigger from jack
    PORT->Group[PORTB].OUTCLR.reg = 1ul << 31;             // trigger fire!
    previous_trigger=false;
  } else {
    PORT->Group[PORTB].OUTSET.reg = 1ul << 31;
    if(!previous_trigger){          // check for edge
      leftSamples = ratchet_counts;
      thea_sample=0.0f; // retrigger 
    }
    previous_trigger=true;
  }



  bool is_8bitchipmode = PORT->Group[PORTB].IN.reg & (1ul << 17);        // digitalRead(PB17);

  if(!is_8bitchipmode){
    // 8 Bit OSC
    static float t=0;
    zm8BitPrg  callBackPrg = prgList[prg8];
    if(t>65535.0f) t=0.0f;  {
      t+=inc_8bit * 1024.0f;  // TODO: its wrong!!! needs callBack on phase Reset!!!!
      // if(!DAC->SYNCBUSY.bit.DATA1)
      DAC->DATA[1].reg = callBackPrg((uint16_t)t) << 4;
    }
  }else{
    // Sample Ouput
    // sox /PRJ/test1.aif  --bits 16 --encoding unsigned-integer --endian little -c 1 t1.raw
    // xxd -i t1.raw > /PRJ/IOCore/HimaliaSketch/t1.h 
    // sed -i -r 's/unsigned/const unsigned/g' /PRJ/IOCore/HimaliaSketch/t1.h 
    const float sample_mul[32] = {  (float)s0_raw_len / 2.0f -1  , (float)s1_raw_len / 2.0f -1  , (float)s2_raw_len / 2.0f -1  , (float)s3_raw_len / 2.0f -1 ,
                                    (float)s4_raw_len / 2.0f -1  , (float)s5_raw_len / 2.0f -1  , (float)s6_raw_len / 2.0f -1  , (float)s7_raw_len / 2.0f -1 ,
                                    (float)s8_raw_len / 2.0f -1  , (float)s9_raw_len / 2.0f -1  , (float)s10_raw_len / 2.0f -1 , (float)s11_raw_len / 2.0f -1 ,
                                    (float)s12_raw_len / 2.0f -1 , (float)s13_raw_len / 2.0f -1 , (float)s14_raw_len / 2.0f -1 , (float)s15_raw_len / 2.0f -1 ,
                                    
                                    (float)s16_raw_len / 2.0f  -1 , (float)s17_raw_len / 2.0f  -1 , (float)s18_raw_len / 2.0f  -1 , (float)s19_raw_len / 2.0f -1 ,  // replace Samples
                                    (float)s20_raw_len / 2.0f  -1 , (float)s21_raw_len / 2.0f  -1 , (float)s22_raw_len / 2.0f  -1 , (float)s23_raw_len / 2.0f -1 ,
                                    (float)s24_raw_len / 2.0f  -1 , (float)s25_raw_len / 2.0f  -1 , (float)s26_raw_len / 2.0f -1 ,  (float)s27_raw_len / 2.0f -1 ,
                                    (float)s28_raw_len / 2.0f -1 ,  (float)s29_raw_len / 2.0f -1 ,  (float)s30_raw_len / 2.0f -1 ,  (float)s31_raw_len / 2.0f -1                                     
                                  } ; // 2 bytes  as one sample -> safe as float
    
    const uint16_t * samples [32] = { (uint16_t*)&s0_raw,   (uint16_t*)&s1_raw,  (uint16_t*)&s2_raw,   (uint16_t*)&s3_raw,
                                      (uint16_t*)&s4_raw,   (uint16_t*)&s5_raw,  (uint16_t*)&s6_raw,   (uint16_t*)&s7_raw,
                                      (uint16_t*)&s8_raw,   (uint16_t*)&s9_raw,  (uint16_t*)&s10_raw,  (uint16_t*)&s11_raw,
                                      (uint16_t*)&s12_raw,  (uint16_t*)&s13_raw, (uint16_t*)&s14_raw,  (uint16_t*)&s15_raw, 

                                      (uint16_t*)&s16_raw,  (uint16_t*)&s17_raw, (uint16_t*)&s18_raw,  (uint16_t*)&s19_raw,
                                      (uint16_t*)&s20_raw,  (uint16_t*)&s21_raw, (uint16_t*)&s22_raw,  (uint16_t*)&s23_raw,
                                      (uint16_t*)&s24_raw,  (uint16_t*)&s25_raw, (uint16_t*)&s26_raw,  (uint16_t*)&s27_raw,
                                      (uint16_t*)&s28_raw,  (uint16_t*)&s29_raw, (uint16_t*)&s30_raw,  (uint16_t*)&s31_raw
                                    };


    if(leftSamples>0){
      thea_sample+=inc_sample;
      if(thea_sample>1.0f){
        thea_sample=0.0f;
        leftSamples--;
      }
      uint16_t sample_h = samples[samplePrg][(uint32_t)(sample_mul[samplePrg] * thea_sample)];  // extend to 32 Bit
      DAC->DATA[1].reg = (uint16_t)sample_h >> 4;
    }

  }

  // PORT->Group[PORTA].OUTCLR.reg = 1ul << 22;
}





void loop() {

  // S/H Speed -------------------------------------------------------

  uint16_t noise_pitch_jack = adc51.readAnalog(PB01,ADC_Channel13,false);      // Buchse #2 (erste Digitale) Signal: Digital_Noise_Pitch normalized 12v
  uint16_t noise_pitch_poti = adc51.readAnalog(PB02,ADC_Channel14,false);      // Poti #2  Signal: Manual_Digital_Noise_Pitch
  uint32_t noise_pitch_sum = ((noise_pitch_poti + noise_pitch_jack) );
  if(noise_pitch_jack>4000){  // if is connected
     noise_pitch_sum = noise_pitch_poti;
  }

  if(noise_pitch_sum> 4095) noise_pitch_sum= 4095;
  inc_noise = adc51.adcToInc[noise_pitch_sum] * 4.0f;
  if(inc_noise> 0.99)inc_noise=0.99;

  // -----------------------------------------------------------------
  // SAMPLE Speed
  uint16_t sample_pitch_poti = adc51.readAnalog(PA07,ADC_Channel7,false); 
  uint16_t sample_pitch_jack = adc51.readAnalog(PA06,ADC_Channel8,true);  
  uint16_t sample_pitch_sum =  sample_pitch_poti;
  if(sample_pitch_jack<4095){  // if is connected
     sample_pitch_sum += sample_pitch_jack;
  }
  if(sample_pitch_sum > 4095){
    sample_pitch_sum = 4095;
  }
  float clk_sample_f = pitches[(sample_pitch_sum >> 2) & 0x03ff];  // Limit 1024 array size
  inc_sample = clk_sample_f ;
  if(inc_sample>1.0f) inc_sample=1.0f;
  if(inc_sample<0.000001f) inc_sample=0.000001f;  

  // 8Bit ChipMusic Speed
  inc_8bit =  inc_sample;

  // -----------------------------------------------------------------
  // SQR Speed
  uint16_t sqr_pitch_poti = adc51.readAnalog(PB03,ADC_Channel15,false);  // Manual_6XSqr_Pitch Poti
  uint16_t sqr_pitch_jack = adc51.readAnalog(PB04,ADC_Channel6,true);    // CV_6XSqr_Pitch Poti
  uint16_t sqr_pitch_sum = ((sqr_pitch_poti + sqr_pitch_jack)  );
  if(sqr_pitch_jack > 4000)
    sqr_pitch_sum = sqr_pitch_poti;

  if(sqr_pitch_sum>4000) sqr_pitch_sum=4000;


  float inc_sqr = adc51.adcToInc[sqr_pitch_sum];
  thea_inc[0]=  spreads[0] * inc_sqr;
  thea_inc[1]=  spreads[1] * inc_sqr;
  thea_inc[2]=  spreads[2] * inc_sqr;
  thea_inc[3]=  spreads[3] * inc_sqr;
  thea_inc[4]=  spreads[4] * inc_sqr;
  thea_inc[5]=  spreads[5] * inc_sqr;

  // Bank Switch
  uint16_t spread_bank_offset=0;
  if(!(PORT->Group[PORTA].IN.reg & (1ul << 21))) // PA21 button A/B Bank
    spread_bank_offset=16;

  // prg spread
  uint16_t spread_adc = adc51.readAnalog(PB05,ADC_Channel7,true);
  uint16_t spread   = MAP_RANGE(spread_adc,250,3700, 0, 15) + spread_bank_offset;
  


  switch(spread){
    case 0: // all Off
      flt_TRS[0]= -3.0f;      flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= -3.0f;      sq_TRS[1]= 3.0f;        sq_TRS[2]= -3.0f;       sq_TRS[3]= 3.0f;       sq_TRS[4]= -3.0f;        sq_TRS[5]= 3.0f;
      spreads[0]= 1.001f;   spreads[1]= 1.001f;    spreads[2]= 1.0001f;    spreads[3]= 1.0001f;   spreads[4]= 1.0001f;     spreads[5]= 1.0001f;
      thea[1] = thea[0]; // sync phases
      break;    
    case 1:
      flt_TRS[0]= -3.0f;    flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;      flt_TRS[3]= 3.0f;      flt_TRS[4]= -3.0f;       flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.2f;      sq_TRS[1]= -0.3f;        sq_TRS[2]= -3.0f;       sq_TRS[3]= 3.0f;       sq_TRS[4]= -3.0f;        sq_TRS[5]= 3.0f;
      spreads[0]= 1.001f;   spreads[1]= 1.001f;      spreads[2]= 1.0001f;    spreads[3]= 1.0001f;   spreads[4]= 1.0001f;     spreads[5]= 1.0001f;
                            thea[1] = thea[0]; // sync phases
      break;
    case 2:
      flt_TRS[0]= 0.2f;      flt_TRS[1]= 0.8f;        flt_TRS[2]= 0.9f;       flt_TRS[3]= 0.4f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.2f;       sq_TRS[1]= -0.2f;        sq_TRS[2]= 0.4f;        sq_TRS[3]= -0.4f;        sq_TRS[4]= 0.5f;       sq_TRS[5]= 0.6f;
      spreads[0]= 1.0000f;   spreads[1]= 1.0000f;    spreads[2]= 1.0000f;    spreads[3]= 1.0000f;    spreads[4]= 1.0000f;    spreads[5]= 1.0000f;
                             thea[1] = thea[0];     thea[2] = thea[0];      thea[3] = thea[0];       thea[4] = thea[0];      thea[5] = thea[0]; 
      break;
    case 3:
      flt_TRS[0]= -3.0f;      flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= -3.0f;       sq_TRS[5]= 3.0f;
      spreads[0]= 1.001f;   spreads[1]= 1.0015f;   spreads[2]= 1.002f;       spreads[3]= 1.003f;    spreads[4]= 1.004f;    spreads[5]= 1.005f;
      break;
    case 4:
      flt_TRS[0]= -3.0f;      flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.0f;        sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.0f;       spreads[2]= 0.416666f;  spreads[3]= 0.416666f;  spreads[4]= 0.583333f;    spreads[5]= 0.583333f;
      break;
    case 5:
      flt_TRS[0]= -3.0f;      flt_TRS[1]= -3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.0f;        sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;    spreads[1]= 0.5;     spreads[2]= 1.20f;    spreads[3]= 1.50f;            spreads[4]= 1.875f;         spreads[5]= 1.8f;
      break;
    case 6: // SUS2
      flt_TRS[0]= -3.0f;      flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.0f;        sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;    spreads[1]= 1.0;     spreads[2]= 1.33333f;    spreads[3]= 1.33333333f;        spreads[4]= 1.5f; spreads[5]= 1.5f;
      break;      
    case 7: // Dur
      flt_TRS[0]= -3.0f;      flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.0f;        sq_TRS[5]= 0.0f;
      spreads[0]= 1.2f;    spreads[1]= 1.2;     spreads[2]= 1.5f;    spreads[3]= 1.5f; spreads[4]= 1.8f; spreads[5]= 1.8f;
      break;  
    case 8: // Moll 6/5 
      flt_TRS[0]= -3.0f;      flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.0f;        sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;    spreads[1]= 1.0;     spreads[2]= 1.20f;    spreads[3]= 1.20f; spreads[4]= 1.5f; spreads[5]= 1.5f;
      break;  
    case 9:
      flt_TRS[0]= -3.0f;      flt_TRS[1]= 3.0f;        flt_TRS[2]= -3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= -3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.2f;       sq_TRS[1]= -0.2f;        sq_TRS[2]= 0.8f;        sq_TRS[3]= 0.2f;        sq_TRS[4]= 0.9f;        sq_TRS[5]= 0.1f;
      spreads[0]= 1.001f;    spreads[1]= 1.002f;     spreads[2]= 1.001f * 0.66666f;    spreads[3]= 1.002f*0.66666f; spreads[4]= 1.004f * 0.66666f * 2.0f; spreads[5]= 1.008f  * 0.66666f * 2.0f;
      break;  
    case 10:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;       flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;        sq_TRS[5]= 0.1f;
      spreads[0]= 1.0f;      spreads[1]= 1.249f;     spreads[2]= 1.667f;     spreads[3]= 2.0f;       spreads[4]= 2.503f;     spreads[5]= 3.004f;
      break;  
    case 11:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;       flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;        sq_TRS[5]= 0.1f;
      spreads[0]= 1.0f;      spreads[1]= 1.249f;     spreads[2]= 1.506f;     spreads[3]= 1.874f;     spreads[4]= 2.252f;     spreads[5]= 3.07f;
      break;  
    case 12:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;       flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;        sq_TRS[5]= 0.1f;
      spreads[0]= 1.001f;    spreads[1]= 2.0f;       spreads[2]= 3.0f;       spreads[3]= 4.0f;       spreads[4]= 5.0f;       spreads[5]= 6.0f;
      break;  
    case 13:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;       flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.0f;        sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.067f;     spreads[2]= 1.099f;     spreads[3]= 1.111f;     spreads[4]= 1.222f;     spreads[5]= 1.417f;
      break;  
    case 14:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.0f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.0f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.534f;     spreads[2]= 2.199f;     spreads[3]= 2.352f;     spreads[4]= 2.377f;      spreads[5]= 2.614f;
      break;  
    case 15:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.11f;      spreads[2]= 1.211f;     spreads[3]= 1.314f;     spreads[4]= 1.379f;      spreads[5]= 1.401f;
      break;  




  // BANK 2
    case 16:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 2.211f;      spreads[2]= 2.513f;     spreads[3]= 2.632f;     spreads[4]= 3.316f;      spreads[5]= 3.682f;
      break;  
    case 17:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.539f;      spreads[2]= 1.776f;     spreads[3]= 2.224f;     spreads[4]= 2.632f;      spreads[5]= 3.316f;
      break;  
    case 18:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.105f;      spreads[2]= 1.171f;     spreads[3]= 1.237f;     spreads[4]= 1.276f;      spreads[5]= 1.355f;
      break;  
    case 19:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 2.01f;      spreads[2]= 2.99f;     spreads[3]= 4.02f;     spreads[4]= 4.97f;      spreads[5]= 6.03f;
      break;  
    case 20:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 3.346f;      spreads[2]= 3.564f;     spreads[3]= 3.974f;     spreads[4]= 5.282f;      spreads[5]= 5.949f;
      break;  
    case 21:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.141f;      spreads[2]= 1.218f;     spreads[3]= 1.333f;     spreads[4]= 1.5f;      spreads[5]= 1.603f;
      break;  
    case 22:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.769f;      spreads[2]= 2.244f;     spreads[3]= 2.666f;     spreads[4]= 3.0f;      spreads[5]= 3.551f;
      break;  
    case 23:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.012f;      spreads[2]= 2.011f;     spreads[3]= 2.034f;     spreads[4]= 2.391f;      spreads[5]= 2.414f;
      break;
    case 24:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.013f;      spreads[2]= 1.506f;     spreads[3]= 2.023f;     spreads[4]= 2.413f;      spreads[5]= 3.034f;
      break;  
    case 25:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.131f;      spreads[2]= 1.187f;     spreads[3]= 1.285f;     spreads[4]= 1.355f;      spreads[5]= 1.480f;
      break;  
    case 26:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.537f;      spreads[2]= 2.349f;     spreads[3]= 2.752f;     spreads[4]= 2.357f;      spreads[5]= 3.114f;
      break;  
    case 27:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.572f;      spreads[2]= 1.845f;     spreads[3]= 2.2f;     spreads[4]= 3.379f;      spreads[5]= 3.401f;
      break;
   case 28:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.346f;      spreads[1]= 2.015f;      spreads[2]= 3.584f;     spreads[3]= 4.792f;     spreads[4]= 7.162f;      spreads[5]= 10.754f;
      break;  
    case 29:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 2.515f;      spreads[1]= 3.015f;      spreads[2]= 3.777f;     spreads[3]= 4.523f;     spreads[4]= 5.654f;      spreads[5]= 6.785f;
      break;  
    case 30:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.508f;      spreads[2]= 2.015f;     spreads[3]= 2.5156f;     spreads[4]= 3.015f;      spreads[5]= 3.577f;
      break;  
    case 31:
      flt_TRS[0]= 3.0f;      flt_TRS[1]= 3.0f;       flt_TRS[2]= 3.0f;       flt_TRS[3]= 3.0f;       flt_TRS[4]= 3.0f;        flt_TRS[5]= 3.0f;
      sq_TRS[0]= 0.0f;       sq_TRS[1]= 0.0f;        sq_TRS[2]= 0.7f;        sq_TRS[3]= 0.0f;        sq_TRS[4]= 0.9f;         sq_TRS[5]= 0.0f;
      spreads[0]= 1.0f;      spreads[1]= 1.792f;      spreads[2]= 2.015f;     spreads[3]= 2.392f;     spreads[4]= 3.584f;      spreads[5]= 4.523f;
      break;






  }

  

  uint16_t cv_sample_select_adc = adc51.readAnalog(PB09,ADC_Channel3,false);


  uint16_t prg8_smpl_select_adc = adc51.readAnalog(PA06,ADC_Channel6,false);
  if(cv_sample_select_adc < 4000 ) // if jack is conneced (no normalized)
    prg8_smpl_select_adc+=cv_sample_select_adc;
  
  int16_t prg8_smpl_select    = MAP_RANGE(prg8_smpl_select_adc,310,3750, 0, 15);
  
  uint16_t smpl_bank_offset=0;
  if(!(PORT->Group[PORTA].IN.reg & (1ul << 20))) // PA20 button A/B Bank
    smpl_bank_offset=16;

  uint16_t t_prg_select = (prg8_smpl_select + smpl_bank_offset) ; // fix a possible crash

  prg8      = t_prg_select & 0x1f;
  samplePrg = t_prg_select & 0x1f;

  uint16_t ratchet_adc          = adc51.readAnalog(PB07,ADC_Channel9,true);
  uint16_t ratchet_adc_select   = MAP_RANGE(ratchet_adc,300,3750, 0, 15);

  if(ratchet_adc_select == 15)
    ratchet_counts = 0xffffffff;
  else
    ratchet_counts = ratchet_adc_select + 1;

/** /

  static int dsbug = 0;
  dsbug++;
  if(!(dsbug % 500)){
    Serial.print(samplePrg,DEC);
    Serial.print(" ");
    Serial.print(prg8_smpl_select);
    Serial.print(" ");
    Serial.print(prg8_smpl_select_adc,DEC);
    Serial.print(" ");
    Serial.print(cv_sample_select_adc,DEC);
//    Serial.print(" PA:");
//    Serial.print(PORT->Group[PORTA].IN.reg,BIN);
//    Serial.print(" PB:");
//    Serial.print(PORT->Group[PORTB].IN.reg,BIN);
    Serial.print(" ");
    Serial.println(spread,DEC);
  }
/**/


}
