/*
  xlgt_01_ws2812.ino - led string support for Tasmota

  Copyright (C) 2021  Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_LIGHT
#ifdef USE_WS2812
/*********************************************************************************************\
 * WS2812 RGB / RGBW Leds using NeopixelBus library
 *
 * light_scheme  WS2812  3+ Colors  1+2 Colors  Effect
 * ------------  ------  ---------  ----------  -----------------
 *  0            yes     no         no          Clock
 *  1            yes     no         no          Incandescent
 *  2            yes     no         no          RGB
 *  3            yes     no         no          Christmas
 *  4            yes     no         no          Hanukkah
 *  5            yes     no         no          Kwanzaa
 *  6            yes     no         no          Rainbow
 *  7            yes     no         no          Fire
 *
\*********************************************************************************************/

#define XLGT_01             1

#ifdef USE_DERG_RGB
#include <base64.hpp>
#endif // USE_DERG_RGB


#ifdef USE_DERG_RGB
const uint8_t WS2812_SCHEMES = 9;      // Number of WS2812 schemes
#else
const uint8_t WS2812_SCHEMES = 8;      // Number of WS2812 schemes
#endif // USE_DERG_RGB

const char kWs2812Commands[] PROGMEM = "|"  // No prefix
#ifdef USE_DERG_RGB
  D_CMND_DRAGON "|"
#endif // USE_DERG_RGB
  D_CMND_LED "|" D_CMND_PIXELS "|" D_CMND_ROTATION "|" D_CMND_WIDTH ;

void (* const Ws2812Command[])(void) PROGMEM = {
#ifdef USE_DERG_RGB
  &CmndDragon,
#endif // USE_DERG_RGB
  &CmndLed, &CmndPixels, &CmndRotation, &CmndWidth };

#include <NeoPixelBus.h>

// See NeoEspDmaMethod.h for available options
// See NeoEspBitBangMethod.h for available options

// Build `selectedNeoFeatureType` as Neo-Rgb-Feature
// parametrized as: NEO_FEATURE_NEO+NEO_FEATURE_TYPE+NEO_FEATURE_FEATURE
#define CONCAT2(A,B)    CONCAT2_(A,B)   // ensures expansion first, see https://stackoverflow.com/questions/3221896/how-can-i-guarantee-full-macro-expansion-of-a-parameter-before-paste
#define CONCAT2_(A,B)    A ## B
#define CONCAT3(A,B,C)    CONCAT3_(A,B,C)   // ensures expansion first, see https://stackoverflow.com/questions/3221896/how-can-i-guarantee-full-macro-expansion-of-a-parameter-before-paste
#define CONCAT3_(A,B,C)    A ## B ## C

#define NEO_FEATURE_NEO       Neo
#define NEO_FEATURE_FEATURE   Feature

// select the right Neo feature based on USE_WS2812_CTYPE
// NEO_FEATURE_TYPE can be one of: Rgb (default), Grb, Brg, Rgb, Rgbw, Grbw
#if   (USE_WS2812_CTYPE == NEO_GRB)
  #define NEO_FEATURE_TYPE  Grb
#elif (USE_WS2812_CTYPE == NEO_BRG)
  #define NEO_FEATURE_TYPE  Brg
#elif (USE_WS2812_CTYPE == NEO_RBG)
  #define NEO_FEATURE_TYPE  Rbg
#elif (USE_WS2812_CTYPE == NEO_RGBW)
  #define NEO_FEATURE_TYPE  Rgbw
#elif (USE_WS2812_CTYPE == NEO_GRBW)
  #define NEO_FEATURE_TYPE  Grbw
#else
  #define NEO_FEATURE_TYPE  Rgb
#endif

// Exception for NEO_HW_P9813
#if (USE_WS2812_HARDWARE == NEO_HW_P9813)
  #undef NEO_FEATURE_NEO
  #undef NEO_FEATURE_TYPE
  #define NEO_FEATURE_NEO     P9813   // P9813BgrFeature
  #define NEO_FEATURE_TYPE    Bgr
  #undef USE_WS2812_DMA
  #undef USE_WS2812_INVERTED
#endif  // USE_WS2812_CTYPE

typedef CONCAT3(NEO_FEATURE_NEO,NEO_FEATURE_TYPE,NEO_FEATURE_FEATURE) selectedNeoFeatureType;

// selectedNeoSpeedType is built as Neo+Esp8266+Dma+Inverted+Ws2812x+Method
// Or NEO_NEO+NEO_CHIP+NEO_PROTO+NEO_INV+NEO_HW+Method
#define CONCAT6(A,B,C,D,E,F)    CONCAT6_(A,B,C,D,E,F)   // ensures expansion first, see https://stackoverflow.com/questions/3221896/how-can-i-guarantee-full-macro-expansion-of-a-parameter-before-paste
#define CONCAT6_(A,B,C,D,E,F)    A ## B ## C ## D ## E ## F

#define NEO_NEO         Neo

#ifdef ESP32
  #define NEO_CHIP      Esp32
#else
  #define NEO_CHIP      Esp8266
#endif

// Proto = DMA or BigBang
#if defined(USE_WS2812_DMA) && defined(ESP8266)
  #define NEO_PROTO     Dma
#elif defined(USE_WS2812_RMT) && defined(ESP32)
  #define NEO_PROTO     CONCAT2(Rmt,USE_WS2812_RMT)
#elif defined(USE_WS2812_I2S) && defined(ESP32)
  #define NEO_PROTO     CONCAT2(I2s,USE_WS2812_I2S)
#else
  #define NEO_PROTO     BitBang
#endif

#ifdef USE_WS2812_INVERTED
  #define NEO_INV       Inverted
#else
  #define NEO_INV
#endif

#if (USE_WS2812_HARDWARE == NEO_HW_WS2812X)
  #define NEO_HW        Ws2812x
#elif (USE_WS2812_HARDWARE == NEO_HW_SK6812)
  #define NEO_HW        Sk6812
#elif (USE_WS2812_HARDWARE == NEO_HW_APA106)
  #define NEO_HW        Apa106
#else   // USE_WS2812_HARDWARE
  #define NEO_HW        800Kbps
#endif  // USE_WS2812_HARDWARE


#if (USE_WS2812_HARDWARE == NEO_HW_P9813)
  #undef NEO_NEO
  #define NEO_NEO
  #undef NEO_CHIP
  #define NEO_CHIP
  #undef NEO_PROTO
  #define NEO_PROTO
  #undef NEO_INV
  #define NEO_INV
  #undef NEO_HW
  #define NEO_HW      P9813       // complete driver is P9813Method
#endif

#if defined(ESP8266) && defined(USE_WS2812_DMA)
typedef CONCAT6(NEO_NEO,NEO_CHIP,NEO_PROTO,NEO_INV,NEO_HW,Method)   selectedNeoSpeedType;
#else // Dma : different naming scheme
typedef CONCAT6(NEO_NEO,NEO_CHIP,NEO_PROTO,NEO_HW,NEO_INV,Method)   selectedNeoSpeedType;
#endif

NeoPixelBus<selectedNeoFeatureType, selectedNeoSpeedType> *strip = nullptr;

struct WsColor {
  uint8_t red, green, blue;
};

struct ColorScheme {
  WsColor* colors;
  uint8_t count;
};

WsColor kIncandescent[2] = { 255,140,20, 0,0,0 };
WsColor kRgb[3] = { 255,0,0, 0,255,0, 0,0,255 };
WsColor kChristmas[2] = { 255,0,0, 0,255,0 };
WsColor kHanukkah[2] = { 0,0,255, 255,255,255 };
WsColor kwanzaa[3] = { 255,0,0, 0,0,0, 0,255,0 };
WsColor kRainbow[7] = { 255,0,0, 255,128,0, 255,255,0, 0,255,0, 0,0,255, 128,0,255, 255,0,255 };
WsColor kFire[3] = { 255,0,0, 255,102,0, 255,192,0 };

