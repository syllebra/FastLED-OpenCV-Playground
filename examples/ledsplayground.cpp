#include "flogl/flogl.hpp"
#include <FastLED.h>
#include <math.h>
#include <unistd.h>
#include <cstdio>
#include <ctime>

std::clock_t start = std::clock();

uint32_t get_millisecond_timer()
{
  return (std::clock() - start)*10000 / (double) (CLOCKS_PER_SEC);
}
#define millis GET_MILLIS

float vsc = 0.5;
float hsc = 0.3;

using flogl::LED;

#define NUM_LEDS 60
std::vector<LED> led_coords;
flogl::Flogl FastLEDev(led_coords);
   
CRGB* leds;

void loop();
void initLedsPos();

int main()
{
   initLedsPos();
   leds = new CRGB[led_coords.size()];


   CRGB* col = leds;
   for (LED& led: led_coords)
   {
      led.color = col++;
   }
   
   int key = 0;
   do {
      //usleep(3000);
      loop();
      key = FastLEDev.draw();
   } while(key != 27);
   
   delete[] leds;
   
   return 0;
}

#define FastLED FastLEDev
int random(int min, int max)
{
   float tmp = random() / RAND_MAX; 
   return int(min + (max-min)*tmp);
}

int random(int max)
{
   return random(0,max);
}


void changeEffect();


/*
 * paste in the Fire2012 code with a small edit at the end which uses the
 * setPixelColor() function to copy the color data to the ws2812fx instance. 
*/

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  100

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120
bool gReverseDirection = false;
void Fire2012()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }

    leds[pixelnumber] = color;
    }
}

uint8_t circle_id(uint8_t i, int dec,int max=NUM_LEDS){
      return i+dec <0 ? max+i+dec : (i+dec > max ? i+dec-max -1: i+dec) ;
}

void cool_down(uint8_t *v, uint8_t dec=1)
{
    for( int i = 0; i < NUM_LEDS; i++) {
      v[i] = qsub8(v[i], dec);
    }  
}

void heat_to_col(uint8_t *h, CRGB *d, float mult=1, uint8_t hue = 240)
{
    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      int v = h[j]*mult;
      v = v > 255 ? 255 : v;
      CRGB color = HeatColor(v);
      CHSV hsv = rgb2hsv_approximate(color);
      hsv.h = hue;//mod8(hsv.h+5, 255);
      d[j] = hsv;
    }
}

void heat_to_hue(uint8_t *h, CRGB *d)
{
    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CHSV hsv;
      hsv.h = 255-h[j];
      hsv.s= 255;
      hsv.v= qmul8(h[j],4);;// == 0 ? 0 : 255;
      d[j] = hsv;
    }
}


int diffuse_heat(uint8_t *h, float mult = 0.3)
{
  static uint8_t d[NUM_LEDS];
  
    for( int k = 0; k < NUM_LEDS; k++)
      d[k] = 0;

  int sum = 0;
    // Step 2.  Diffuse
    for( int k = 0; k < NUM_LEDS; k++) {
      int kp = k == 0 ? NUM_LEDS-1 : k-1;
      int kn = k == NUM_LEDS-1 ? 0 : k+1;
      d[k] = (h[k] + h[kp] + h[kn] ) *mult;
      sum += h[k];
    }

    for( int k = 0; k < NUM_LEDS; k++)
      h[k] = d[k];
    return sum;
}


void hueshift(uint8_t shift,float mult = 4.0)
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if(shift>0)
    {
      CHSV hsv = rgb2hsv_approximate(leds[i]);
      hsv.h = (hsv.h+shift)%256;
      leds[i] = hsv;
    }
    leds[i].r = qmul8(leds[i].r,mult);
    leds[i].g = qmul8(leds[i].g,mult);
    leds[i].b = qmul8(leds[i].b,mult);

  }
}

void wave()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];
  static int sparkid_id = 0;
  // Step 1.  Cool down every cell a little
    cool_down(heat,1);
 
    // Step 2.  Diffuse
    int sum = diffuse_heat(heat,0.30);

    if(sum < 10)
    {
      sparkid_id = random8(NUM_LEDS);
      heat[sparkid_id] = 255;
      heat[circle_id(sparkid_id,-2)] =255;
      heat[circle_id(sparkid_id,2)] =255;
    }

    heat_to_col(heat,leds);
    hueshift(150,4);
    // FastLEDev.show();
    // FastLEDev.delay(100);
}


