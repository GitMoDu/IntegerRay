/*
 * IntegerRay SimpleLight
 *
   * Minimal example for driving an RGB or RGBW addressable LED strip
 * with a generated animation through an Inertia or Adafruit NeoPixel adapter.
 *
 * Required hardware:
 * - A supported Arduino-compatible board.
   * - An RGB/RGBW strip or pixel chain supported by the selected driver.
 * - A suitable external power supply for the strip.
 * - A shared ground between the board and the strip power supply.
 * - A short data connection from LedPin to the strip data input.
 * - A 3.3V-to-5V level shifter when the strip does not reliably recognize
 *   the board's logic-high voltage on 3.3V boards.
 *
 * Add the recommended data-line resistor and power-supply bypass capacitor
 * for the strip and wiring length used in the project. Do not power a large
 * strip through the board's regulator or USB connection.
 */

#define DEBUG
#define SERIAL_BAUD_RATE 115200

 // TaskScheduler provides the cooperative animation scheduler used by the engine.
#define _TASK_OO_CALLBACKS
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TScheduler.hpp>


// Select exactly one IntegerRay driver adapter and output format.
#define SIMPLELIGHT_ADAPTER_ADAFRUIT_RGB
//#define SIMPLELIGHT_ADAPTER_ADAFRUIT_RGBW
//#define SIMPLELIGHT_ADAPTER_INERTIA_RGB
//#define SIMPLELIGHT_ADAPTER_INERTIA_RGBW

#if (defined(SIMPLELIGHT_ADAPTER_ADAFRUIT_RGB) + defined(SIMPLELIGHT_ADAPTER_ADAFRUIT_RGBW) \
	+ defined(SIMPLELIGHT_ADAPTER_INERTIA_RGB) + defined(SIMPLELIGHT_ADAPTER_INERTIA_RGBW)) != 1
#error Select exactly one SimpleLight adapter option.
#endif

#if defined(SIMPLELIGHT_ADAPTER_ADAFRUIT_RGB) || defined(SIMPLELIGHT_ADAPTER_ADAFRUIT_RGBW)
#include <Adafruit_NeoPixel.h>
#include <IntegerRayAdafruit.h>
#else
#include <IntegerRayInertia.h>
#endif

#include "SimpleAnimator.h"

// LED strip configuration. Set this to the number of pixels in the chain.
static constexpr uint16_t LedCount = 15;

// Data output pin used by bit-bang drivers. DMA/SPI drivers use the selected
// SPI peripheral's MOSI pin instead.
static constexpr uint8_t LedPin = 11;

#if defined(SIMPLELIGHT_ADAPTER_ADAFRUIT_RGB)
using DriverType = IntegerRay::Adapters::Adafruit::NeoPixelDriver<
	Adafruit_NeoPixel, IntegerRay::ColorModel::Output::Grb, LedCount, 1, 0, 2>;
Adafruit_NeoPixel HardwareStrip(LedCount, LedPin, NEO_GRB + NEO_KHZ800);
DriverType StripDriver(HardwareStrip);
#elif defined(SIMPLELIGHT_ADAPTER_ADAFRUIT_RGBW)
using DriverType = IntegerRay::Adapters::Adafruit::NeoPixelDriver<
	Adafruit_NeoPixel, IntegerRay::ColorModel::Output::Grbw, LedCount, 1, 0, 2, 3>;
Adafruit_NeoPixel HardwareStrip(LedCount, LedPin, NEO_GRBW + NEO_KHZ800);
DriverType StripDriver(HardwareStrip);
#elif defined(SIMPLELIGHT_ADAPTER_INERTIA_RGB)
using HardwareDriverType = Inertia::Drivers::Light::LedStrip::Ws2812b::Avr::BitBangDriver<LedPin, LedCount>;
using DriverType = IntegerRay::Adapters::Inertia::StripDriver<HardwareDriverType,
	IntegerRay::Adapters::Inertia::IntegerRayFormat::Rgb,
	Inertia::Drivers::Light::LedStrip::Ws2812b::OutputFormat::Grb,
	LedCount>;
HardwareDriverType HardwareStrip{};
DriverType StripDriver(HardwareStrip);
#elif defined(SIMPLELIGHT_ADAPTER_INERTIA_RGBW)
using HardwareDriverType = Inertia::Drivers::Light::LedStrip::Sk6812::Avr::BitBangRgbwDriver<LedPin, LedCount>;
using DriverType = IntegerRay::Adapters::Inertia::StripDriver<HardwareDriverType,
	IntegerRay::Adapters::Inertia::IntegerRayFormat::Rgbw,
	IntegerRay::Adapters::Inertia::OutputFormat::Rgbw,
	LedCount>;
HardwareDriverType HardwareStrip{};
DriverType StripDriver(HardwareStrip);
#endif

// Native color type determines the channel count and color layout used by the
// animation engine. RGBW drivers allocate four channels per pixel.
using strip_color_t = DriverType::color_t;

// Target animation update period. 10,000 microseconds is a 100 Hz target.
static constexpr uint32_t TargetPeriodMicros = 10000;

// Cooperative scheduler used by the animation engine.
TS::Scheduler SchedulerBase{};


// Coordinates animation updates, rendering, and strip-buffer submission.
IntegerRay::Engine::TemplateRayEngine<strip_color_t, LedCount> RayEngine(
	SchedulerBase,
	StripDriver,
	TargetPeriodMicros);

SimpleAnimator Animator{};

inline void halt();


void setup()
{
#if defined(DEBUG)
	// Serial output is optional and can be disabled by removing DEBUG above.
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;;
	delay(1000);
#endif

	// Initialize the selected strip driver before starting the render engine.
	if (!StripDriver.Setup())
	{
#if defined(DEBUG)
		Serial.println(F("StripDriver setup failed."));
#endif
		halt();
	}

	// Initialize the engine and attach the animation implementation.
	if (!RayEngine.Setup())
	{
#if defined(DEBUG)
		Serial.println(F("RayEngine setup failed."));
#endif
		halt();
	}

	RayEngine.Start(&Animator);

}


void loop()
{
	// The scheduler runs due tasks and returns; do not block this loop.
	SchedulerBase.execute();
}

void halt()
{
#if defined(DEBUG)
	Serial.println(F("Setup Failed."));
#endif
	while (true)
	{
		;
	}
}