#ifdef USE_DERG_RGB
ColorScheme kSchemes[WS2812_SCHEMES -2] = {  // Skip clock+dragon schemes
#else
ColorScheme kSchemes[WS2812_SCHEMES -1] = {  // Skip clock scheme
#endif // USE_DERG_RGB
  kIncandescent, 2,
  kRgb, 3,
  kChristmas, 2,
  kHanukkah, 2,
  kwanzaa, 3,
  kRainbow, 7,
  kFire, 3 };

uint8_t kWidth[5] = {
    1,     // Small
    2,     // Medium
    4,     // Large
    8,     // Largest
  255 };   // All
uint8_t kWsRepeat[5] = {
    8,     // Small
    6,     // Medium
    4,     // Large
    2,     // Largest
    1 };   // All

struct WS2812 {
  uint8_t show_next = 1;
  uint8_t scheme_offset = 0;
  bool suspend_update = false;
} Ws2812;

/********************************************************************************************/

// For some reason map fails to compile so renamed to wsmap
long wsmap(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Ws2812StripShow(void)
{
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
#else
  RgbColor c;
#endif

  if (Settings->light_correction) {
    for (uint32_t i = 0; i < Settings->light_pixels; i++) {
      c = strip->GetPixelColor(i);
      c.R = ledGamma(c.R);
      c.G = ledGamma(c.G);
      c.B = ledGamma(c.B);
#if (USE_WS2812_CTYPE > NEO_3LED)
      c.W = ledGamma(c.W);
#endif
      strip->SetPixelColor(i, c);
    }
  }
  strip->Show();
}

int mod(int a, int b)
{
   int ret = a % b;
   if (ret < 0) ret += b;
   return ret;
}

void Ws2812UpdatePixelColor(int position, struct WsColor hand_color, float offset)
{
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor color;
#else
  RgbColor color;
#endif

  uint32_t mod_position = mod(position, (int)Settings->light_pixels);

  color = strip->GetPixelColor(mod_position);
  float dimmer = 100 / (float)Settings->light_dimmer;
  color.R = tmin(color.R + ((hand_color.red / dimmer) * offset), 255);
  color.G = tmin(color.G + ((hand_color.green / dimmer) * offset), 255);
  color.B = tmin(color.B + ((hand_color.blue / dimmer) * offset), 255);
  strip->SetPixelColor(mod_position, color);
}

void Ws2812UpdateHand(int position, uint32_t index)
{
  uint32_t width = Settings->light_width;
  if (index < WS_MARKER) { width = Settings->ws_width[index]; }
  if (!width) { return; }  // Skip

  position = (position + Settings->light_rotation) % Settings->light_pixels;

  if (Settings->flag.ws_clock_reverse) {  // SetOption16 - Switch between clockwise or counter-clockwise
    position = Settings->light_pixels -position;
  }
  WsColor hand_color = { Settings->ws_color[index][WS_RED], Settings->ws_color[index][WS_GREEN], Settings->ws_color[index][WS_BLUE] };

  Ws2812UpdatePixelColor(position, hand_color, 1);

  uint32_t range = ((width -1) / 2) +1;
  for (uint32_t h = 1; h < range; h++) {
    float offset = (float)(range - h) / (float)range;
    Ws2812UpdatePixelColor(position -h, hand_color, offset);
    Ws2812UpdatePixelColor(position +h, hand_color, offset);
  }
}

void Ws2812Clock(void)
{
  strip->ClearTo(0); // Reset strip
  int clksize = 60000 / (int)Settings->light_pixels;

  Ws2812UpdateHand((RtcTime.second * 1000) / clksize, WS_SECOND);
  Ws2812UpdateHand((RtcTime.minute * 1000) / clksize, WS_MINUTE);
  Ws2812UpdateHand((((RtcTime.hour % 12) * 5000) + ((RtcTime.minute * 1000) / 12 )) / clksize, WS_HOUR);
  if (Settings->ws_color[WS_MARKER][WS_RED] + Settings->ws_color[WS_MARKER][WS_GREEN] + Settings->ws_color[WS_MARKER][WS_BLUE]) {
    for (uint32_t i = 0; i < 12; i++) {
      Ws2812UpdateHand((i * 5000) / clksize, WS_MARKER);
    }
  }

  Ws2812StripShow();
}

void Ws2812GradientColor(uint32_t schemenr, struct WsColor* mColor, uint32_t range, uint32_t gradRange, uint32_t i)
{
/*
 * Compute the color of a pixel at position i using a gradient of the color scheme.
 * This function is used internally by the gradient function.
 */
  ColorScheme scheme = kSchemes[schemenr];
  uint32_t curRange = i / range;
  uint32_t rangeIndex = i % range;
  uint32_t colorIndex = rangeIndex / gradRange;
  uint32_t start = colorIndex;
  uint32_t end = colorIndex +1;
  if (curRange % 2 != 0) {
    start = (scheme.count -1) - start;
    end = (scheme.count -1) - end;
  }
  float dimmer = 100 / (float)Settings->light_dimmer;
  float fmyRed = (float)wsmap(rangeIndex % gradRange, 0, gradRange, scheme.colors[start].red, scheme.colors[end].red) / dimmer;
  float fmyGrn = (float)wsmap(rangeIndex % gradRange, 0, gradRange, scheme.colors[start].green, scheme.colors[end].green) / dimmer;
  float fmyBlu = (float)wsmap(rangeIndex % gradRange, 0, gradRange, scheme.colors[start].blue, scheme.colors[end].blue) / dimmer;
  mColor->red = (uint8_t)fmyRed;
  mColor->green = (uint8_t)fmyGrn;
  mColor->blue = (uint8_t)fmyBlu;
}

void Ws2812Gradient(uint32_t schemenr)
{
/*
 * This routine courtesy Tony DiCola (Adafruit)
 * Display a gradient of colors for the current color scheme.
 *  Repeat is the number of repetitions of the gradient (pick a multiple of 2 for smooth looping of the gradient).
 */
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif

  ColorScheme scheme = kSchemes[schemenr];
  if (scheme.count < 2) { return; }

  uint32_t repeat = kWsRepeat[Settings->light_width];  // number of scheme.count per ledcount
  uint32_t range = (uint32_t)ceil((float)Settings->light_pixels / (float)repeat);
  uint32_t gradRange = (uint32_t)ceil((float)range / (float)(scheme.count - 1));
  uint32_t speed = ((Settings->light_speed * 2) -1) * (STATES / 10);
  uint32_t offset = speed > 0 ? Light.strip_timer_counter / speed : 0;

  WsColor oldColor, currentColor;
  Ws2812GradientColor(schemenr, &oldColor, range, gradRange, offset);
  currentColor = oldColor;
  speed = speed ? speed : 1;    // should never happen, just avoid div0
  for (uint32_t i = 0; i < Settings->light_pixels; i++) {
    if (kWsRepeat[Settings->light_width] > 1) {
      Ws2812GradientColor(schemenr, &currentColor, range, gradRange, i + offset + 1);
    }
    // Blend old and current color based on time for smooth movement.
    c.R = wsmap(Light.strip_timer_counter % speed, 0, speed, oldColor.red, currentColor.red);
    c.G = wsmap(Light.strip_timer_counter % speed, 0, speed, oldColor.green, currentColor.green);
    c.B = wsmap(Light.strip_timer_counter % speed, 0, speed, oldColor.blue, currentColor.blue);
    strip->SetPixelColor(i, c);
    oldColor = currentColor;
  }
  Ws2812StripShow();
}

void Ws2812Bars(uint32_t schemenr)
{
/*
 * This routine courtesy Tony DiCola (Adafruit)
 * Display solid bars of color for the current color scheme.
 * Width is the width of each bar in pixels/lights.
 */
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif

  ColorScheme scheme = kSchemes[schemenr];

  uint32_t maxSize = Settings->light_pixels / scheme.count;
  if (kWidth[Settings->light_width] > maxSize) { maxSize = 0; }

  uint32_t speed = ((Settings->light_speed * 2) -1) * (STATES / 10);
  uint32_t offset = (speed > 0) ? Light.strip_timer_counter / speed : 0;

  WsColor mcolor[scheme.count];
  memcpy(mcolor, scheme.colors, sizeof(mcolor));
  float dimmer = 100 / (float)Settings->light_dimmer;
  for (uint32_t i = 0; i < scheme.count; i++) {
    float fmyRed = (float)mcolor[i].red / dimmer;
    float fmyGrn = (float)mcolor[i].green / dimmer;
    float fmyBlu = (float)mcolor[i].blue / dimmer;
    mcolor[i].red = (uint8_t)fmyRed;
    mcolor[i].green = (uint8_t)fmyGrn;
    mcolor[i].blue = (uint8_t)fmyBlu;
  }
  uint32_t colorIndex = offset % scheme.count;
  for (uint32_t i = 0; i < Settings->light_pixels; i++) {
    if (maxSize) { colorIndex = ((i + offset) % (scheme.count * kWidth[Settings->light_width])) / kWidth[Settings->light_width]; }
    c.R = mcolor[colorIndex].red;
    c.G = mcolor[colorIndex].green;
    c.B = mcolor[colorIndex].blue;
    strip->SetPixelColor(i, c);
  }
  Ws2812StripShow();
}

#ifdef USE_DERG_RGB
// note: when using Settings->dimmer, scale it: changeUIntScale(dimmer, 0, 100, 0, 255)
int64_t dragonOffset_current;
uint32_t dragonOffset_head;
uint32_t dragonOffset_head_remainder;
int64_t dragonOffset_sync;
unsigned long dragonLastMillis;

uint8_t dragonOverlay_type;
uint8_t dragonOverlay_status;
unsigned long dragonOverlayLastMillis;
unsigned long dragonOverlaySpeed;


void Ws2812Dragon(void)
{
  unsigned long now = millis();
  uint16_t diff = now - dragonLastMillis;
  dragonLastMillis = now;
  if (Settings->light_speed > 0) {
    dragonOffset_head_remainder += diff * 360;
    uint32_t divisor = 500 * Settings->light_speed * Settings->light_speed;
    while(dragonOffset_head_remainder >= divisor) {
      dragonOffset_head_remainder -= divisor;
      dragonOffset_head++;
    }
  }
  if (Light.power & 8) {
  dragonOffset_current = (Settings->light_speed > 0) ? (((u_int64_t) millis()) - dragonOffset_sync) * 360 / 500 / Settings->light_speed / Settings->light_speed : 0;
  } else {
  dragonOffset_current = dragonOffset_head;
  }

  uint8_t firstLed = 0;
  uint8_t lastLed = Settings->dragon_len1;
  Ws2812Dragon_Fx(firstLed, lastLed, Light.power & 1 ? Settings->dragon_fx1 : 0, Settings->light_color[0]);
  firstLed = lastLed; lastLed += Settings->dragon_len2;
  Ws2812Dragon_Fx(firstLed, lastLed, Light.power & 2 ? Settings->dragon_fx2 : 0, Settings->light_color[1]);
  firstLed = lastLed; lastLed += Settings->dragon_len3;
  Ws2812Dragon_Fx(firstLed, lastLed, Light.power & 2 ? Settings->dragon_fx3 : 0, Settings->light_color[1]);
  DragonFx_Misan();
  if (dragonOverlay_type > 0) {
    DragonFx_Overlay(Settings->dragon_len1, lastLed); // overlay only on segments 2&3
  }
  Ws2812StripShow();
}

/* dragonOverlay_status
 * 0: no overlay
 * 1: fade-in
 * 2,4,6,8: black
 * 3,5,7: selected color
 * 9: fade-out
 */
void DragonFx_Overlay(uint16_t firstLed, uint16_t lastLed) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif

  unsigned long now = millis();
  uint16_t diff = now - dragonOverlayLastMillis;
  if (diff > dragonOverlaySpeed) {
    dragonOverlay_status++;
    if (dragonOverlay_status > 9) {
      dragonOverlay_type = 0;
      return;
    }
    dragonOverlayLastMillis = now;
    diff = 0;
  }

  bool allBlack = dragonOverlay_status % 2 == 0;

  if (dragonOverlay_status == 1) {
    uint16_t midLed = (lastLed - firstLed) >> 1;
    firstLed = wsmap(diff, 0, dragonOverlaySpeed, midLed, 0);
    lastLed = wsmap(diff, 0, dragonOverlaySpeed, midLed, lastLed);
    allBlack = true;
  } else if (dragonOverlay_status == 9) {
    uint16_t midLed = (lastLed - firstLed) >> 1;
    firstLed = wsmap(diff, dragonOverlaySpeed, 0, midLed, 0);
    lastLed = wsmap(diff, dragonOverlaySpeed, 0, midLed, lastLed);
    allBlack = true;
  } 

  if (allBlack) {
    c.R = 0;
    c.G = 0;
    c.B = 0;
#if (USE_WS2812_CTYPE > NEO_3LED)
    c.W = 0;
#endif
  } else {
    uint8_t colmap[4] = { 0, 32, 128, 255 };
    c.R = colmap[(dragonOverlay_type & 0b00110000) >> 4];
    c.G = colmap[(dragonOverlay_type & 0b00001100) >> 2];
    c.B = colmap[(dragonOverlay_type & 0b00000011) >> 0];
#if (USE_WS2812_CTYPE > NEO_3LED)
    c.W = colmap[(dragonOverlay_type & 0b11000000) >> 6];
#endif
  }

  for(uint16_t idx = firstLed; idx < lastLed; idx++) {
    strip->SetPixelColor(idx, c);
  }
}

void Ws2812Dragon_Fx(uint16_t firstLed, uint16_t lastLed, uint8_t fx, uint8_t dimmer)
{
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  switch(fx) {
    case 0:
      c.R = 0; c.G = 0; c.B = 0;
      for (uint32_t i = firstLed; i < lastLed; i++) { strip->SetPixelColor(i, c); }
      break;
    case 15:
      dragonOffset_current = dragonOffset_head;
    case 1:
#if (USE_WS2812_CTYPE > NEO_3LED)
      c.R = (Light.power & 4) && Settings->light_color[2] < 128 ? changeUIntScale(128-Settings->light_color[2], 0, 127, 0, dimmer) : 0;
      c.G = 0;
      c.B = (Light.power & 4) && Settings->light_color[2] > 128 ? changeUIntScale(Settings->light_color[2]-128, 0, 127, 0, dimmer) : 0;
      c.W = changeUIntScale(255, 0, 255, 0, dimmer);
#else
      c.R = dimmer; c.G = c.R; c.B = c.R;
#endif
      for (uint32_t i = firstLed; i < lastLed; i++) { strip->SetPixelColor(i, c); }
      break;
    case 2:
      DragonFx_Rainbow(firstLed, lastLed, 0, dimmer);
      break;
    case 3:
      DragonFx_Rainbow(firstLed, lastLed, Settings->dragon_offset, dimmer);
      break;
    case 4:
      DragonFx_Rainbow(firstLed, lastLed, -Settings->dragon_offset, dimmer);
      break;
    case 5:
      DragonFx_Blink(firstLed, lastLed, dimmer);
      break;
    case 6:
      DragonFx_Colorlist(firstLed, lastLed, Settings->dragon_offset, dimmer);
      break;
    case 7:
      DragonFx_Colorlist(firstLed, lastLed, -Settings->dragon_offset, dimmer);
      break;
  }
}

void DragonFx_Rainbow(uint16_t firstLed, uint16_t lastLed, int16_t speed, uint8_t dimmer) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  for (uint32_t i = firstLed; i < lastLed; i++) {
    uint8_t r, g, b, sat;
    uint16_t hue;
    sat = 255;
    dragonOffset_current += speed;
    hue = dragonOffset_current / 10;
    HsToRgb(hue, sat, &r, &g, &b);
    c.R = changeUIntScale(r, 0, 255, 0, dimmer);
    c.G = changeUIntScale(g, 0, 255, 0, dimmer);
    c.B = changeUIntScale(b, 0, 255, 0, dimmer);
    strip->SetPixelColor(i, c);
  }
}