void wave2(  int start = 15, int delay = 25, uint8_t hue = 240, bool rainbow = false, int cooldown=14, int delta = NUM_LEDS/3)
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  int id1 = start;
  int id2 = start;
  for(int i=0;i<NUM_LEDS-delta;i++)
  {
    if(i<NUM_LEDS/2+1)
    {
      heat[id1] = 255;
      heat[id2] = 255;
      id1 = circle_id(id1,-1);
      id2 = circle_id(id2, 1);
    }
    cool_down(heat,cooldown);
    if(rainbow)
      heat_to_hue(heat, leds);
    else
      heat_to_col(heat, leds,1.2, hue);
    FastLEDev.show();
    FastLEDev.delay(delay);
  }
}

void lowton(int start=15, int delay = 25)
{
  int starts[4]={start, NUM_LEDS-start,start, NUM_LEDS-start};
  int hues[4]={150, 136,150, 136};

  for(int s = 0;s<4;s++)
    wave2(starts[s],delay,hues[s]);
}

void hattrick(int start=15, int delay = 15)
{
  int starts[4]={start, NUM_LEDS-start,start, NUM_LEDS-start};

  for(int s = 0;s<4;s++)
    wave2(starts[s],delay,0, true,6,-4);
}

void hitbull(int pct_on = 30)
{
  static uint8_t heat[NUM_LEDS];
  static uint32_t cycle = 0;

    cool_down(heat,4);

    // Step 2.  Diffuse
    int sum = diffuse_heat(heat,0.30);

    if(cycle%4 == 0 && cycle<25)
    {
      for(int i=0;i<NUM_LEDS;i++)
        if(random8(100)<pct_on)
          heat[i] = 128;
    }

    heat_to_col(heat,leds);
    //hueshift(150,4);
    // FastLEDev.show();
    // FastLEDev.delay();
    cycle++;

    if(cycle>25 && sum <10)
    cycle = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The code for this animation is more complicated than other examples, and 
// while it is "ready to run", and documented in general, it is probably not 
// the best starting point for learning.  Nevertheless, it does illustrate some
// useful techniques.
//
//////////////////////////////////////////////////////////////////////////
//
// In this animation, there are four "layers" of waves of light.  
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then 
// another filter is applied that adds "whitecaps" of brightness where the 
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent 
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
//
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };



// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}


void pacifica_loop(float speed = 1.0)
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS() * speed;
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();

}

#define MAX(a,b) ((a)>(b) ? (a) : (b))

void mode_fire_flicker(CRGB color, uint8_t intensity=255) {
  
  uint8_t lum = MAX(color.r, MAX(color.g, color.b));
  //lum /= (((256-intensity)/16)+1);
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t flicker = random8(lum);
    //SEGMENT.setPixelColor(i, MAX(r - flicker, 0), MAX(g - flicker, 0), MAX(b - flicker, 0), MAX(w - flicker, 0));
    
    leds[i]=CRGB(MAX(color.r - flicker, 0), MAX(color.g - flicker, 0), MAX(color.b - flicker, 0));
    //SEGMENT.setPixelColor(i, SEGMENT.color_from_palette(i, true, PALETTE_SOLID_WRAP, 0, 255 - flicker));
  }
  blur1d(leds,NUM_LEDS,172);
}

//////////////////////////////////////////////////////////////


void running_lights(CRGB color = CRGB::Red, int WaveDelay=50) {
  static int Position=0;
 
  // for(int i=0; i<NUM_LEDS*2; i++)
  // {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        float level = (sin(i+Position) * 127 + 128) / 255.0; 
        leds[i].r = level*color.r;
        leds[i].g = level*color.g;
        leds[i].b = level*color.b;
      }
     
      // FastLEDev.show();
      // FastLEDev.delay(WaveDelay);
  //}
}

void loop()
{
   //static uint8_t hue = 0;
   // fill_rainbow(leds, led_coords.size(), hue+=4, 255/NUM_LEDS);
   //changeEffect();
   hitbull();
   //lowton();
   //hattrick();
   //wave();
   //mode_fire_flicker(CRGB::Red,120);
   //shimmer();


  //   //running_lights();
  //  uint32_t ms = GET_MILLIS() * 50;
  //   hue = (ms/1000) %256;
  // //   pacifica_loop(5);
  //    hueshift( hue,1.5);
    FastLEDev.delay(50);
}

