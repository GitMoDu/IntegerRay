# IntegerRay

C++11 Arduino-compatible library for creating efficient animations and driving RGB/RGBW addressable LED strips across multiple embedded platforms.

## Features

- Cooperative scheduling engine.
- HDR internal color model (15x15x15 bits color) for interpolation and effects.
- HDR color only converted to native strip color on last step.
- Native addressable LED drivers: WS2812B, SK6812.
  - AVR, STM32, RaspberryPiPico, etc... Bit-bang and SPI/DMA-based output drivers.


## Example

See [`examples/SimpleLight/SimpleLight.ino`](examples/SimpleLight/SimpleLight.ino) for a basic animated LED-strip setup.