#define DRAGON_COLORLIST_NUM 9
struct colorPoint {
  uint8_t r, g, b;
  uint16_t len;
  int16_t dr, dg, db;
  uint16_t lensum;
} colorpoints[DRAGON_COLORLIST_NUM];
uint16_t colorPointTotalLen;

uint8_t dragon_flex_a = 5;
uint8_t dragon_flex_b = 170;
uint8_t dragon_flex_c = 50;
void DragonColorlistInit() {
  colorpoints[0].r =   0; colorpoints[0].g = 255; colorpoints[0].b =   0; colorpoints[0].len = 600;
  colorpoints[1].r =   0; colorpoints[1].g = 255; colorpoints[1].b = 255; colorpoints[1].len = 600;
  colorpoints[2].r =   0; colorpoints[2].g =   0; colorpoints[2].b = 255; colorpoints[2].len = 600;
  colorpoints[3].r = 255; colorpoints[3].g =   0; colorpoints[3].b = 255; colorpoints[3].len = 600 - dragon_flex_b;
  colorpoints[4].r = 255; colorpoints[4].g =   0; colorpoints[4].b =   dragon_flex_a; colorpoints[4].len = dragon_flex_b;
  colorpoints[5].r = 255; colorpoints[5].g =   0; colorpoints[5].b =   0; colorpoints[5].len = dragon_flex_c;
  colorpoints[6].r = 255; colorpoints[6].g =   0; colorpoints[6].b =   0; colorpoints[6].len = dragon_flex_b;
  colorpoints[7].r = 255; colorpoints[7].g =   dragon_flex_a; colorpoints[7].b =   0; colorpoints[7].len = 600 - dragon_flex_b;
  colorpoints[8].r = 255; colorpoints[8].g = 255; colorpoints[8].b =   0; colorpoints[8].len = 600;
  colorPointTotalLen = 0;
  for (uint8_t i = 0; i < DRAGON_COLORLIST_NUM; i++) {
    struct colorPoint &cp = colorpoints[i];
    struct colorPoint &ncp = colorpoints[(i+1) % DRAGON_COLORLIST_NUM];
    cp.lensum = colorPointTotalLen;
    colorPointTotalLen += cp.len;
    cp.dr = ncp.r - cp.r;
    cp.dg = ncp.g - cp.g;
    cp.db = ncp.b - cp.b;
  }
}

uint8_t map2(int8_t a, int32_t b, int32_t y, uint16_t z) {
	return ((uint8_t) (b * z / y)) + a;
}