void initLedsPos()
{
   float a = 0;
   float inc = M_PI * 2.0 / (NUM_LEDS);
   for(int i=0;i<NUM_LEDS;i++)
   {
      LED led(500 + cos(a)*300,350+ sin(a)*300,-20,10);
      led_coords.push_back(led);
      a+=inc;
   }
}

#define delay FastLEDev.delay
#define byte uint8_t
#define boolean bool


// ***************************************
// ** FastLed/NeoPixel Common Functions **
// ***************************************

// Apply LED color changes
void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}

// Set a LED color (not yet visible)
void setPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}

// Set all LEDs to a given color and apply it (visible)
void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  showStrip();
}


void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}


// used by meteorrain
void fadeToBlack(int ledNo, byte fadeValue) {
 #ifdef ADAFRUIT_NEOPIXEL_H
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
   
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   
    strip.setPixelColor(ledNo, r,g,b);
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}


// *************************
// ** LEDEffect Functions **
// *************************

void RGBLoop(){
  for(int j = 0; j < 3; j++ ) {
    // Fade IN
    for(int k = 0; k < 256; k++) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      delay(3);
    }
    // Fade OUT
    for(int k = 255; k >= 0; k--) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      delay(3);
    }
  }
}

void FadeInOut(byte red, byte green, byte blue){
  float r, g, b;
     
  for(int k = 0; k < 256; k=k+1) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
     
  for(int k = 255; k >= 0; k=k-2) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
}

void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause){
  for(int j = 0; j < StrobeCount; j++) {
    setAll(red,green,blue);
    showStrip();
    delay(FlashDelay);
    setAll(0,0,0);
    showStrip();
    delay(FlashDelay);
  }
 
 delay(EndPause);
}

void HalloweenEyes(byte red, byte green, byte blue,
                   int EyeWidth, int EyeSpace,
                   boolean Fade, int Steps, int FadeDelay,
                   int EndPause){
  //randomSeed(analogRead(0));
 
  int i;
  int StartPoint  = random( 0, NUM_LEDS - (2*EyeWidth) - EyeSpace );
  int Start2ndEye = StartPoint + EyeWidth + EyeSpace;
 
  for(i = 0; i < EyeWidth; i++) {
    setPixel(StartPoint + i, red, green, blue);
    setPixel(Start2ndEye + i, red, green, blue);
  }
 
  showStrip();
 
  if(Fade==true) {
    float r, g, b;
 
    for(int j = Steps; j >= 0; j--) {
      r = j*(red/Steps);
      g = j*(green/Steps);
      b = j*(blue/Steps);
     
      for(i = 0; i < EyeWidth; i++) {
        setPixel(StartPoint + i, r, g, b);
        setPixel(Start2ndEye + i, r, g, b);
      }
     
      showStrip();
      delay(FadeDelay);
    }
  }
 
  setAll(0,0,0); // Set all black
 
  delay(EndPause);
}

void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){

  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
 
  delay(ReturnDelay);
}

// used by NewKITT
void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i =((NUM_LEDS-EyeSize)/2); i>=0; i--) {
    setAll(0,0,0);
   
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
   
    setPixel(NUM_LEDS-i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(NUM_LEDS-i-j, red, green, blue);
    }
    setPixel(NUM_LEDS-i-EyeSize-1, red/10, green/10, blue/10);
   
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i<=((NUM_LEDS-EyeSize)/2); i++) {
    setAll(0,0,0);
   
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
   
    setPixel(NUM_LEDS-i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(NUM_LEDS-i-j, red, green, blue);
    }
    setPixel(NUM_LEDS-i-EyeSize-1, red/10, green/10, blue/10);
   
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

void Twinkle(byte red, byte green, byte blue, int Count, int SpeedDelay, boolean OnlyOne) {
  for(int i=0;i<NUM_LEDS*2;i++)
  {
  setAll(0,0,0);
 
  for (int i=0; i<Count; i++) {
     setPixel(random(NUM_LEDS),red,green,blue);
     showStrip();
     delay(SpeedDelay);
     if(OnlyOne) {
       setAll(0,0,0);
     }
   }
 
  delay(SpeedDelay);
  }
}

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne) {
  
  setAll(0,0,0);
 
  for (int i=0; i<Count; i++) {
     setPixel(random(NUM_LEDS),random(0,255),random(0,255),random(0,255));
     showStrip();
     delay(SpeedDelay);
     if(OnlyOne) {
       setAll(0,0,0);
     }
   }
 
  delay(SpeedDelay);
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay) {
  for(int i=0;i<NUM_LEDS*2;i++)
  {
    int Pixel = random(NUM_LEDS);
    setPixel(Pixel,red,green,blue);
    showStrip();
    delay(SpeedDelay);
    setPixel(Pixel,0,0,0);
  }
}

