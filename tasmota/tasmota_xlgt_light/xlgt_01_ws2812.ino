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

#if defined(ESP8266) || defined(USE_WS2812_FORCE_NEOPIXELBUS)
#ifdef USE_LIGHT
#ifdef USE_WS2812

/*********************************************************************************************\
 * WS2812 RGB / RGBW Leds using NeopixelBus library
 *
 * light_scheme  WS2812  3+ Colors  1+2 Colors  Effect
 * ------------  ------  ---------  ----------  -----------------
 *  0 (5)        yes     no         no          Clock
 *  1 (6)        yes     no         no          Incandescent
 *  2 (7)        yes     no         no          RGB
 *  3 (8)        yes     no         no          Christmas
 *  4 (9)        yes     no         no          Hanukkah
 *  5 (10)       yes     no         no          Kwanzaa
 *  6 (11)       yes     no         no          Rainbow
 *  7 (12)       yes     no         no          Fire
 *  8 (13)       yes     no         no          Stairs
 *  9 (14)       yes     no         no          Clear (= Berry)
 * 10 (15)       yes     no         no          Optional DDP
\*********************************************************************************************/

#define XLGT_01             1

#ifdef USE_DERG_RGB
const uint8_t WS2812_SCHEMES = 11;      // Number of WS2812 schemes
#else
const uint8_t WS2812_SCHEMES = 10;      // Number of WS2812 schemes
#endif // USE_DERG_RGB

const char kWs2812Commands[] PROGMEM = "|"  // No prefix
#ifdef USE_DERG_RGB
  D_CMND_DRAGON "|"
#endif // USE_DERG_RGB
  D_CMND_LED "|" D_CMND_PIXELS "|" D_CMND_ROTATION "|" D_CMND_WIDTH "|" D_CMND_STEPPIXELS ;

void (* const Ws2812Command[])(void) PROGMEM = {
#ifdef USE_DERG_RGB
  &CmndDragon,
#endif // USE_DERG_RGB
  &CmndLed, &CmndPixels, &CmndRotation, &CmndWidth, &CmndStepPixels };

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
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
typedef NeoEsp32SpiN800KbpsMethod   selectedNeoSpeedType;
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
WsColor kStairs[2] = { 0,0,0, 255,255,255 };