void DragonFx_Colorlist(uint16_t firstLed, uint16_t lastLed, int16_t speed, uint8_t dimmer) {
  if (!colorPointTotalLen) DragonColorlistInit();
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  uint8_t cci = 0;
  for (uint16_t i = firstLed; i < lastLed; i++) {
    dragonOffset_current += speed;
    while (dragonOffset_current >= colorPointTotalLen) { dragonOffset_current -= colorPointTotalLen; }
    while (dragonOffset_current < 0) { dragonOffset_current += colorPointTotalLen; }
    while (dragonOffset_current < colorpoints[cci].lensum) { cci--; }
    while (dragonOffset_current >= colorpoints[cci].lensum + colorpoints[cci].len) { cci++; }
    uint16_t pos = dragonOffset_current - colorpoints[cci].lensum;
    struct colorPoint &cp = colorpoints[cci];

    uint8_t r, g, b;

    r = map2(cp.r, cp.dr, cp.len, pos);
    g = map2(cp.g, cp.dg, cp.len, pos);
    b = map2(cp.b, cp.db, cp.len, pos);
    c.R = changeUIntScale(r, 0, 255, 0, dimmer);
    c.G = changeUIntScale(g, 0, 255, 0, dimmer);
    c.B = changeUIntScale(b, 0, 255, 0, dimmer);
    strip->SetPixelColor(i, c);
  }
}

#define MISAN_SIZE 500
uint8_t misan[MISAN_SIZE];

#pragma pack(push, 1)
struct drgn_misan_segment {
  /* Effect for this segment
   * 0 - END MARKER
   * 1 - black / off
   * 2 - linear blend, linear segment
   * 3 - linear blend, circular segment
   * 4 - copy
   * 5 - static single color
   * 6 - white-balanced white (parameter: 0=adds blue, 50=only white, 100=adds red)
   */
  uint8_t type;
  uint16_t start;
  uint16_t length;
  struct {
    unsigned int power_selector: 3; // 0=always on, otherwise see value of PowerX. If OFF, then no colors are set/changed
    unsigned int dimm_channel: 2; // use ChannelX for brightness control (0=full brightness)
    unsigned int disabled: 1; // if effect is disabled, no colors are set/changed
    unsigned int blackout: 1; // if effect is off (based on power_selector), 0=no colors are set/changed 1=everything set to black
    unsigned int unused: 1;
  };
  union {
    struct {
      uint8_t r, g, b;
    } singlecolor;
    struct {
      int16_t speed;
      int8_t px_offset;
      uint8_t px_dimm;
      uint16_t color_table_idx;
      uint16_t color_offset;
      int16_t circle_speed; // only for fx=2
      int8_t circle_offset; // only for fx=2
    } linear;
    struct {
      /* handling of different source/target lengths
       * 0 - wrap around
       * 1 - mirror
       * 2 - stretch
       */
      uint8_t boundary_mode;
      /** copy source index */
      uint16_t start;
      uint16_t length;
      /** Copy modification type
       * 0 - none, copy exactly (but respect dimm_channel)
       * 1 - dim, (0..255 -> 0..param)
       * 2 - brighten (0..param -> 0..255)
       * 3 - use dimm_channel to darken or brighten (128 = unchanged)
       */
      uint8_t type;
      uint16_t param;
    } copy;
    struct {
      uint8_t active_selector; // 0=active, >0 depending on PowerX
      uint8_t balance_channel;
    } white;
  };
};
struct drgn_misan_blendentry {
  uint8_t r, g, b;
  uint16_t len;
  uint16_t lensum; // sum of all len of previous entries
};
struct drgn_misan_blendtable {
  uint8_t size; // number of entries
  uint16_t totalLen; // sum of all len of all entries
  struct drgn_misan_blendentry entry[0]; // actual size is dynamic
};
#pragma pack(pop)
uint8_t drgn_misan_segSizeByType[] = { 1, 6, 14, 17, 14, 9, 8 };

// returns the lengths of the segment definition based on the segment effect. Returns MISAN_SIZE (as "too big value") for unknown types
uint16_t DragonFx_Misan_getSegmentSizeByType(uint8_t type) {
  return type < sizeof(drgn_misan_segSizeByType) ? drgn_misan_segSizeByType[type] : MISAN_SIZE;
}

void DragonFx_Misan() {
  uint16_t misanIdx = 0;
  do {
    if (misanIdx >= MISAN_SIZE) return; // emergency brake
    struct drgn_misan_segment *seg = reinterpret_cast<struct drgn_misan_segment*> (&misan[misanIdx]);
    if (seg->type == 0) break;
    misanIdx += DragonFx_Misan_getSegmentSizeByType(seg->type);
    if (seg->disabled) continue; // effect disabled, skip to next one
    uint8_t type = seg->type;
    uint8_t pwrselect = seg->power_selector;
    if (pwrselect && (((Light.power >> (pwrselect-1)) & 1) == 0)) { type = seg->blackout ? 1 : -1; } // segment activation based on PowerX
    switch(type) {
      case 0:
        // END MARKER, should not reach here
        break;
      case 1:
        DragonFx_Misan_static(seg, 0, 0, 0);
        break;
      case 2:
        DragonFx_Misan_linear(seg, false);
        break;
      case 3:
        DragonFx_Misan_linear(seg, true);
        break;
      case 4:
        DragonFx_Misan_copy(seg);
        break;
      case 5:
        DragonFx_Misan_static(seg, seg->singlecolor.r, seg->singlecolor.g, seg->singlecolor.b);
        break;
      case 6: 
        DragonFx_Misan_white(seg);
        break;
      default:
        // unknown fx -> no change
        break;
    }
  } while(true);
}

unsigned long florp_millis = 0;
void DragonFx_Misan_linear(struct drgn_misan_segment *seg, bool circular) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  // TODO use a segment-local florp_millis
  unsigned long now = millis() - florp_millis;
/*
  uint16_t diff = now - florp_millis;
  if (seg->speed > 0 && diff >= seg->speed) {
    florp_millis = now - diff % seg->speed;
    seg->color_offset += diff / seg->speed;
  } else if (seg->speed < 0 && diff >= -seg->speed) { // use "negative seg->speed" everywhere, otherwise signed int casts would be necessary
    florp_millis = now - diff % -seg->speed;
    seg->color_offset -= diff / -seg->speed;
  }
*/
  struct drgn_misan_blendtable *table = reinterpret_cast<struct drgn_misan_blendtable*> (&misan[seg->linear.color_table_idx]);

  int32_t blend_index_current = seg->linear.color_offset + (seg->linear.speed == 0 ? 0 : seg->linear.speed > 0 ? now / seg->linear.speed : table->totalLen - 1 - ((now / (-seg->linear.speed)) % table->totalLen));
  int32_t circle_offset_current = 0;
  if (circular) 
    circle_offset_current = seg->linear.circle_offset + (seg->linear.circle_speed == 0 ? 0 : seg->linear.circle_speed > 0 ? now / seg->linear.circle_speed : seg->length - 1 - ((now / (-seg->linear.circle_speed)) % seg->length));

  uint16_t dst_start = seg->start;
  uint8_t cci = 0;
  uint8_t cached_cci = -1; // cache starts invalid
  uint16_t len;
  uint8_t r, g, b;
  int16_t dr, dg, db;
  uint16_t px_count = circular && seg->length > 0 ? (seg->length >> 1) + 1 : seg->length;
  uint8_t pxdimm = seg->dimm_channel > 0 ? Settings->light_color[seg->dimm_channel-1] : 255;
  for (uint16_t i = 0; i < px_count; i++) {
    // limit global offset to total range of color-blend table
    if (blend_index_current >= table->totalLen) { blend_index_current %= table->totalLen; }
    if (blend_index_current < 0) { blend_index_current = table->totalLen - 1 - ((blend_index_current) % table->totalLen); }

    // calculate current color index (cci) in the color-blend table
    // since the offset is already capped, no over/underflow can occur
    while (blend_index_current < table->entry[cci].lensum) { cci--; }
    while (blend_index_current >= table->entry[cci].lensum + table->entry[cci].len) { cci++; }

    // relative position inside the current color-blend table entry
    uint16_t pos = blend_index_current - table->entry[cci].lensum;

    // cache some values
    if (cached_cci != cci) { // cache still valid?
      cached_cci = cci;
      struct drgn_misan_blendentry entry = table->entry[cci];
      struct drgn_misan_blendentry next_entry = table->entry[(cci+1) % table->size];
      len = entry.len;
      r = entry.r;
      g = entry.g;
      b = entry.b;
      dr = next_entry.r - r;
      dg = next_entry.g - g;
      db = next_entry.b - b;
    }

    // calculate color
    c.R = map2(r, dr, len, pos);
    c.G = map2(g, dg, len, pos);
    c.B = map2(b, db, len, pos);
    if (pxdimm < 255) {    
      c.R = changeUIntScale(c.R, 0, 255, 0, pxdimm);
      c.G = changeUIntScale(c.G, 0, 255, 0, pxdimm);
      c.B = changeUIntScale(c.B, 0, 255, 0, pxdimm);
    }

    // set color
    uint16_t dst_idx = dst_start + ((circle_offset_current + i) % seg->length);
    if (dst_idx < Settings->light_pixels) {
      strip->SetPixelColor(dst_idx, c);
    }

    if (circular) {
      uint16_t dst_idx2 = dst_start + ((seg->length + circle_offset_current - i) % seg->length);
      if (dst_idx2 != dst_idx && dst_idx2 < Settings->light_pixels) {
        strip->SetPixelColor(dst_idx2, c);
      }
    }

    // calculate per-led (pixel) offset inside this segment for next loop iteration
    blend_index_current = (table->totalLen + blend_index_current + (seg->linear.px_offset % table->totalLen)) % table->totalLen;
    if (seg->linear.px_dimm != 0x80) pxdimm = (pxdimm * seg->linear.px_dimm) >> 7;
  }
}