void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay) {
  for(int i=0;i<NUM_LEDS*2;i++)
  {  
    setAll(red,green,blue);
  
    int Pixel = random(NUM_LEDS);
    setPixel(Pixel,0xff,0xff,0xff);
    showStrip();
    delay(SparkleDelay);
    setPixel(Pixel,red,green,blue);
    showStrip();
    delay(SpeedDelay);
  }
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;
 
  for(int i=0; i<NUM_LEDS*2; i++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }
     
      showStrip();
      delay(WaveDelay);
  }
}

void colorWipe(byte red, byte green, byte blue, int SpeedDelay) {
  for(uint16_t i=0; i<NUM_LEDS; i++) {
      setPixel(i, red, green, blue);
      showStrip();
      delay(SpeedDelay);
  }
}


// used by rainbowCycle and theaterChaseRainbow
byte * Wheel(byte WheelPos) {
  static byte c[3];
 
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}


void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< NUM_LEDS; i++) {
      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      setPixel(i, *c, *(c+1), *(c+2));
    }
    showStrip();
    delay(SpeedDelay);
  }
}

void theaterChase(byte red, byte green, byte blue, int SpeedDelay) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < NUM_LEDS; i=i+3) {
        setPixel(i+q, red, green, blue);    //turn every third pixel on
      }
      showStrip();
     
      delay(SpeedDelay);
     
      for (int i=0; i < NUM_LEDS; i=i+3) {
        setPixel(i+q, 0,0,0);        //turn every third pixel off
      }
    }
  }
}

void theaterChaseRainbow(int SpeedDelay) {
  byte *c;
 
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < NUM_LEDS; i=i+3) {
          c = Wheel( (i+j) % 255);
          setPixel(i+q, *c, *(c+1), *(c+2));    //turn every third pixel on
        }
        showStrip();
       
        delay(SpeedDelay);
       
        for (int i=0; i < NUM_LEDS; i=i+3) {
          setPixel(i+q, 0,0,0);        //turn every third pixel off
        }
    }
  }
}

void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS];
  int cooldown;
 
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
   
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  showStrip();
  delay(SpeedDelay);
}


void BouncingColoredBalls(int BallCount, byte colors[][3], boolean continuous) {
  float Gravity = -9.81;
  int StartHeight = 1;
 
  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];
  boolean ballBouncing[BallCount];
  boolean ballsStillBouncing = true;
 
  for (int i = 0 ; i < BallCount ; i++) {  
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2);
    ballBouncing[i]=true;
  }

  while (ballsStillBouncing) {
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;
 
      if ( Height[i] < 0 ) {                      
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();
 
        if ( ImpactVelocity[i] < 0.01 ) {
          if (continuous) {
            ImpactVelocity[i] = ImpactVelocityStart;
          } else {
            ballBouncing[i]=false;
          }
        }
      }
      Position[i] = round( Height[i] * (NUM_LEDS - 1) / StartHeight);
    }

    ballsStillBouncing = false; // assume no balls bouncing
    for (int i = 0 ; i < BallCount ; i++) {
      setPixel(Position[i],colors[i][0],colors[i][1],colors[i][2]);
      if ( ballBouncing[i] ) {
        ballsStillBouncing = true;
      }
    }
   
    showStrip();
    setAll(0,0,0);
  }
}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
 
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      }
    }
   
    showStrip();
    delay(SpeedDelay);
  }
}

// *** REPLACE TO HERE ***




void NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

