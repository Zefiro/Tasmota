![Tasmota logo](/tools/logo/TASMOTA_FullLogo_Vector.svg)

THIS IS A FORK - go look at the official repo for all details.

This adds a new compile environment 'tasmota-dragon-neopixels' for a different way of driving Neopixel strips.

If you've got an RGBW strip, add this line in tasmota/user_config_override.h
#define USE_WS2812_CTYPE     NEO_GRBW 

It also adds an environment 'tasmota-dragon-sensors' which is WIP. The idea is to minimize self-heating of certain sensor boards by slower polling and switching them off inbetween.