void DragonFx_Misan_copy(struct drgn_misan_segment *seg) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  uint16_t src_start = seg->copy.start;
  uint16_t dst_start = seg->start;
  uint8_t dimmer = seg->dimm_channel > 0 ? Settings->light_color[seg->dimm_channel-1] : 255;
  // TODO: handle overlapping areas
  uint16_t segLen = abs(seg->length); // TODO can't currently happen, as seg->length is unsigned
  for (uint16_t idx = 0; idx < segLen; idx++) {
    uint16_t dst_idx = seg->length >= 0 ? dst_start + idx : dst_start - seg->length - 1 - idx;
    if (dst_idx >= Settings->light_pixels) continue;
    uint16_t src_idx;
    switch(seg->copy.boundary_mode) {
      case 1: { // mirror
        uint16_t j = idx % ((seg->copy.length-1) << 1);
        if (j >= seg->copy.length) j = 2 * (seg->copy.length-1) - j;
        src_idx = src_start + j;
        break; }
      case 2: // stretch
        src_idx = src_start + changeUIntScale(idx, 0, segLen-1, 0, seg->copy.length-1);
        break;
      case 10: // reverse wrap
        src_idx = src_start + seg->copy.length-1 - (idx % seg->copy.length);
        break;
      case 12: // reverse stretch
        src_idx = src_start + changeUIntScale(idx, 0, segLen-1, seg->copy.length-1, 0);
        break;
      default: // wrap
        src_idx = src_start + (idx % seg->copy.length);
    }
    if (src_idx < Settings->light_pixels) {
      c = strip->GetPixelColor(src_idx);
      if (dimmer < 255) {    
        c.R = changeUIntScale(c.R, 0, 255, 0, dimmer);
        c.G = changeUIntScale(c.G, 0, 255, 0, dimmer);
        c.B = changeUIntScale(c.B, 0, 255, 0, dimmer);
      }
    } else {
      c.R = 0;
      c.G = 0;
      c.B = 0;
    }
    switch(seg->copy.type) {
      case 1:
        c.R = changeUIntScale(c.R, 0, 255, 0, seg->copy.param);
        c.G = changeUIntScale(c.G, 0, 255, 0, seg->copy.param);
        c.B = changeUIntScale(c.B, 0, 255, 0, seg->copy.param);
        break;
      case 2:
        c.R = changeUIntScale(c.R, 0, seg->copy.param, 0, 255);
        c.G = changeUIntScale(c.G, 0, seg->copy.param, 0, 255);
        c.B = changeUIntScale(c.B, 0, seg->copy.param, 0, 255);
        break;
      case 3:
        if (seg->dimm_channel > 0)
          if (dimmer > 128) {
            uint8_t d = dimmer & 0x7F;
            uint8_t rd = 255 - d;
            c.R = c.R > rd ? 255 : c.R + d;
            c.G = c.G > rd ? 255 : c.G + d;
            c.B = c.B > rd ? 255 : c.B + d;
          } if (dimmer < 128) {
            c.R = c.R < dimmer ? 0 : c.R - dimmer & 0x7F;
            c.G = c.G < dimmer ? 0 : c.G - dimmer & 0x7F;
            c.B = c.B < dimmer ? 0 : c.B - dimmer & 0x7F;
        }
        break;
    }
    strip->SetPixelColor(dst_idx, c);
  }
}

void DragonFx_Misan_static(struct drgn_misan_segment *seg, uint8_t r, uint8_t g, uint8_t b) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  uint8_t dimmer = seg->dimm_channel > 0 ? Settings->light_color[seg->dimm_channel-1] : 255;
  if (dimmer < 255) {
    c.R = changeUIntScale(r, 0, 255, 0, dimmer);
    c.G = changeUIntScale(g, 0, 255, 0, dimmer);
    c.B = changeUIntScale(b, 0, 255, 0, dimmer);
  } else {
    c.R = r;
    c.G = g;
    c.B = b;
  }
  uint16_t dst_start = seg->start;
  for (uint16_t i = 0; i < seg->length; i++) {
    uint16_t dst_idx = dst_start + i;
    if (dst_idx >= Settings->light_pixels) break;
    strip->SetPixelColor(dst_idx, c);
  }
}

void DragonFx_Misan_white(struct drgn_misan_segment *seg) {
  uint8_t pwrselect = seg->white.active_selector;
  bool balance_active = (pwrselect == 0) || (((Light.power >> (pwrselect-1)) & 1) == 1);
  uint8_t balance = seg->white.balance_channel > 0 ? Settings->light_color[seg->white.balance_channel-1] : 128;
  uint8_t dimmer = seg->dimm_channel > 0 ? Settings->light_color[seg->dimm_channel-1] : 255;
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.R = balance_active && balance < 128 ? changeUIntScale(128-balance, 0, 127, 0, dimmer) : 0;
  c.G = 0;
  c.B = balance_active && balance > 128 ? changeUIntScale(balance-128, 0, 127, 0, dimmer) : 0;
  c.W = dimmer;
#else
  RgbColor c;
  // balanced white currently not supported without white LED
  c.R = dimmer; c.G = dimmer; c.B = dimmer;
#endif

  uint16_t dst_start = seg->start;
  for (uint16_t i = 0; i < seg->length; i++) {
    uint16_t dst_idx = dst_start + i;
    if (dst_idx >= Settings->light_pixels) break;
    strip->SetPixelColor(dst_idx, c);
  }
}

