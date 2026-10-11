/*
* Couch Light for shelfs.
*
* Target: STM32F103C8T6 (Maple Mini, Roger's Core)
*/


//#define DEBUG
//#define DEBUG_ANIMATION
//#define DEBUG_MOCK_BUTTON
#define SERIAL_BAUD_RATE 115200

// Using Archipenko's TaskScheduler library for Arduino.
#define _TASK_OO_CALLBACKS
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TScheduler.hpp>


#include <IntegerRay.h>
#include <IntegerRayDrivers.h>

#include "Device.h"
#include "Model.h"
#include "StripDrawer.h"
#include "StateController.h"


// Process scheduler.
TS::Scheduler SchedulerBase{};


// LED strip drivers.
Device::StripTop::DriverType StripTopDriver(Device::StripTop::Spi);
Device::StripBottom::DriverType StripBottomDriver(Device::StripBottom::Spi);


// Strip engines for the top and bottom LED strips, using the native color type and the number of LEDs in each strip.
IntegerRay::Engine::TemplateRayEngine<Device::StripTop::DriverType::color_t, Device::StripTop::LedCount> TopEngine(
	SchedulerBase,
	StripTopDriver,
	Device::Display::TargetPeriodMicros);
IntegerRay::Engine::TemplateRayEngine<Device::StripBottom::DriverType::color_t, Device::StripBottom::LedCount> BottomEngine(
	SchedulerBase,
	StripBottomDriver,
	Device::Display::TargetPeriodMicros);

// Strip drawers for the top and bottom LED strips.
StripDrawer TopDrawer{};
StripDrawer BottomDrawer{};

// State controller task for handling button input and controlling the light state of the top and bottom drawers.
StateControllerTask<Device::Button::Pin> StateController(SchedulerBase, TopDrawer, BottomDrawer);

#if defined(DEBUG_MOCK_BUTTON)
class MockButtonTask : public TS::Task
{
private:
	StateControllerTask<Device::Button::Pin>& Controller;

public:
	MockButtonTask(TS::Scheduler& scheduler, StateControllerTask<Device::Button::Pin>& controller)
		: TS::Task(5000, TASK_FOREVER, &scheduler, false)
		, Controller(controller)
	{}

	bool Callback() final
	{
		Serial.println(F("Mock short press"));
		Controller.SimulateShortPress();
		return true;
	}
};

MockButtonTask MockButton(SchedulerBase, StateController);
#endif


void setup()
{
#if defined(DEBUG) || defined(DEBUG_ANIMATION) || defined(DEBUG_MOCK_BUTTON)
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;;
	delay(1000);
	Serial.println();
	delay(1000);

	Serial.println(F("Serial On"));
	Serial.println(F("Couch light setup begin"));
#endif

	SPI.end();
	Device::StripBottom::Spi.onTransmit(OnStripBottomTransmitComplete);
	Device::StripTop::Spi.onTransmit(OnStripTopTransmitComplete);

	if (!StripBottomDriver.Setup()
		|| !StripTopDriver.Setup())
	{
#if defined(DEBUG)
		Serial.println(F("StripDriver setup failed."));
#endif
		halt();
	}

	if (!BottomEngine.Setup()
		|| !TopEngine.Setup())
	{
#if defined(DEBUG)
		Serial.println(F("RayEngine setup failed."));
#endif
		halt();
	}

	BottomEngine.Start(&BottomDrawer);
	TopEngine.Start(&TopDrawer);

	StateController.Start();

#if defined(DEBUG_MOCK_BUTTON)
	MockButton.enable();
#endif

#if defined(DEBUG)
	Serial.println(F("Couch light setup complete."));
#endif
}


void loop()
{
	SchedulerBase.execute();
}

void halt()
{
#if defined(DEBUG)
	Serial.println(F("Setup Failed."));
#endif
	while (true)
		;
}

void OnStripTopTransmitComplete()
{
	StripTopDriver.HandleTransmitComplete();
}

void OnStripBottomTransmitComplete()
{
	StripBottomDriver.HandleTransmitComplete();
}