#ifdef USE_DERG_RGB
ColorScheme kSchemes[WS2812_SCHEMES -3] = {  // Skip clock and clear and Dragon scheme
#else
ColorScheme kSchemes[WS2812_SCHEMES -2] = {  // Skip clock and clear scheme
#endif // USE_DERG_RGB
  kIncandescent, 2,
  kRgb, 3,
  kChristmas, 2,
  kHanukkah, 2,
  kwanzaa, 3,
  kRainbow, 7,
  kFire, 3,
  kStairs, 2 };

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

void Ws2812LibStripShow(void) {
  strip->Show();

#if defined(USE_WS2812_DMA) || defined(USE_WS2812_RMT) || defined(USE_WS2812_I2S)
  // Wait for DMA/RMT/I2S to complete fixes distortion due to analogRead
//  delay((Settings->light_pixels >> 6) +1);  // 256 / 64 = 4 +1 = 5
  SystemBusyDelay( (Settings->light_pixels + 31) >> 5);  // (256 + 32) / 32 = 8
#endif
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
  Ws2812LibStripShow();
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

void Ws2812Steps(uint32_t schemenr) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif

  ColorScheme scheme = kSchemes[schemenr];
	// apply main color if current sheme == kStairs
	if (scheme.colors == kStairs) {
		scheme.colors[1].red = Settings->light_color[0];
		scheme.colors[1].green = Settings->light_color[1];
		scheme.colors[1].blue = Settings->light_color[2];
	}

	uint8_t scheme_count = scheme.count;
	if (Settings->light_fade) {
		scheme_count = Settings->ws_width[WS_HOUR];  // Width4
	}
  if (scheme_count < 2) {
    scheme_count = 2;
  }

  WsColor mcolor[scheme_count];

	uint8_t color_start = 0;
	uint8_t color_end = 1;
	if (Settings->light_rotation & 0x01) {
		color_start = 1;
		color_end = 0;
	}

	if (Settings->light_fade) {
		// generate gradient (width = Width4)
		for (uint32_t i = 1; i < scheme_count - 1; i++) {
			mcolor[i].red = (uint8_t) wsmap(i, 0, scheme_count, scheme.colors[color_start].red, scheme.colors[color_end].red);
			mcolor[i].green = (uint8_t) wsmap(i, 0, scheme_count, scheme.colors[color_start].green, scheme.colors[color_end].green);
			mcolor[i].blue = (uint8_t) wsmap(i, 0, scheme_count, scheme.colors[color_start].blue, scheme.colors[color_end].blue);
		}
	} else {
		memcpy(mcolor, scheme.colors, sizeof(mcolor));
	}
	// Repair first & last color in gradient; apply scheme rotation if fade==0
	mcolor[0].red = scheme.colors[color_start].red;
	mcolor[0].green = scheme.colors[color_start].green;
	mcolor[0].blue = scheme.colors[color_start].blue;
	mcolor[scheme_count-1].red = scheme.colors[color_end].red;
	mcolor[scheme_count-1].green = scheme.colors[color_end].green;
	mcolor[scheme_count-1].blue = scheme.colors[color_end].blue;

	// Adjust to dimmer value
  float dimmer = 100 / (float)Settings->light_dimmer;
  for (uint32_t i = 0; i < scheme_count; i++) {
    float fmyRed = (float)mcolor[i].red / dimmer;
    float fmyGrn = (float)mcolor[i].green / dimmer;
    float fmyBlu = (float)mcolor[i].blue / dimmer;
    mcolor[i].red = (uint8_t)fmyRed;
    mcolor[i].green = (uint8_t)fmyGrn;
    mcolor[i].blue = (uint8_t)fmyBlu;
  }

  uint32_t speed = Settings->light_speed;
	int32_t current_position = Light.strip_timer_counter / speed;

	//all pixels are shown already | rotation change will not change current state
	if (current_position >  Settings->light_pixels / Settings->light_step_pixels + scheme_count ) {
		return;
	}

  int32_t colorIndex;
  int32_t step_nr;

  for (uint32_t i = 0; i < Settings->light_pixels; i++) {
		step_nr = i / Settings->light_step_pixels;
		colorIndex = current_position - step_nr;
	  if (colorIndex < 0) { colorIndex = 0; }
		if (colorIndex > scheme_count - 1) { colorIndex = scheme_count - 1; }
    c.R = mcolor[colorIndex].red;
    c.G = mcolor[colorIndex].green;
    c.B = mcolor[colorIndex].blue;
		// Adjust the scheme rotation
		if (Settings->light_rotation & 0x02) {
			strip->SetPixelColor(Settings->light_pixels - i - 1, c);
		} else {
			strip->SetPixelColor(i, c);
		}
  }
  Ws2812StripShow();
}

#ifdef USE_NETWORK_LIGHT_SCHEMES
void Ws2812DDP(void)
{
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  c.R = 0;
  c.G = 0;
  c.B = 0;

  // Can't be trying to initialize UDP too early.
  if (TasmotaGlobal.restart_flag || TasmotaGlobal.global_state.network_down) return;

  // Start DDP listener, if fail, just set last ddp_color
  if (!ddp_udp_up) {
    if (!ddp_udp.begin(4048)) return;
    ddp_udp_up = 1;
    AddLog(LOG_LEVEL_DEBUG_MORE, "DDP: UDP Listener Started: WS2812 Scheme");
  }

  // Get the DDP payload over UDP
  std::vector<uint8_t> payload;
  while (uint16_t packet_size = ddp_udp.parsePacket()) {
    payload.resize(packet_size);
    if (!ddp_udp.read(&payload[0], payload.size())) {
      continue;
    }
  }

  // No verification checks performed against packet besides length
  if (payload.size() > (9+3*Settings->light_pixels)) {
    for (uint32_t i = 0; i < Settings->light_pixels; i++) {
      c.R = payload[10+3*i];
      c.G = payload[11+3*i];
      c.B = payload[12+3*i];
      strip->SetPixelColor(i, c);
    }
    Ws2812StripShow();
  }
}
#endif  // USE_NETWORK_LIGHT_SCHEMES

#ifdef USE_DERG_RGB
// note: when using Settings->dimmer, scale it: changeUIntScale(dimmer, 0, 100, 0, 255)

void Ws2812Dragon(void)
{
  DragonFx_Misan();
  Ws2812StripShow();
}

uint8_t map2(int8_t a, int32_t b, int32_t y, uint16_t z) {
	return ((uint8_t) (b * z / y)) + a;
}

#define MISAN_SIZE 500
uint8_t misan[MISAN_SIZE];

#pragma pack(push, 1)
struct drgn_misan_part {
  uint16_t start; // index of first led of this part (zero-based)
  uint16_t length; // number of leds in this part
};
#define SIZEOF_DRGN_MISAN_PART 4
/* definitions of all known strips in this setup. Strips may overlap. */
struct drgn_misan_parts {
  uint8_t partcount;
  struct drgn_misan_part parts[];
};
#define SIZEOF_DRGN_MISAN_PARTS 1
struct drgn_misan_segment {
  /* Effect for this segment
   * 0 - END MARKER
   * 1 - black / off
   * 2 - linear blend, linear segment
   * 3 - linear blend, circular segment
   * 4 - copy colors from other parts of the strip
   * 5 - static single color
   * 6 - white-balanced white (parameter: 0=adds blue, 50=only white, 100=adds red)
   */
  uint8_t type;
  uint8_t partId;
  struct { // segment control bitfield
    unsigned int power_selector: 3; // 0=always on, otherwise see value of PowerX. If power is OFF, see parameter 'blackout'
    unsigned int dimm_channel: 2; // use ChannelX for brightness control (0=full brightness)
    unsigned int disabled: 1; // effect disabled, no colors are set/changed
    unsigned int blackout: 1; // if effect is off (based on power_selector), 0=no colors are set/changed 1=everything set to black
    unsigned int unused: 1;
  };
  union {
    struct {
      uint8_t r, g, b;
    } singlecolor;
    struct {
      int16_t speed; // speed at which color_offset moves (in ms)
      int8_t px_offset; // pixel-cummulative added color table offset
      uint8_t px_dimm; // pixel-cummulative brightness multiplier, 0x80 = unchanged
      struct {
        uint8_t color_table_id: 4; // id of the color table (zero-based)
        unsigned int unusedA: 1;
        unsigned int unusedB: 1;
        unsigned int unusedC: 1;
        unsigned int unusedD: 1;
      };
      uint16_t color_offset; // initial/current offest in color table
      int16_t circle_speed; // only for fx=3: speed at which circle_offset moves (in ms)
      int8_t circle_offset; // only for fx=3: initial/current offset of 'first pixel' relative to pixel range
    } linear;
    struct {
      /* handling of different source/target lengths
       *  0 - wrap around
       *  1 - mirror
       *  2 - stretch
       * 10 - reverse wrap
       * 11 - not implemented: reverse mirror
       * 12 - reverse stretch
       */
      uint8_t boundary_mode;
      /** copy source index */
      uint8_t srcPartId;
      /** Copy modification type
       * 0 - none, copy exactly (but respect dimm_channel)
       * 1 - dim, (0..255 -> 0..param)
       * 2 - brighten (0..param -> 0..255)
       * 3 - use dimm_channel to darken or brighten (128 = unchanged)
       */
      uint8_t type;
      /** parameter for copy type */
      uint16_t param;
    } copy;
    struct {
      uint8_t active_selector; // White Balancing: 0=active, >0 depending on PowerX
      uint8_t balance_channel;
    } white;
  };
};
struct drgn_misan_blendentry {
  uint8_t r, g, b;
  uint16_t len;
  uint16_t lensum; // sum of all len of previous entries
};
#define SIZEOF_DRGN_MISAN_BLENDENTRY 7
struct drgn_misan_blendtable {
  uint8_t size; // number of entries
  uint16_t totalLen; // sum of all len of all entries
  struct drgn_misan_blendentry entry[]; // actual size is dynamic
};
#define SIZEOF_DRGN_MISAN_BLENDTABLE 3
#pragma pack(pop)
// size: 6 bytes header + X bytes for the respective struct (except END_MARKER as special case)
uint8_t drgn_misan_segSizeByType[] = { 1, 3, 10, 13, 8, 6, 5 };

// returns the lengths of the segment definition based on the segment effect. Returns MISAN_SIZE (as "too big value") for unknown types
uint16_t DragonFx_Misan_getSegmentSizeByType(uint8_t type) {
  return type < sizeof(drgn_misan_segSizeByType) ? drgn_misan_segSizeByType[type] : MISAN_SIZE;
}

struct drgn_misan_part DragonFx_Misan_MaximumPart; // a fallback part definition for the full strip length
uint16_t DragonFx_Misan_SegmentsIdx; // index in MISAN for the first segment
uint16_t DragonFx_Misan_ColortablesIdx; // index in MISAN for the first color table
// call after changes to MISAN, for some validity checks and cache updates
void DragonFx_Misan_Initialize() {
  DragonFx_Misan_MaximumPart.start = 0;
  DragonFx_Misan_MaximumPart.length = Settings->light_pixels;
  struct drgn_misan_parts *drgn_misan_parts = reinterpret_cast<struct drgn_misan_parts*> (&misan[0]);
  for(uint8_t idx = 0; idx < drgn_misan_parts->partcount; idx++) {
    if (drgn_misan_parts->parts[idx].start >= Settings->light_pixels) drgn_misan_parts->parts[idx].start = Settings->light_pixels - 1;
    if (drgn_misan_parts->parts[idx].start + drgn_misan_parts->parts[idx].length > Settings->light_pixels) drgn_misan_parts->parts[idx].length = Settings->light_pixels - drgn_misan_parts->parts[idx].start;
  }

  DragonFx_Misan_SegmentsIdx = SIZEOF_DRGN_MISAN_PARTS + drgn_misan_parts->partcount * SIZEOF_DRGN_MISAN_PART; // segment definitions start after part definitions

  uint16_t misanIdx = DragonFx_Misan_SegmentsIdx;
  do {
    if (misanIdx >= MISAN_SIZE) break; // emergency brake
    struct drgn_misan_segment *seg = reinterpret_cast<struct drgn_misan_segment*> (&misan[misanIdx]);
    misanIdx += DragonFx_Misan_getSegmentSizeByType(seg->type);
    if (seg->type == 0) break;
  } while(true);
  DragonFx_Misan_ColortablesIdx = misanIdx;
}

struct drgn_misan_part* DragonFx_Misan_getPart(uint8_t partId) {
  struct drgn_misan_parts *drgn_misan_parts = reinterpret_cast<struct drgn_misan_parts*> (&misan[0]);
  if (partId < drgn_misan_parts->partcount) return &drgn_misan_parts->parts[partId];
  return &DragonFx_Misan_MaximumPart;
}

// returns segment number 'id', starts with 0, nullptr if id doesn't exist
struct drgn_misan_segment* DragonFx_getSegmentById(uint8_t id) {
  uint16_t misanIdx = DragonFx_Misan_SegmentsIdx;
  do {
    if (misanIdx >= MISAN_SIZE) return nullptr; // emergency brake
    struct drgn_misan_segment *seg = reinterpret_cast<struct drgn_misan_segment*> (&misan[misanIdx]);
    if (seg->type == 0) return nullptr;
    if (id == 0) return seg;
    misanIdx += DragonFx_Misan_getSegmentSizeByType(seg->type);
    id--;
  } while(true);
}

struct drgn_misan_blendtable* DragonFx_getColortableById(uint8_t id) {
  uint16_t misanIdx = DragonFx_Misan_ColortablesIdx;
  do {
    if (misanIdx >= MISAN_SIZE) return nullptr; // emergency brake
    struct drgn_misan_blendtable *table = reinterpret_cast<struct drgn_misan_blendtable*> (&misan[misanIdx]);
    if (id == 0) return table;
    misanIdx += SIZEOF_DRGN_MISAN_BLENDTABLE + table->size * SIZEOF_DRGN_MISAN_BLENDENTRY;
    id--;
  } while(true);
}

void DragonFx_Misan() {
  uint16_t misanIdx = DragonFx_Misan_SegmentsIdx;
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

unsigned long dragon_millis_offset = 0; // TODO rename dragon_millis_offset
void DragonFx_Misan_linear(struct drgn_misan_segment *seg, bool circular) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor c;
  c.W = 0;
#else
  RgbColor c;
#endif
  // TODO use a segment-local dragon_millis_offset
  unsigned long now = millis() - dragon_millis_offset;
  struct drgn_misan_part* part = DragonFx_Misan_getPart(seg->partId);
  struct drgn_misan_blendtable *table = DragonFx_getColortableById(seg->linear.color_table_id);

  int32_t blend_index_current = seg->linear.color_offset + (seg->linear.speed == 0 ? 0 : seg->linear.speed > 0 ? now / seg->linear.speed : table->totalLen - 1 - ((now / (-seg->linear.speed)) % table->totalLen));
  int32_t circle_offset_current = 0;
  if (circular) 
    circle_offset_current = seg->linear.circle_offset + (seg->linear.circle_speed == 0 ? 0 : seg->linear.circle_speed > 0 ? now / seg->linear.circle_speed : part->length - 1 - ((now / (-seg->linear.circle_speed)) % part->length));

  uint8_t cci = 0;
  uint8_t cached_cci = -1; // cache starts invalid
  uint16_t len;
  uint8_t r, g, b;
  int16_t dr, dg, db;
  uint16_t px_count = circular && part->length > 0 ? (part->length >> 1) + 1 : part->length;
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
    uint16_t dst_idx = part->start + ((circle_offset_current + i) % part->length);
    if (dst_idx < Settings->light_pixels) {
      strip->SetPixelColor(dst_idx, c);
    }

    if (circular) {
      uint16_t dst_idx2 = part->start + ((part->length + circle_offset_current - i) % part->length);
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
  struct drgn_misan_part* part = DragonFx_Misan_getPart(seg->partId);
  struct drgn_misan_part* srcPart = DragonFx_Misan_getPart(seg->copy.srcPartId);
  uint16_t src_start = srcPart->start;
  uint16_t dst_start = part->start;
  uint8_t dimmer = seg->dimm_channel > 0 ? Settings->light_color[seg->dimm_channel-1] : 255;
  // TODO: handle overlapping areas
  uint16_t segLen = abs(part->length); // TODO can't currently happen, as part->length is unsigned
  for (uint16_t idx = 0; idx < segLen; idx++) {
    uint16_t dst_idx = part->length >= 0 ? dst_start + idx : dst_start - part->length - 1 - idx;
    if (dst_idx >= Settings->light_pixels) continue;
    uint16_t src_idx;
    switch(seg->copy.boundary_mode) {
      case 1: { // mirror
        uint16_t j = idx % ((srcPart->length-1) << 1);
        if (j >= srcPart->length) j = 2 * (srcPart->length-1) - j;
        src_idx = src_start + j;
        break; }
      case 2: // stretch
        src_idx = src_start + changeUIntScale(idx, 0, segLen-1, 0, srcPart->length-1);
        break;
      case 10: // reverse wrap
        src_idx = src_start + srcPart->length-1 - (idx % srcPart->length);
        break;
      case 12: // reverse stretch
        src_idx = src_start + changeUIntScale(idx, 0, segLen-1, srcPart->length-1, 0);
        break;
      default: // wrap
        src_idx = src_start + (idx % srcPart->length);
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
  struct drgn_misan_part* part = DragonFx_Misan_getPart(seg->partId);
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
  uint16_t dst_start = part->start;
  for (uint16_t i = 0; i < part->length; i++) {
    uint16_t dst_idx = dst_start + i;
    if (dst_idx >= Settings->light_pixels) break;
    strip->SetPixelColor(dst_idx, c);
  }
}

void DragonFx_Misan_white(struct drgn_misan_segment *seg) {
  struct drgn_misan_part* part = DragonFx_Misan_getPart(seg->partId);
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

  uint16_t dst_start = part->start;
  for (uint16_t i = 0; i < part->length; i++) {
    uint16_t dst_idx = dst_start + i;
    if (dst_idx >= Settings->light_pixels) break;
    strip->SetPixelColor(dst_idx, c);
  }
}

uint8_t misan_active_partlist = 0;
uint8_t misan_active_template = 0;
void SetMisanPartlist(uint8_t partlistIdx) {
  switch(partlistIdx) {
    case 0:
    default:
      // no (valid) partlist
      return;
    case 1: { // dancer
      uint8_t new_parts[] = { 0x0d, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x1c, 0x00, 0x04, 0x00, 0x20, 0x00, 0x0c, 0x00, 0x2c, 0x00, 0x04, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x3c, 0x00, 0x04, 0x00, 0x40, 0x00, 0x0c, 0x00, 0x4c, 0x00, 0x04, 0x00, 0x50, 0x00, 0x0c, 0x00, 0x5c, 0x00, 0x04, 0x00, 0x00, 0x00, 0x70, 0x00 };
      std::copy(new_parts, new_parts+sizeof(new_parts)/sizeof(new_parts[0]), misan);
      break;
    }
    case 2: { // bad
      uint8_t new_parts[] = { 0x06, 0x00, 0x00, 0x70, 0x00, 0x70, 0x00, 0x49, 0x00, 0xb9, 0x00, 0x3d, 0x00, 0x70, 0x00, 0x86, 0x00, 0x00, 0x00, 0xf6, 0x00, 0x36, 0x00, 0x04, 0x00 };
      std::copy(new_parts, new_parts+sizeof(new_parts)/sizeof(new_parts[0]), misan);
      break;
    }
    case 3: { // flur
      uint8_t new_parts[] = { 0x05, 0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x2b, 0x00, 0x4b, 0x00, 0x2b, 0x00, 0x20, 0x00, 0x56, 0x00, 0x00, 0x00, 0x76, 0x00 };
      std::copy(new_parts, new_parts+sizeof(new_parts)/sizeof(new_parts[0]), misan);
      break;
    }
    case 4: { // food
      uint8_t new_parts[] = { 0x01, 0x00, 0x00, 0x70, 0x00 };
      std::copy(new_parts, new_parts+sizeof(new_parts)/sizeof(new_parts[0]), misan);
      break;
    }
  }
  DragonFx_Misan_Initialize();
  misan_active_partlist = partlistIdx;
  // clear segments (because they are probably technically misaligned and semantically not fitting now)
  struct drgn_misan_segment *seg = reinterpret_cast<struct drgn_misan_segment*> (&misan[DragonFx_Misan_SegmentsIdx]);
  seg->type = 0;
  misan_active_template = 0;
  Ws2812Clear(false);
}

void SetMisanTemplate(uint8_t mode) {
  uint8_t *segments = &misan[DragonFx_Misan_SegmentsIdx];
  switch (mode) {
    case 0:
    default:
      // no (valid) template
      return;
    case 1: { // flur pattern
      uint8_t new_segment[] = { 0x01, 0x04, 0x00, 0x03, 0x00, 0x29, 0x0a, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x29, 0x0a, 0x00, 0x0a, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x52, 0x2b, 0x02, 0x04, 0x80, 0x02, 0x00, 0x00, 0x02, 0x02, 0x52, 0x2b, 0x02, 0xfc, 0x80, 0x02, 0xac, 0x00, 0x00, 0x06, 0x58, 0x02, 0x00, 0x40, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x64, 0x00, 0x64, 0x00, 0x00, 0x20, 0x00, 0x64, 0x00, 0xc8, 0x00, 0x00, 0x60, 0x00, 0x64, 0x00, 0x2c, 0x01, 0x00, 0x80, 0x00, 0x64, 0x00, 0x90, 0x01, 0x00, 0x60, 0x00, 0x64, 0x00, 0xf4, 0x01, 0x06, 0x58, 0x02, 0x40, 0x02, 0x00, 0x64, 0x00, 0x00, 0x00, 0x80, 0x04, 0x00, 0x64, 0x00, 0x64, 0x00, 0x20, 0x01, 0x00, 0x64, 0x00, 0xc8, 0x00, 0x60, 0x03, 0x00, 0x64, 0x00, 0x2c, 0x01, 0x80, 0x04, 0x00, 0x64, 0x00, 0x90, 0x01, 0x60, 0x03, 0x00, 0x64, 0x00, 0xf4, 0x01, 0x09, 0x42, 0x0e, 0x00, 0xff, 0x00, 0x58, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0x58, 0x02, 0x58, 0x02, 0x00, 0x00, 0xff, 0x58, 0x02, 0xb0, 0x04, 0xff, 0x00, 0xff, 0xae, 0x01, 0x08, 0x07, 0xff, 0x00, 0x05, 0xaa, 0x00, 0xb6, 0x08, 0xff, 0x00, 0x00, 0x32, 0x00, 0x60, 0x09, 0xff, 0x00, 0x00, 0xaa, 0x00, 0x92, 0x09, 0xff, 0x05, 0x00, 0xae, 0x01, 0x3c, 0x0a, 0xff, 0xff, 0x00, 0x58, 0x02, 0xea, 0x0b };
      std::copy(new_segment, new_segment+sizeof(new_segment)/sizeof(new_segment[0]), segments);
      break;
    }
    case 2: { // Food Strip
      uint8_t new_segment[] = { 0x06, 0x00, 0x52, 0x03, 0x03, 0x06, 0x00, 0x09, 0x03, 0x03, 0x00 };
      std::copy(new_segment, new_segment+sizeof(new_segment)/sizeof(new_segment[0]), segments);
      break;
    }
    case 3: { // Dancer pattern
      uint8_t new_segment[] = { 0x03, 0x00, 0x09, 0x0a, 0x00, 0x14, 0x80, 0x00, 0x00, 0x00, 0xfa, 0x00, 0x04, 0x04, 0x01, 0x12, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x03, 0x02, 0x09, 0x0a, 0x00, 0x14, 0x80, 0x00, 0xc8, 0x00, 0xfa, 0x00, 0x05, 0x04, 0x03, 0x12, 0x0c, 0x02, 0x01, 0x80, 0x00, 0x03, 0x04, 0x09, 0x0a, 0x00, 0x14, 0x80, 0x00, 0x90, 0x01, 0xfa, 0x00, 0x03, 0x04, 0x05, 0x12, 0x0c, 0x04, 0x01, 0x80, 0x00, 0x03, 0x06, 0x09, 0x0a, 0x00, 0x14, 0x80, 0x00, 0x58, 0x02, 0xfa, 0x00, 0x0a, 0x04, 0x07, 0x12, 0x0c, 0x06, 0x01, 0x80, 0x00, 0x03, 0x08, 0x09, 0x0a, 0x00, 0x14, 0x80, 0x00, 0x20, 0x03, 0xfa, 0x00, 0x09, 0x04, 0x09, 0x12, 0x0c, 0x08, 0x01, 0x80, 0x00, 0x03, 0x0a, 0x09, 0x0a, 0x00, 0x14, 0x80, 0x00, 0xe8, 0x03, 0xfa, 0x00, 0x0a, 0x04, 0x0b, 0x12, 0x0c, 0x0a, 0x01, 0x80, 0x00, 0x00, 0x06, 0xb0, 0x04, 0xff, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xc8, 0x00, 0xc8, 0x00, 0x00, 0xff, 0x00, 0xc8, 0x00, 0x90, 0x01, 0x00, 0xff, 0xff, 0xc8, 0x00, 0x58, 0x02, 0x00, 0x00, 0xff, 0xc8, 0x00, 0x20, 0x03, 0xff, 0x00, 0xff, 0xc8, 0x00, 0xe8, 0x03 };
      std::copy(new_segment, new_segment+sizeof(new_segment)/sizeof(new_segment[0]), segments);
      break;
    }
    case 4: { // Dancer pattern (variant2)
      uint8_t new_segment[] = { 0x03, 0x00, 0x09, 0xc8, 0x00, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x06, 0xff, 0x04, 0x04, 0x01, 0x12, 0x0c, 0x00, 0x03, 0x80, 0x00, 0x03, 0x02, 0x09, 0xc8, 0x00, 0x00, 0x6f, 0x00, 0xc8, 0x00, 0xfa, 0x00, 0x05, 0x04, 0x03, 0x12, 0x0c, 0x02, 0x03, 0x80, 0x00, 0x03, 0x04, 0x09, 0xc8, 0x00, 0x00, 0x6f, 0x00, 0x90, 0x01, 0x06, 0xff, 0x03, 0x04, 0x05, 0x12, 0x0c, 0x04, 0x03, 0x80, 0x00, 0x03, 0x06, 0x09, 0xc8, 0x00, 0x00, 0x6f, 0x00, 0x58, 0x02, 0xfa, 0x00, 0x0a, 0x04, 0x07, 0x12, 0x0c, 0x06, 0x03, 0x80, 0x00, 0x03, 0x08, 0x09, 0xc8, 0x00, 0x00, 0x6f, 0x00, 0x20, 0x03, 0x06, 0xff, 0x09, 0x04, 0x09, 0x12, 0x0c, 0x08, 0x03, 0x80, 0x00, 0x03, 0x0a, 0x09, 0xc8, 0x00, 0x00, 0x6f, 0x00, 0xe8, 0x03, 0xfa, 0x00, 0x0a, 0x04, 0x0b, 0x12, 0x0c, 0x0a, 0x03, 0x80, 0x00, 0x00, 0x10, 0x97, 0x04, 0xff, 0x00, 0x00, 0x96, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x19, 0x00, 0x96, 0x00, 0xff, 0xff, 0x00, 0x19, 0x00, 0xaf, 0x00, 0x00, 0xff, 0xff, 0x96, 0x00, 0xc8, 0x00, 0x00, 0xff, 0xff, 0x19, 0x00, 0x5e, 0x01, 0x00, 0xff, 0x00, 0x96, 0x00, 0x77, 0x01, 0x00, 0xff, 0x00, 0x19, 0x00, 0x0d, 0x02, 0xff, 0xff, 0x00, 0x19, 0x00, 0x26, 0x02, 0xff, 0x00, 0xff, 0x96, 0x00, 0x3f, 0x02, 0xff, 0x00, 0xff, 0x19, 0x00, 0xd5, 0x02, 0xff, 0x00, 0x00, 0x19, 0x00, 0xee, 0x02, 0xff, 0xff, 0x00, 0x96, 0x00, 0x07, 0x03, 0xff, 0xff, 0x00, 0x19, 0x00, 0x9d, 0x03, 0x00, 0xff, 0x00, 0x19, 0x00, 0xb6, 0x03, 0x00, 0x00, 0xff, 0x96, 0x00, 0xcf, 0x03, 0x00, 0x00, 0xff, 0x32, 0x00, 0x65, 0x04 }; 
      std::copy(new_segment, new_segment+sizeof(new_segment)/sizeof(new_segment[0]), segments);
      break;
    }
    case 5: { // Dancer pattern (boring test)
      uint8_t new_segment[] = { 0x03, 0x00, 0x09, 0x00, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x00, 0xb4, 0xe2, 0x04, 0x04, 0x01, 0x12, 0x0c, 0x00, 0x01, 0x80, 0x00, 0x03, 0x02, 0x09, 0x00, 0x00, 0x0a, 0x80, 0x00, 0x78, 0x00, 0x4c, 0x1d, 0x05, 0x04, 0x03, 0x12, 0x0c, 0x02, 0x01, 0x80, 0x00, 0x03, 0x04, 0x09, 0x00, 0x00, 0x0a, 0x80, 0x00, 0xf0, 0x00, 0xb4, 0xe2, 0x03, 0x04, 0x05, 0x12, 0x0c, 0x04, 0x01, 0x80, 0x00, 0x03, 0x06, 0x09, 0x00, 0x00, 0x0a, 0x80, 0x00, 0x68, 0x01, 0x4c, 0x1d, 0x0a, 0x04, 0x07, 0x12, 0x0c, 0x06, 0x01, 0x80, 0x00, 0x03, 0x08, 0x09, 0x00, 0x00, 0x0a, 0x80, 0x00, 0xe0, 0x01, 0xb4, 0xe2, 0x09, 0x04, 0x09, 0x12, 0x0c, 0x08, 0x01, 0x80, 0x00, 0x03, 0x0a, 0x09, 0x00, 0x00, 0x0a, 0x80, 0x00, 0x58, 0x02, 0x4c, 0x1d, 0x0a, 0x04, 0x0b, 0x12, 0x0c, 0x0a, 0x01, 0x80, 0x00, 0x00, 0x0c, 0x68, 0x01, 0xff, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0xff, 0x00, 0x0a, 0x00, 0x78, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xff, 0x0a, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x01 };
      std::copy(new_segment, new_segment+sizeof(new_segment)/sizeof(new_segment[0]), segments);
      break;
    }
    case 6: { // Bad Strip
      uint8_t new_segment[] = { 0x06, 0x00, 0x49, 0x03, 0x03, 0x02, 0x01, 0x52, 0x2b, 0x02, 0x04, 0x80, 0x00, 0x00, 0x00, 0x02, 0x02, 0x52, 0x2b, 0x02, 0xfc, 0x80, 0x00, 0x24, 0x01, 0x05, 0x05, 0x30, 0x80, 0xff, 0x00, 0x00, 0x09, 0x42, 0x0e, 0x00, 0xff, 0x00, 0x58, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0x58, 0x02, 0x58, 0x02, 0x00, 0x00, 0xff, 0x58, 0x02, 0xb0, 0x04, 0xff, 0x00, 0xff, 0xae, 0x01, 0x08, 0x07, 0xff, 0x00, 0x05, 0xaa, 0x00, 0xb6, 0x08, 0xff, 0x00, 0x00, 0x32, 0x00, 0x60, 0x09, 0xff, 0x00, 0x00, 0xaa, 0x00, 0x92, 0x09, 0xff, 0x05, 0x00, 0xae, 0x01, 0x3c, 0x0a, 0xff, 0xff, 0x00, 0x58, 0x02, 0xea, 0x0b };
      std::copy(new_segment, new_segment+sizeof(new_segment)/sizeof(new_segment[0]), segments);
      break;
    }
  }
  misan_active_template = mode;
  DragonFx_Misan_Initialize();
  Ws2812Clear(false);
}

#endif // USE_DERG_RGB

void Ws2812Clear(bool display = true);
void Ws2812Clear(bool display)
{
  strip->ClearTo(0);
  if (display) {
    Ws2812LibStripShow();
    Ws2812.show_next = 1;
  }
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
    Ws2812LibStripShow();
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
  Ws2812LibStripShow();
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

#ifdef USE_NETWORK_LIGHT_SCHEMES
  if ((scheme != 10) && (ddp_udp_up)) {
    ddp_udp.stop();
    ddp_udp_up = 0;
    AddLog(LOG_LEVEL_DEBUG_MORE, "DDP: UDP Stopped: WS2812 Scheme not DDP");
  }
#endif
  switch (scheme) {
    case 0:  // Clock
      if ((1 == TasmotaGlobal.state_250mS) || (Ws2812.show_next)) {
        Ws2812Clock();
        Ws2812.show_next = 0;
      }
      break;
    case 9:  // Clear
      if (Settings->light_scheme != Light.last_scheme) {
        Ws2812Clear();
      }
      break;
#ifdef USE_NETWORK_LIGHT_SCHEMES
    case 10:
      Ws2812DDP();
      break;
#endif  // USE_NETWORK_LIGHT_SCHEMES
#ifdef USE_DERG_RGB
    case 11: // Dragon
      Ws2812Dragon();
      Ws2812.show_next = 1;
      break;
#endif // USE_DERG_RGB
    default:
			if(Settings->light_step_pixels > 0){
				Ws2812Steps(scheme -1);
			} else {
				if (1 == Settings->light_fade) {
					Ws2812Gradient(scheme -1);
				} else {
					Ws2812Bars(scheme -1);
				}
      }
      Ws2812.show_next = 1;
      break;
  }
}

bool Ws2812InitStrip(void)
{
  if (strip != nullptr) {
    return true;
  }

#if (USE_WS2812_HARDWARE == NEO_HW_P9813)
  if (PinUsed(GPIO_P9813_CLK) && PinUsed(GPIO_P9813_DAT)) {  // RGB led
    strip = new NeoPixelBus<selectedNeoFeatureType, selectedNeoSpeedType>(Settings->light_pixels, Pin(GPIO_P9813_CLK), Pin(GPIO_P9813_DAT));
#else
  if (PinUsed(GPIO_WS2812)) {  // RGB led
    // For DMA, the Pin is ignored as it uses GPIO3 due to DMA hardware use.
    strip = new NeoPixelBus<selectedNeoFeatureType, selectedNeoSpeedType>(Settings->light_pixels, Pin(GPIO_WS2812));
#endif  // NEO_HW_P9813
    strip->Begin();

    Ws2812Clear();
    return true;
  }
  return false;
}

void Ws2812ModuleSelected(void)
{
  if (Ws2812InitStrip()) {
    Ws2812.scheme_offset = Light.max_scheme +1;
    Light.max_scheme += WS2812_SCHEMES;

#ifdef USE_NETWORK_LIGHT_SCHEMES
    Light.max_scheme++;
#endif

#if (USE_WS2812_CTYPE > NEO_3LED)
    TasmotaGlobal.light_type = LT_RGBW;
#else
    TasmotaGlobal.light_type = LT_RGB;
#endif
    TasmotaGlobal.light_driver = XLGT_01;
  }
}

#ifdef ESP32
#ifdef USE_BERRY
/********************************************************************************************/
// Callbacks for Berry driver
//
// Since we dont' want to export all the template stuff, we need to encapsulate the calls
// in plain functions
//
void *Ws2812GetStrip(void) {
  return strip;
}

void Ws2812Begin(void) {
  if (strip) { strip->Begin(); }
}

void Ws2812Show(void) {
  if (strip) { strip->Show(); }
}

uint32_t Ws2812PixelsSize(void) {
  if (strip) { return strip->PixelCount(); }
  return 0;
}

bool Ws2812CanShow(void) {
  if (strip) { return strip->CanShow(); }
  return false;
}

bool Ws2812IsDirty(void) {
  if (strip) { return strip->IsDirty(); }
  return false;
}

void Ws2812Dirty(void) {
  if (strip) { strip->Dirty(); }
}

uint8_t * Ws2812Pixels(void) {
  if (strip) { return strip->Pixels(); }
  return nullptr;
}

size_t Ws2812PixelSize(void) {
  if (strip) { return strip->PixelSize(); }
  return 0;
}

size_t Ws2812PixelCount(void) {
  if (strip) { return strip->PixelCount(); }
  return 0;
}

void Ws2812ClearTo(uint8_t r, uint8_t g, uint8_t b, uint8_t w, int32_t from, int32_t to) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor lcolor;
  lcolor.W = w;
#else
  RgbColor lcolor;
#endif

  lcolor.R = r;
  lcolor.G = g;
  lcolor.B = b;
  if (strip) {
    if (from < 0) {
      strip->ClearTo(lcolor);
    } else {
      strip->ClearTo(lcolor, from, to);
    }
  }
}

void Ws2812SetPixelColor(uint32_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor lcolor;
  lcolor.W = w;
#else
  RgbColor lcolor;
#endif

  lcolor.R = r;
  lcolor.G = g;
  lcolor.B = b;
  if (strip) {
    strip->SetPixelColor(idx, lcolor);
  }
}

uint32_t Ws2812GetPixelColor(uint32_t idx) {
#if (USE_WS2812_CTYPE > NEO_3LED)
  RgbwColor lcolor;
#else
  RgbColor lcolor;
#endif
  if (strip) {
    lcolor = strip->GetPixelColor(idx);
#if (USE_WS2812_CTYPE > NEO_3LED)
    return (lcolor.W << 24) | (lcolor.R << 16) | (lcolor.G << 8) | lcolor.B;
#else
    return (lcolor.R << 16) | (lcolor.G << 8) | lcolor.B;
#endif
  }
  return 0;
}

#endif  // ESP32
#endif  // USE_BERRY

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
/*
    Settings->light_pixels = XdrvMailbox.payload;
    Settings->light_rotation = 0;
    Ws2812ReinitStrip();   -- does not work with latest NeoPixelBus driver
    Light.update = true;
*/
    Ws2812Clear();                     // Clear all known pixels
    Settings->light_pixels = XdrvMailbox.payload;
    Settings->light_rotation = 0;
    TasmotaGlobal.restart_flag = 2;    // reboot instead
  }
  ResponseCmndNumber(Settings->light_pixels);
}

void CmndStepPixels(void)
{
  if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload <= 255)) {
    Settings->light_step_pixels = (XdrvMailbox.payload > WS2812_MAX_LEDS) ? WS2812_MAX_LEDS :  XdrvMailbox.payload;
    // Ws2812ReinitStrip();   -- not sure it's actually needed
    Light.update = true;
  }
  ResponseCmndNumber(Settings->light_step_pixels);
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

/*********************************************************************************************\
 * Internal calls for ArtNet
\*********************************************************************************************/
// check is the Neopixel strip is configured
bool Ws2812StripConfigured(void) {
  return strip != nullptr;
}
size_t Ws2812StripGetPixelSize(void) {
  return strip->PixelSize();
}
// return true if strip was dirty and an actual refresh was triggered
bool Ws2812StripRefresh(void) {
  if (strip->IsDirty()) {
    Ws2812LibStripShow();
    return true;
  } else {
    return false;
  }
}
void Ws2812CopyPixels(const uint8_t *buf, size_t len, size_t offset_in_matrix) {
  uint8_t *pixels = strip->Pixels();
  memmove(&pixels[offset_in_matrix], buf, len);
  strip->Dirty();
}


/* Dragon Command: index vs payload
 * 1 - enable segment X
 * 2 - disable segment X
 * 8 - sync color calculation offset (by setting to zero)
 * 15 - write byte/word value to MISAN.
 *      for value as 0xAAAABBCC
 *      AA: 0..MISAN_SIZE write 8 bit value CC to address AA
 *      AA: >8192 write 16 bit value, CC to address AA, BB to address AA+1
 * 16 - read byte/word value from MISAN, value = 0xAAAA0000 to specify address and 8/16 bit
 * 17 - read specific MISAN cooked values (for debug)
 * 18 - MISAN template
 * 19 - set MISAN partlist (clears template, so call this one first)
 * 
 * Tasmota Mailbox magic value for "no (numeric) parameter given" is -99
 * 
 * TODO
 * - test/verify MISAN Base64 write, and add paginated Base64 read
 * - implement Overlay effect in MISAN
 * - implement 'power toggle' based on other data (to use instead of full overwrite with DRAGON18)
 * - perhaps enable cmd to send MISAN as Base64 string?
 * - possibly enably dynamic add/remove of Misan effects? (needs memory management, data move, possibly moving of colortables)
 */
#ifdef USE_DERG_RGB
void CmndDragon(void) {
  switch(XdrvMailbox.index) {
    case 1: { // enable segment
      if (-99 != XdrvMailbox.payload) {
        struct drgn_misan_segment *seg = DragonFx_getSegmentById(XdrvMailbox.payload);
        if (seg != nullptr) {
          seg->disabled = false;
          ResponseCmndIdxNumber(seg->disabled ? 1 : 0);
        } else
          ResponseCmndIdxNumber(-1); // segment not found
      } else
        ResponseCmndIdxNumber(-2); // must give a segment index
      break;
    }
    case 2: { // disable segment
      if (-99 != XdrvMailbox.payload) {
        struct drgn_misan_segment *seg = DragonFx_getSegmentById(XdrvMailbox.payload);
        if (seg != nullptr) {
          seg->disabled = true;
          ResponseCmndIdxNumber(seg->disabled ? 1 : 0);
        } else
          ResponseCmndIdxNumber(-1); // segment not found
      } else
        ResponseCmndIdxNumber(-2); // must give a segment index
      break;
    }
    case 8: // sync color calculation offset (by setting to zero)
      if (-99 != XdrvMailbox.payload) {
//        dragonOffset_sync = ((uint64_t) millis()) - ((uint64_t) XdrvMailbox.payload);
//        dragonOffset_head = XdrvMailbox.payload;
        dragon_millis_offset = millis();
      }
      ResponseCmndIdxNumber(dragon_millis_offset);
      break;
    case 15: {
      misan_active_partlist = 0;
      misan_active_template = 0;
      if (-99 != XdrvMailbox.payload) {
        int16_t idx = ((uint32_t) XdrvMailbox.payload) >> 16;
        bool wide = idx >= 8192;
        if (wide) idx -= 8192;
        if (idx < (MISAN_SIZE - (wide ? 1 : 0))) {
          misan[idx] = XdrvMailbox.payload & 0xFF;
          if (wide) misan[idx+1] = (XdrvMailbox.payload >> 8) & 0xFF;
        }
        DragonFx_Misan_Initialize();
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
        DragonFx_Misan_Initialize();
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
      struct drgn_misan_segment *seg = DragonFx_getSegmentById(XdrvMailbox.payload >> 8);
      struct drgn_misan_blendtable *table = reinterpret_cast<struct drgn_misan_blendtable*> (&misan[XdrvMailbox.payload >> 8]);
      switch (XdrvMailbox.payload & 0xFF) {
        case 0: ResponseCmndIdxNumber(DragonFx_Misan_getPart(seg->partId)->start); break;
        case 1: ResponseCmndIdxNumber(DragonFx_Misan_getPart(seg->partId)->length); break;
        case 2: ResponseCmndIdxNumber(seg->type); break;
        case 3: ResponseCmndIdxNumber(seg->singlecolor.r); break;
        case 4: ResponseCmndIdxNumber(seg->singlecolor.g); break;
        case 5: ResponseCmndIdxNumber(seg->singlecolor.b); break;
        case 6: ResponseCmndIdxNumber(seg->linear.speed); break;
        case 7: ResponseCmndIdxNumber(seg->linear.px_offset); break;
        case 8: ResponseCmndIdxNumber(seg->linear.circle_speed); break;
        case 9: ResponseCmndIdxNumber(seg->linear.circle_offset); break;
        case 10: ResponseCmndIdxNumber(seg->linear.color_table_id); break;
        case 11: ResponseCmndIdxNumber(seg->linear.color_offset); break;
        case 12: ResponseCmndIdxNumber(seg->copy.boundary_mode); break;
        case 13: ResponseCmndIdxNumber(DragonFx_Misan_getPart(seg->copy.srcPartId)->start); break;
        case 14: ResponseCmndIdxNumber(DragonFx_Misan_getPart(seg->copy.srcPartId)->length); break;
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
        case 105: ResponseCmndIdxNumber(sizeof(drgn_misan_part)); break;
        case 106: ResponseCmndIdxNumber(sizeof(drgn_misan_parts)); break;
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
    case 19: // set MISAN partlist (clears template, so call this one first)
      if (-99 != XdrvMailbox.payload) {
        uint8_t partlistIdx = (uint8_t) XdrvMailbox.payload;
        SetMisanPartlist(partlistIdx);
      } 
      ResponseCmndIdxNumber(misan_active_partlist);
      break;
  }
}
#endif // USE_DERG_RGB


/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xlgt01(uint32_t function)
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
#endif  // defined(ESP8266) || defined(USE_WS2812_FORCE_NEOPIXELBUS)