uint8_t misan_active_template = 0;
void SetMisanTemplate(uint8_t mode) {
  switch (mode) {
    case 0:
    default:
      // no (valid) template
      break;
    case 1: { // low glow green
      uint8_t new_misan[] = { 0x03, 0x00, 0x00, 0x20, 0x00, 0x00, 0x0a, 0x00, 0x0a, 0x80, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x58, 0x02, 0x00, 0x40, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x64, 0x00, 0x64, 0x00, 0x00, 0x20, 0x00, 0x64, 0x00, 0xc8, 0x00, 0x00, 0x60, 0x00, 0x64, 0x00, 0x2c, 0x01, 0x00, 0x80, 0x00, 0x64, 0x00, 0x90, 0x01, 0x00, 0x60, 0x00, 0x64, 0x00, 0xf4, 0x01 };
      std::copy(new_misan, new_misan+sizeof(new_misan)/sizeof(new_misan[0]), misan);
      break;
    }
    case 2: { // low glow orange
      uint8_t new_misan[] = { 0x03, 0x00, 0x00, 0x20, 0x00, 0x00, 0x0a, 0x00, 0x0a, 0x80, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x58, 0x02, 0x40, 0x02, 0x00, 0x64, 0x00, 0x00, 0x00, 0x80, 0x04, 0x00, 0x64, 0x00, 0x64, 0x00, 0x20, 0x01, 0x00, 0x64, 0x00, 0xc8, 0x00, 0x60, 0x03, 0x00, 0x64, 0x00, 0x2c, 0x01, 0x80, 0x04, 0x00, 0x64, 0x00, 0x90, 0x01, 0x60, 0x03, 0x00, 0x64, 0x00, 0xf4, 0x01 };
      std::copy(new_misan, new_misan+sizeof(new_misan)/sizeof(new_misan[0]), misan);
      break;
    }
    case 3: { // Dancer pattern (fast)
      uint8_t new_misan[] = { 0x03, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0a, 0x00, 0x14, 0x80, 0xbb, 0x00, 0x00, 0x00, 0xfa, 0x00, 0x04, 0x04, 0x0c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x03, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x0a, 0x00, 0x14, 0x80, 0xbb, 0x00, 0xc8, 0x00, 0xfa, 0x00, 0x05, 0x04, 0x1c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x10, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x03, 0x20, 0x00, 0x0c, 0x00, 0x00, 0x0a, 0x00, 0x14, 0x80, 0xbb, 0x00, 0x90, 0x01, 0xfa, 0x00, 0x03, 0x04, 0x2c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x20, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x03, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x0a, 0x00, 0x14, 0x80, 0xbb, 0x00, 0x58, 0x02, 0xfa, 0x00, 0x0a, 0x04, 0x3c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x03, 0x40, 0x00, 0x0c, 0x00, 0x00, 0x0a, 0x00, 0x14, 0x80, 0xbb, 0x00, 0x20, 0x03, 0xfa, 0x00, 0x09, 0x04, 0x4c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x40, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x03, 0x50, 0x00, 0x0c, 0x00, 0x00, 0x0a, 0x00, 0x14, 0x80, 0xbb, 0x00, 0xe8, 0x03, 0xfa, 0x00, 0x0a, 0x04, 0x5c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x50, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x00, 0x06, 0xb0, 0x04, 0xff, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xc8, 0x00, 0xc8, 0x00, 0x00, 0xff, 0x00, 0xc8, 0x00, 0x90, 0x01, 0x00, 0xff, 0xff, 0xc8, 0x00, 0x58, 0x02, 0x00, 0x00, 0xff, 0xc8, 0x00, 0x20, 0x03, 0xff, 0x00, 0xff, 0xc8, 0x00, 0xe8, 0x03 };
      std::copy(new_misan, new_misan+sizeof(new_misan)/sizeof(new_misan[0]), misan);
      break;
    }
    case 4: { // Dancer pattern (variant2)
      uint8_t new_misan[] = { 0x03, 0x00, 0x00, 0x0c, 0x00, 0x09, 0x64, 0x00, 0x00, 0x74, 0xbb, 0x00, 0x00, 0x00, 0xfa, 0x00, 0x04, 0x04, 0x0c, 0x00, 0x04, 0x00, 0x12, 0x0c, 0x00, 0x00, 0x0c, 0x00, 0x03, 0x80, 0x00, 0x03, 0x10, 0x00, 0x0c, 0x00, 0x09, 0x64, 0x00, 0x00, 0x74, 0xbb, 0x00, 0xc8, 0x00, 0xfa, 0x00, 0x05, 0x04, 0x1c, 0x00, 0x04, 0x00, 0x12, 0x0c, 0x10, 0x00, 0x0c, 0x00, 0x03, 0x80, 0x00, 0x03, 0x20, 0x00, 0x0c, 0x00, 0x09, 0x64, 0x00, 0x00, 0x74, 0xbb, 0x00, 0x90, 0x01, 0xfa, 0x00, 0x03, 0x04, 0x2c, 0x00, 0x04, 0x00, 0x12, 0x0c, 0x20, 0x00, 0x0c, 0x00, 0x03, 0x80, 0x00, 0x03, 0x30, 0x00, 0x0c, 0x00, 0x09, 0x64, 0x00, 0x00, 0x74, 0xbb, 0x00, 0x58, 0x02, 0xfa, 0x00, 0x0a, 0x04, 0x3c, 0x00, 0x04, 0x00, 0x12, 0x0c, 0x30, 0x00, 0x0c, 0x00, 0x03, 0x80, 0x00, 0x03, 0x40, 0x00, 0x0c, 0x00, 0x09, 0x64, 0x00, 0x00, 0x74, 0xbb, 0x00, 0x20, 0x03, 0xfa, 0x00, 0x09, 0x04, 0x4c, 0x00, 0x04, 0x00, 0x12, 0x0c, 0x40, 0x00, 0x0c, 0x00, 0x03, 0x80, 0x00, 0x03, 0x50, 0x00, 0x0c, 0x00, 0x09, 0x64, 0x00, 0x00, 0x74, 0xbb, 0x00, 0xe8, 0x03, 0xfa, 0x00, 0x0a, 0x04, 0x5c, 0x00, 0x04, 0x00, 0x12, 0x0c, 0x50, 0x00, 0x0c, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0c, 0xb0, 0x04, 0xff, 0x00, 0x00, 0x96, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x32, 0x00, 0x96, 0x00, 0xff, 0xff, 0x00, 0x96, 0x00, 0xc8, 0x00, 0xff, 0xff, 0x00, 0x32, 0x00, 0x5e, 0x01, 0x00, 0xff, 0x00, 0x96, 0x00, 0x90, 0x01, 0x00, 0xff, 0x00, 0x32, 0x00, 0x26, 0x02, 0x00, 0xff, 0xff, 0x96, 0x00, 0x58, 0x02, 0x00, 0xff, 0xff, 0x32, 0x00, 0xee, 0x02, 0x00, 0x00, 0xff, 0x96, 0x00, 0x20, 0x03, 0x00, 0x00, 0xff, 0x32, 0x00, 0xb6, 0x03, 0xff, 0x00, 0xff, 0x96, 0x00, 0xe8, 0x03, 0xff, 0x00, 0xff, 0x32, 0x00, 0x7e, 0x04 };
      std::copy(new_misan, new_misan+sizeof(new_misan)/sizeof(new_misan[0]), misan);
      break;
    }
    case 5: { // Dancer pattern (test)
      uint8_t new_misan[] = { 0x02, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x80, 0xa9, 0x00, 0x00, 0x00, 0x04, 0x0c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x02, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x80, 0xa9, 0x00, 0x78, 0x00, 0x04, 0x1c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x10, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x02, 0x20, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x80, 0xa9, 0x00, 0xf0, 0x00, 0x04, 0x2c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x20, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x02, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x80, 0xa9, 0x00, 0x68, 0x01, 0x04, 0x3c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x02, 0x40, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x80, 0xa9, 0x00, 0xe0, 0x01, 0x04, 0x4c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x40, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x02, 0x50, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x80, 0xa9, 0x00, 0x58, 0x02, 0x04, 0x5c, 0x00, 0x04, 0x00, 0x00, 0x0c, 0x50, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x00, 0x0c, 0x68, 0x01, 0xff, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0xff, 0x00, 0x0a, 0x00, 0x78, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xff, 0x0a, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x01 };
      std::copy(new_misan, new_misan+sizeof(new_misan)/sizeof(new_misan[0]), misan);
      break;
    }
    case 6: { // Bad Strip
      uint8_t new_misan[] = { 0x06, 0x00, 0x00, 0x70, 0x00, 0x49, 0x03, 0x03, 0x02, 0x70, 0x00, 0x49, 0x00, 0x52, 0x2b, 0x02, 0x05, 0x80, 0x2e, 0x00, 0x00, 0x00, 0x02, 0xb9, 0x00, 0x3d, 0x00, 0x52, 0x2b, 0x02, 0xfb, 0x80, 0x2e, 0x00, 0x6d, 0x01, 0x05, 0x37, 0x00, 0x02, 0x00, 0x14, 0xff, 0x80, 0x00, 0x00, 0x09, 0x42, 0x0e, 0x00, 0xff, 0x00, 0x58, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0x58, 0x02, 0x58, 0x02, 0x00, 0x00, 0xff, 0x58, 0x02, 0xb0, 0x04, 0xff, 0x00, 0xff, 0xae, 0x01, 0x08, 0x07, 0xff, 0x00, 0x05, 0xaa, 0x00, 0xb6, 0x08, 0xff, 0x00, 0x00, 0x32, 0x00, 0x60, 0x09, 0xff, 0x00, 0x00, 0xaa, 0x00, 0x92, 0x09, 0xff, 0x05, 0x00, 0xae, 0x01, 0x3c, 0x0a, 0xff, 0xff, 0x00, 0x58, 0x02, 0xea, 0x0b };
      std::copy(new_misan, new_misan+sizeof(new_misan)/sizeof(new_misan[0]), misan);
      break;
    }
  }
  misan_active_template = mode;
}


// TODO was used for debugging / returning calculated values for a specific offset
struct RgbwColor DragonFx_Test(int64_t offset, uint8_t dimmer) {
  RgbwColor c;
  c.W = 0;
  uint8_t cci = 0;
//    offset += speed;
    while (offset >= colorPointTotalLen) { offset -= colorPointTotalLen; }
    while (offset < 0) { offset += colorPointTotalLen; }
    while (offset < colorpoints[cci].lensum) { cci--; }
    while (offset >= colorpoints[cci].lensum + colorpoints[cci].len) { cci++; }
    uint16_t pos = offset - colorpoints[cci].lensum;
    struct colorPoint &cp = colorpoints[cci];

    uint8_t r, g, b;

    r = map2(cp.r, cp.dr, cp.len, pos);
    g = map2(cp.g, cp.dg, cp.len, pos);
    b = map2(cp.b, cp.db, cp.len, pos);
    c.R = changeUIntScale(r, 0, 255, 0, dimmer);
    c.G = changeUIntScale(g, 0, 255, 0, dimmer);
    c.B = changeUIntScale(b, 0, 255, 0, dimmer);
    return c;
}

void DragonFx_Blink(uint16_t firstLed, uint16_t lastLed, uint8_t dimmer) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  uint32_t now = millis();
  uint8_t i = Settings->light_speed;
  while (i) {
    now /= 10;
    i--;
  }
  c.G = (now & 1) ? 255 : 0;
  c.R = changeUIntScale(c.R, 0, 255, 0, dimmer);
  c.G = changeUIntScale(c.G, 0, 255, 0, dimmer);
  c.B = changeUIntScale(c.B, 0, 255, 0, dimmer);
  for (uint32_t i = firstLed; i < lastLed; i++) {
      strip->SetPixelColor(i, c);
  }
}

#endif // USE_DERG_RGB

void Ws2812Clear(void)
{
  strip->ClearTo(0);
  strip->Show();
  Ws2812.show_next = 1;
}