void setEffect(int selectedEffect) {
switch(selectedEffect) {
   
    case 0  : {
                // RGBLoop - no parameters
                RGBLoop();
                break;
              }

    case 1  : {
                // FadeInOut - Color (red, green. blue)
                FadeInOut(0xff, 0x00, 0x00); // red
                FadeInOut(0xff, 0xff, 0xff); // white
                FadeInOut(0x00, 0x00, 0xff); // blue
                break;
              }
             
    case 2  : {
                // Strobe - Color (red, green, blue), number of flashes, flash speed, end pause
                Strobe(0xff, 0xff, 0xff, 10, 50, 1000);
                break;
              }

    case 3  : {
                // HalloweenEyes - Color (red, green, blue), Size of eye, space between eyes, fade (true/false), steps, fade delay, end pause
                HalloweenEyes(0xff, 0x00, 0x00,
                              1, 4,
                              true, random(5,50), random(50,150),
                              random(1000, 10000));
                HalloweenEyes(0xff, 0x00, 0x00,
                              1, 4,
                              true, random(5,50), random(50,150),
                              random(1000, 10000));
                break;
              }
             
    case 4  : {
                // CylonBounce - Color (red, green, blue), eye size, speed delay, end pause
                CylonBounce(0xff, 0x00, 0x00, 4, 10, 50);
                break;
              }
             
    case 5  : {
                // NewKITT - Color (red, green, blue), eye size, speed delay, end pause
                NewKITT(0xff, 0x00, 0x00, 8, 10, 50);
                break;
              }
             
    case 6  : {
                // Twinkle - Color (red, green, blue), count, speed delay, only one twinkle (true/false)
                Twinkle(0xff, 0x00, 0x00, 10, 100, false);
                break;
              }
             
    case 7  : {
                // TwinkleRandom - twinkle count, speed delay, only one (true/false)
                TwinkleRandom(20, 100, false);
                break;
              }
             
    case 8  : {
                // Sparkle - Color (red, green, blue), speed delay
                Sparkle(0xff, 0xff, 0xff, 0);
                break;
              }
               
    case 9  : {
                // SnowSparkle - Color (red, green, blue), sparkle delay, speed delay
                SnowSparkle(0x10, 0x10, 0x10, 20, random(100,1000));
                break;
              }
             
    case 10 : {
                // Running Lights - Color (red, green, blue), wave dealy
                RunningLights(0xff,0x00,0x00, 50);  // red
                RunningLights(0xff,0xff,0xff, 50);  // white
                RunningLights(0x00,0x00,0xff, 50);  // blue
                break;
              }
             
    case 11 : {
                // colorWipe - Color (red, green, blue), speed delay
                colorWipe(0x00,0xff,0x00, 50);
                colorWipe(0x00,0x00,0x00, 50);
                break;
              }

    case 12 : {
                // rainbowCycle - speed delay
                rainbowCycle(20);
                break;
              }

    case 13 : {
                // theatherChase - Color (red, green, blue), speed delay
                theaterChase(0xff,0,0,50);
                break;
              }

    case 14 : {
                // theaterChaseRainbow - Speed delay
                theaterChaseRainbow(50);
                break;
              }

    case 15 : {
                // Fire - Cooling rate, Sparking rate, speed delay
                Fire(55,120,15);
                break;
              }


              // simple bouncingBalls not included, since BouncingColoredBalls can perform this as well as shown below
              // BouncingColoredBalls - Number of balls, color (red, green, blue) array, continuous
              // CAUTION: If set to continuous then this effect will never stop!!!
             
    case 16 : {
                // mimic BouncingBalls
                byte onecolor[1][3] = { {0xff, 0x00, 0x00} };
                BouncingColoredBalls(1, onecolor, false);
                break;
              }

    case 17 : {
                // multiple colored balls
                byte colors[3][3] = { {0xff, 0x00, 0x00},
                                      {0xff, 0xff, 0xff},
                                      {0x00, 0x00, 0xff} };
                BouncingColoredBalls(3, colors, false);
                break;
              }

    case 18 : {
                // meteorRain - Color (red, green, blue), meteor size, trail decay, random trail decay (true/false), speed delay
                meteorRain(0xff,0xff,0xff,10, 64, true, 30);
                break;
              }
  }
}

int selectedEffect=17;
void changeEffect() {
//   if (digitalRead (BUTTON) == HIGH) {
//     selectedEffect++;
//     EEPROM.put(0, selectedEffect);
//     asm volatile ("  jmp 0");
//   }
   selectedEffect =((selectedEffect+1) % 19);
   printf("effect:%d\n",selectedEffect);
   setEffect(selectedEffect);
   changeEffect();
}
