# IntegerRay

IntegerRay provides strip animation, color grading, and output-format conversion. It is a pure software library and has no dependency on hardware-driver libraries. Concrete strip transports are provided by separate libraries.

## Driver adapters

Inertia is the primary option for strip drivers. Include `IntegerRayInertia.h` to use IntegerRay with Inertia strip drivers. 
Include `IntegerRayAdafruit.h` for Adafruit_NeoPixel-compatible drivers; 

The adapter is templated on the pixel object and explicit channel indices, so the selected IntegerRay output format and the pixel object's configured RGB/RGBW order must agree. Each integration header is independent and includes only its own wrapped library.

IntegerRay's existing hardware driver headers remain available during this transition; the adapters are opt-in.

C++11 Arduino-compatible library for creating efficient animations and driving RGB/RGBW addressable LED strips across multiple embedded platforms.

<!-- Placeholder: README reviewed before adding adapter usage documentation. -->

## Features

- Cooperative scheduling engine.
- HDR internal color model (15x15x15 bits color) for interpolation and effects.
- HDR color only converted to native strip color on last step.
- Native addressable LED drivers: WS2812B, SK6812.
  - AVR, STM32, RaspberryPiPico, etc... Bit-bang and SPI/DMA-based output drivers.


## Example

See [`examples/SimpleLight/SimpleLight.ino`](examples/SimpleLight/SimpleLight.ino) for a basic animated LED-strip setup.