void Ws2812SetColor(uint32_t led, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor lcolor;
  lcolor.W = white;
#else
  RgbColor lcolor;
#endif

  lcolor.R = red;
  lcolor.G = green;
  lcolor.B = blue;
  if (led) {
    strip->SetPixelColor(led -1, lcolor);  // Led 1 is strip Led 0 -> substract offset 1
  } else {
//    strip->ClearTo(lcolor);  // Set WS2812_MAX_LEDS pixels
    for (uint32_t i = 0; i < Settings->light_pixels; i++) {
      strip->SetPixelColor(i, lcolor);
    }
  }

  if (!Ws2812.suspend_update) {
    strip->Show();
    Ws2812.show_next = 1;
  }
}

char* Ws2812GetColor(uint32_t led, char* scolor)
{
  uint8_t sl_ledcolor[4];

 #if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor lcolor = strip->GetPixelColor(led -1);
  sl_ledcolor[3] = lcolor.W;
 #else
  RgbColor lcolor = strip->GetPixelColor(led -1);
 #endif
  sl_ledcolor[0] = lcolor.R;
  sl_ledcolor[1] = lcolor.G;
  sl_ledcolor[2] = lcolor.B;
  scolor[0] = '\0';
  for (uint32_t i = 0; i < Light.subtype; i++) {
    if (Settings->flag.decimal_text) {  // SetOption17 - Switch between decimal or hexadecimal output (0 = hexadecimal, 1 = decimal)
      snprintf_P(scolor, 25, PSTR("%s%s%d"), scolor, (i > 0) ? "," : "", sl_ledcolor[i]);
    } else {
      snprintf_P(scolor, 25, PSTR("%s%02X"), scolor, sl_ledcolor[i]);
    }
  }
  return scolor;
}

/*********************************************************************************************\
 * Public - used by scripter only
\*********************************************************************************************/

void Ws2812ForceSuspend (void)
{
  Ws2812.suspend_update = true;
}

void Ws2812ForceUpdate (void)
{
  Ws2812.suspend_update = false;
  strip->Show();
  Ws2812.show_next = 1;
}

/********************************************************************************************/

bool Ws2812SetChannels(void)
{
  uint8_t *cur_col = (uint8_t*)XdrvMailbox.data;

  Ws2812SetColor(0, cur_col[0], cur_col[1], cur_col[2], cur_col[3]);

  return true;
}

void Ws2812ShowScheme(void)
{
  uint32_t scheme = Settings->light_scheme - Ws2812.scheme_offset;

  switch (scheme) {
    case 0:  // Clock
      if ((1 == TasmotaGlobal.state_250mS) || (Ws2812.show_next)) {
        Ws2812Clock();
        Ws2812.show_next = 0;
      }
      break;
#ifdef USE_DERG_RGB
    case 8: // Dragon
      Ws2812Dragon();
      Ws2812.show_next = 1;
      break;
#endif // USE_DERG_RGB
    default:
      if (1 == Settings->light_fade) {
        Ws2812Gradient(scheme -1);
      } else {
        Ws2812Bars(scheme -1);
      }
      Ws2812.show_next = 1;
      break;
  }
}

void Ws2812ModuleSelected(void)
{
#if (USE_WS2812_HARDWARE == NEO_HW_P9813)
  if (PinUsed(GPIO_P9813_CLK) && PinUsed(GPIO_P9813_DAT)) {  // RGB led
    strip = new NeoPixelBus<selectedNeoFeatureType, selectedNeoSpeedType>(WS2812_MAX_LEDS, Pin(GPIO_P9813_CLK), Pin(GPIO_P9813_DAT));
#else
  if (PinUsed(GPIO_WS2812)) {  // RGB led
    // For DMA, the Pin is ignored as it uses GPIO3 due to DMA hardware use.
    strip = new NeoPixelBus<selectedNeoFeatureType, selectedNeoSpeedType>(WS2812_MAX_LEDS, Pin(GPIO_WS2812));
#endif  // NEO_HW_P9813
    strip->Begin();

    Ws2812Clear();

    Ws2812.scheme_offset = Light.max_scheme +1;
    Light.max_scheme += WS2812_SCHEMES;

#if (USE_WS2812_CTYPE > NEO_3LED)
    TasmotaGlobal.light_type = LT_RGBW;
#else
    TasmotaGlobal.light_type = LT_RGB;
#endif
    TasmotaGlobal.light_driver = XLGT_01;
  }
}

/********************************************************************************************/

void CmndLed(void)
{
  if ((XdrvMailbox.index > 0) && (XdrvMailbox.index <= Settings->light_pixels)) {
    if (XdrvMailbox.data_len > 0) {
      char *p;
      uint16_t idx = XdrvMailbox.index;
      Ws2812ForceSuspend();
      for (char *color = strtok_r(XdrvMailbox.data, " ", &p); color; color = strtok_r(nullptr, " ", &p)) {
        if (LightColorEntry(color, strlen(color))) {
          Ws2812SetColor(idx, Light.entry_color[0], Light.entry_color[1], Light.entry_color[2], Light.entry_color[3]);
          idx++;
          if (idx > Settings->light_pixels) { break; }
        } else {
          break;
        }
      }
      Ws2812ForceUpdate();
    }
    char scolor[LIGHT_COLOR_SIZE];
    ResponseCmndIdxChar(Ws2812GetColor(XdrvMailbox.index, scolor));
  }
}

void CmndPixels(void)
{
  if ((XdrvMailbox.payload > 0) && (XdrvMailbox.payload <= WS2812_MAX_LEDS)) {
    Settings->light_pixels = XdrvMailbox.payload;
    Settings->light_rotation = 0;
    Ws2812Clear();
    Light.update = true;
  }
  ResponseCmndNumber(Settings->light_pixels);
}

void CmndRotation(void)
{
  if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload < Settings->light_pixels)) {
    Settings->light_rotation = XdrvMailbox.payload;
  }
  ResponseCmndNumber(Settings->light_rotation);
}

void CmndWidth(void)
{
  if ((XdrvMailbox.index > 0) && (XdrvMailbox.index <= 4)) {
    if (1 == XdrvMailbox.index) {
      if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload <= 4)) {
        Settings->light_width = XdrvMailbox.payload;
      }
      ResponseCmndNumber(Settings->light_width);
    } else {
      if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload < 32)) {
        Settings->ws_width[XdrvMailbox.index -2] = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->ws_width[XdrvMailbox.index -2]);
    }
  }
}

/* Dragon Command: index vs payload
 * > Index is 1 by default (not given or out of range)
 * 1 - set hue-offset-per-led in rainbow mode. Use 'speed' for cycle speed (1-40)
 * 2 - length (LEDs) of first segment
 * 3 - length (LEDs) of second segment
 * 4 - length (LEDs) of third segment
 * 5 - effect for segment 1
 * 6 - effect for segment 2
 * 7 - effect for segment 3
 * 8 - sync offset
 * 9 - removed
 * 10-13 - debug stuff
 * 14 - Dragon Overlay, color:
 *      0 - off
 *     48 - red
 *     12 - green
 *      3 - blue
 *     60 - yellow
 *     64 - white
 * 15 - write byte/word value to MISAN.
 *      for value as 0xAAAABBCC
 *      AA: 0..MISAN_SIZE write 8 bit value CC to address AA
 *      AA: >8192 write 16 bit value, CC to address AA, BB to address AA+1
 * 16 - read byte/word value from MISAN, value = 0xAAAA0000 to specify address and 8/16 bit
 * 17 - read specific MISAN cooked values (for debug)
 * 18 - MISAN template
 *    1:
 * 
 * Tasmota Mailbox magic value for "no (numeric) parameter given" is -99
 * 
 * Effects
 * 0 - all black (off)
 * 1 - all white (with 'color temp')
 * 15 - like 1, but rainbow will use the 'head' offset calculation
 * 2 - rainbow (all same color)
 * 3 - rainbow (use Dragon1 <x> to give hue offset per pixel)
 * 4 - rainbow (like 4, but reverse direction)
 * 5 - blink
 * 6 - Colorlist
 * 7 - Colorlist, reverse direction
 * 8 - Effect 'Misan'
 * 
 * TODO
 * - replace "three segments" and "all effects" with new MISAN code
 * - replace dragon2/3/4 segment lengths with Settings->light_pixels / command "Pixels"
 * - ensure "colorlist" can be fully replaced by MISAN (especially: up->down count change)
 * - enhance MISAN to react to POWERx, DIMMERx in segment config
 * - test/verify MISAN Base64 write, and add paginated Base64 read
 */
