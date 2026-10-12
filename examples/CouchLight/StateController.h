#ifndef _STATECONTROLLER_h
#define _STATECONTROLLER_h

#include "StripDrawer.h"
#include "ButtonParser.h"
#include "Model.h"

template<uint8_t ButtonPin>
class StateControllerTask : public TS::Task
{
private:
	static constexpr uint32_t ShortSleepMillis = CouchLight::ButtonUi::ShortPressThresholdMicros / 1000;
private:
	ButtonParser<CouchLight::ButtonUi::LongPressThresholdMicros, CouchLight::ButtonUi::ShortPressThresholdMicros> Parser{};

	CouchLight::LightStateEnum CurrentState = CouchLight::LightStateEnum::Off;

	StripDrawer& TopDrawer;
	StripDrawer& BottomDrawer;

	bool IsActive = false;

public:
	StateControllerTask(TS::Scheduler& scheduler,
		StripDrawer& topDrawer,
		StripDrawer& bottomDrawer)
		: TS::Task(ShortSleepMillis, TASK_FOREVER, &scheduler, false)
		, TopDrawer(topDrawer)
		, BottomDrawer(bottomDrawer)
	{}

	void Start()
	{
		IsActive = true;
		Parser.Clear();
		TS::Task::disable();

		// Set the button pin as an input with an internal pull-down resistor for active-high touch sensor.
		pinMode(ButtonPin, INPUT_PULLDOWN);

		// Start with both strips off until the button selects a lighting state.
		CurrentState = CouchLight::LightStateEnum::Off;
		TopDrawer.SetLightState(CurrentState);
		BottomDrawer.SetLightState(CurrentState);

//#if defined(DEBUG)
		pinMode(LED_BUILTIN, OUTPUT);
		digitalWrite(LED_BUILTIN, HIGH);
//#endif

		TS::Task::enable();
	}

	void Stop()
	{
		IsActive = false;
		TS::Task::disable();
	}

	void AdvanceLightState()
	{
		CurrentState = static_cast<CouchLight::LightStateEnum>((static_cast<uint8_t>(CurrentState) + 1) % static_cast<uint8_t>(CouchLight::LightStateEnum::EnumCount));

		if (CurrentState == CouchLight::LightStateEnum::Off)
		{
			CurrentState = CouchLight::LightStateEnum::ColoredDim;
		}

		TopDrawer.SetLightState(CurrentState);
		BottomDrawer.SetLightState(CurrentState);
	}

	virtual bool Callback() final
	{
		if (!IsActive)
		{
			TS::Task::disable();
			return false;
		}

		const bool pressed = digitalRead(ButtonPin) == HIGH;

//#if defined(DEBUG)
		// Show the button state on the built-in LED.
		digitalWrite(LED_BUILTIN, pressed ? HIGH : LOW);
//#endifS

		if (Parser.Parse(micros(), pressed))
		{
			if (Parser.HasShortEvent)
			{
				AdvanceLightState();
			}
			else if (Parser.HasLongEvent)
			{
				CurrentState = CouchLight::LightStateEnum::Off;
				TopDrawer.SetLightState(CurrentState);
				BottomDrawer.SetLightState(CurrentState);
			}

#if defined(DEBUG)
			Serial.print(F("New Light State: "));
			switch (CurrentState)
			{
			case CouchLight::LightStateEnum::Off:
				Serial.println(F("Off"));
				break;
			case CouchLight::LightStateEnum::ColoredDim:
				Serial.println(F("ColoredDim"));
				break;
			case CouchLight::LightStateEnum::ColoredBright:
				Serial.println(F("ColoredBright"));
				break;
			case CouchLight::LightStateEnum::WhiteLighting:
				Serial.println(F("WhiteLighting"));
				break;
			default:
				Serial.println(F("Unknown"));
				break;
			}
#endif
		}

		return true;
	}
};

#endif

