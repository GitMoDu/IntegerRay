/*
 * IntegerRay SimpleLight
 *
 * Minimal example for driving an RGB or RGBW SK6812-compatible LED strip
 * with a generated animation.
 *
 * Required hardware:
 * - A supported Arduino-compatible board.
 * - An RGB/RGBW SK6812-compatible strip or pixel chain.
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


#include <IntegerRay.h>
#include <IntegerRayDrivers.h>

#include "SimpleAnimator.h"

// LED strip configuration. Set this to the number of pixels in the chain.
static constexpr uint16_t LedCount = 15;

// Data output pin used by bit-bang drivers. DMA/SPI drivers use the selected
// SPI peripheral's MOSI pin instead.
static constexpr uint8_t LedPin = 3;

// Uncomment one of the following driver types to select the desired strip driver.
using DriverType = IntegerRay::Drivers::Ws2812b::Avr::BitBangRgbDriver<LedCount, LedPin>;
//using DriverType = IntegerRay::Drivers::Ws2812b::Avr::SpiRgbDriver<LedCount>;
//using DriverType = IntegerRay::Drivers::Sk6812::Avr::BitBangRgbwDriver<LedCount, LedPin>;
////using DriverType = IntegerRay::Drivers::Sk6812::Stm32::BitBangRgbwDriver<LedCount, LedPin>;
//using DriverType = IntegerRay::Drivers::Sk6812::Stm32::DmaRgbwDriver<LedCount>;

// Enable for a driver that transmits asynchronously through SPI/DMA.
// The selected driver must expose the corresponding SPI callback API.
//#define USE_SPI_ASYNC

// Some drivers accept constructor parameters and some require additional
// setup, such as SPI initialization or a transfer-completion callback.
DriverType StripDriver{};

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
inline void OnStripTransmitComplete();


void setup()
{
#if defined(DEBUG)
	// Serial output is optional and can be disabled by removing DEBUG above.
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;;
	delay(1000);
#endif

#if defined(USE_SPI_ASYNC)
	// Initialize the shared SPI peripheral before registering the completion
	// callback used by asynchronous DMA transfers.
	SPI.begin();
#endif

	// Initialize the selected strip driver before starting the render engine.
	if (!StripDriver.Setup())
	{
#if defined(DEBUG)
		Serial.println(F("StripDriver setup failed."));
#endif
		halt();
	}
#if defined(USE_SPI_ASYNC)
	SPI.onTransmit(OnStripTransmitComplete);
#endif

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

void OnStripTransmitComplete()
{
#if defined(USE_SPI_ASYNC)
	// DMA drivers clear their busy state here.
	StripDriver.HandleTransmitComplete();
#endif
}