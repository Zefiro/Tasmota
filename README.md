![Tasmota logo](/tools/logo/TASMOTA_FullLogo_Vector.svg)

THIS IS A FORK - go look at the official repo for all details.

This adds a new compile environment 'tasmota-dragon-neopixels' for a different way of driving Neopixel strips.

If you've got an RGBW strip, add this line in tasmota/user_config_override.h
#define USE_WS2812_CTYPE     NEO_GRBW 

It also adds an environment 'tasmota-dragon-sensors' which is WIP. The idea is to minimize self-heating of certain sensor boards by slower polling and switching them off inbetween.

------
# Configuration of MISAN
* part list: the led strip can be split into different parts, which are referenced in effects by their index number. They can overlap.
* Each effect is of a specific type, and needs a variable length of configuration
* the number of effects is internally counted, stopping with effect type zero as the end marker


