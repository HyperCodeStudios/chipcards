#include <Arduino.h>
#include <NeoPixelBus.h>
#include "neopixel.hpp"

#define GPIO_NEOPIXEL GPIO_NUM_2

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> pixelBus(1, GPIO_NEOPIXEL);

void neopixel_setup() 
{
    pixelBus.Begin();
}

void neopixel_green()
{
    pixelBus.SetPixelColor(0, RgbColor(0,255,0));
    pixelBus.Show();
}

void neopixel_red()
{
    pixelBus.SetPixelColor(0, RgbColor(255,0,0));
    pixelBus.Show();
}

void neopixel_white()
{
    pixelBus.SetPixelColor(0, RgbColor(255,255,255));
    pixelBus.Show();
}

void neopixel_off()
{
    pixelBus.SetPixelColor(0, RgbColor(0,0,0));
    pixelBus.Show();
}