#ifdef USE_DERG_RGB
void CmndDragon(void)
{
  switch(XdrvMailbox.index) {
    case 0: // set hue-offset-per-led in rainbow mode
    case 1:
      if (-99 != XdrvMailbox.payload) {
        Settings->dragon_offset = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->dragon_offset);
      break;
    case 2:
      if (-99 != XdrvMailbox.payload) {
        Settings->dragon_len1 = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->dragon_len1);
      strip->ClearTo(0);
      break;
    case 3:
      if (-99 != XdrvMailbox.payload) {
        Settings->dragon_len2 = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->dragon_len2);
      strip->ClearTo(0);
      break;
    case 4:
      if (-99 != XdrvMailbox.payload) {
        Settings->dragon_len3 = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->dragon_len3);
      strip->ClearTo(0);
      break;
    case 5:
      if (-99 != XdrvMailbox.payload) {
        Settings->dragon_fx1 = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->dragon_fx1);
      break;
    case 6:
      if (-99 != XdrvMailbox.payload) {
        Settings->dragon_fx2 = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->dragon_fx2);
      break;
    case 7:
      if (-99 != XdrvMailbox.payload) {
        Settings->dragon_fx3 = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->dragon_fx3);
      break;
    case 8:
      if (-99 != XdrvMailbox.payload) {
        dragonOffset_sync = ((uint64_t) millis()) - ((uint64_t) XdrvMailbox.payload);
        dragonOffset_head = XdrvMailbox.payload;
        florp_millis = millis();
      }
      ResponseCmndIdxNumber(dragonOffset_head);
      break;
    case 10:
      if (-99 != XdrvMailbox.payload) {
        dragon_flex_a = (uint8_t) XdrvMailbox.payload;
        DragonColorlistInit();
      }
      ResponseCmndIdxNumber(dragon_flex_a);
      break;
    case 11:
      if (-99 != XdrvMailbox.payload) {
        dragon_flex_b = (uint8_t) XdrvMailbox.payload;
        DragonColorlistInit();
      }
      ResponseCmndIdxNumber(dragon_flex_b);
      break;
    case 12:
      if (-99 != XdrvMailbox.payload) {
        dragon_flex_c = (uint8_t) XdrvMailbox.payload;
        DragonColorlistInit();
      }
      ResponseCmndIdxNumber(dragon_flex_c);
      break;
    case 13: {
      struct RgbwColor c = DragonFx_Test(XdrvMailbox.payload, Settings->light_color[1]);
      ResponseCmndIdxNumber((c.W << 24) + (c.R << 16) + (c.G << 8) + c.B);
      } break;
    case 14: {
      if (-99 != XdrvMailbox.payload) {
        dragonOverlay_type = (uint8_t) XdrvMailbox.payload;
        dragonOverlay_status = 0;
        dragonOverlaySpeed = 600;
      }
      ResponseCmndIdxNumber(dragonOverlay_type);
      break;
    }
    case 15: {
      misan_active_template = 0;
      if (-99 != XdrvMailbox.payload) {
        int16_t idx = ((uint32_t) XdrvMailbox.payload) >> 16;
        bool wide = idx >= 8192;
        if (wide) idx -= 8192;
        if (idx < (MISAN_SIZE - (wide ? 1 : 0))) {
          misan[idx] = XdrvMailbox.payload & 0xFF;
          if (wide) misan[idx+1] = (XdrvMailbox.payload >> 8) & 0xFF;
        }
      } else {
        // byte 0 = checksum (sum of all bytes)
        // byte 1,2 = start address in MISAN
        // byte 3..x = BASE64 of the data

        // in-place BASE64 decode
        unsigned int output_length = decode_base64((unsigned char*) XdrvMailbox.data, (unsigned char*) XdrvMailbox.data);

        // first pass: verify checksum, check total length
        uint8_t crc = 0;
        if (output_length < 3) { ResponseCmndIdxNumber(-1); return; }
        for(uint8_t idx=1; idx<output_length; idx++) { crc += XdrvMailbox.data[idx]; }
        if (crc != XdrvMailbox.data[0]) { ResponseCmndIdxNumber(-2); return; }
        uint16_t targetIdx = XdrvMailbox.data[1] + (XdrvMailbox.data[2] << 8);
        if (targetIdx + output_length - 3 >= MISAN_SIZE) { ResponseCmndIdxNumber(-3); return; }

        // second pass: set MISAN
        for(uint8_t idx=3; idx<output_length; idx++, targetIdx++) {
          misan[targetIdx] = XdrvMailbox.data[idx];
        }
      }
      ResponseCmndIdxNumber(0);
      break;
    }
    case 16: { // read MISAN
      if (-99 != XdrvMailbox.payload) {
        int16_t idx = ((uint32_t) XdrvMailbox.payload) >> 16;
        bool wide = idx >= 8192;
        if (wide) idx -= 8192;
        if (idx < (MISAN_SIZE - (wide ? 1 : 0))) {
          ResponseCmndIdxNumber((idx << 16) + misan[idx] + (wide ? (misan[idx+1] << 8) + (8192 << 16) : 0));
        }
      }
      break;
    }
    case 17: { // read MISAN cooked
      struct drgn_misan_segment *seg = reinterpret_cast<struct drgn_misan_segment*> (&misan[XdrvMailbox.payload >> 8]);
      struct drgn_misan_blendtable *table = reinterpret_cast<struct drgn_misan_blendtable*> (&misan[XdrvMailbox.payload >> 8]);
      switch (XdrvMailbox.payload & 0xFF) {
        case 0: ResponseCmndIdxNumber(seg->start); break;
        case 1: ResponseCmndIdxNumber(seg->length); break;
        case 2: ResponseCmndIdxNumber(seg->type); break;
        case 3: ResponseCmndIdxNumber(seg->singlecolor.r); break;
        case 4: ResponseCmndIdxNumber(seg->singlecolor.g); break;
        case 5: ResponseCmndIdxNumber(seg->singlecolor.b); break;
        case 6: ResponseCmndIdxNumber(seg->linear.speed); break;
        case 7: ResponseCmndIdxNumber(seg->linear.px_offset); break;
        case 8: ResponseCmndIdxNumber(seg->linear.circle_speed); break;
        case 9: ResponseCmndIdxNumber(seg->linear.circle_offset); break;
        case 10: ResponseCmndIdxNumber(seg->linear.color_table_idx); break;
        case 11: ResponseCmndIdxNumber(seg->linear.color_offset); break;
        case 12: ResponseCmndIdxNumber(seg->copy.boundary_mode); break;
        case 13: ResponseCmndIdxNumber(seg->copy.start); break;
        case 14: ResponseCmndIdxNumber(seg->copy.length); break;
        case 15: ResponseCmndIdxNumber(seg->copy.type); break;
        case 16: ResponseCmndIdxNumber(seg->copy.param); break;
        case 20: ResponseCmndIdxNumber(table->size); break;
        case 21: ResponseCmndIdxNumber(table->totalLen); break;
        case 22: ResponseCmndIdxNumber(Light.power); break;
        case 23: ResponseCmndIdxNumber(seg->linear.px_dimm); break;
        case 24: ResponseCmndIdxNumber(seg->power_selector); break;
        case 25: ResponseCmndIdxNumber(seg->dimm_channel); break;
        case 26: ResponseCmndIdxNumber(seg->disabled); break;
        case 27: ResponseCmndIdxNumber(seg->unused); break;
        case 28: ResponseCmndIdxNumber(seg->white.active_selector); break;
        case 29: ResponseCmndIdxNumber(seg->white.balance_channel); break;
        case 30: ResponseCmndIdxNumber(seg->blackout); break;
        case 100: ResponseCmndIdxNumber(sizeof(struct drgn_misan_segment)); break;
        case 101: ResponseCmndIdxNumber(sizeof(struct drgn_misan_blendtable)); break;
        case 102: ResponseCmndIdxNumber(sizeof(struct drgn_misan_blendentry)); break;
        case 103: ResponseCmndIdxNumber(sizeof(int)); break;
        case 104: ResponseCmndIdxNumber(sizeof(misan)); break;
        default: {
          uint8_t idx = (XdrvMailbox.payload & 0xFF) - 50;
          uint8_t entryId = idx / 5;
          switch (idx % 5) {
            case 0: ResponseCmndIdxNumber(table->entry[entryId].r); break;
            case 1: ResponseCmndIdxNumber(table->entry[entryId].g); break;
            case 2: ResponseCmndIdxNumber(table->entry[entryId].b); break;
            case 3: ResponseCmndIdxNumber(table->entry[entryId].len); break;
            case 4: ResponseCmndIdxNumber(table->entry[entryId].lensum); break;
          }
          break; }
      } break;
    } break;
    case 18: // set MISAN template
      if (-99 != XdrvMailbox.payload) {
        uint8_t mode = (uint8_t) XdrvMailbox.payload;
        SetMisanTemplate(mode);
      } 
      ResponseCmndIdxNumber(misan_active_template);
      break;
  }
}
#endif // USE_DERG_RGB

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xlgt01(uint8_t function)
{
  bool result = false;

  switch (function) {
    case FUNC_SET_CHANNELS:
      result = Ws2812SetChannels();
      break;
    case FUNC_SET_SCHEME:
      Ws2812ShowScheme();
      break;
    case FUNC_COMMAND:
      result = DecodeCommand(kWs2812Commands, Ws2812Command);
      break;
    case FUNC_MODULE_INIT:
      Ws2812ModuleSelected();
      break;
  }
  return result;
}

#endif  // USE_WS2812
#endif  // USE_LIGHT
