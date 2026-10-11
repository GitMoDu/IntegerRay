#ifndef _STRIP_DRAWER_h
#define _STRIP_DRAWER_h

#include <IntegerRay.h>
#include "Model.h"


class StripDrawer : public IntegerRay::IRayDrawer
{
private:
	using LightStateEnum = CouchLight::LightStateEnum;


private:
	IntegerRay::component_t SaturationValue = 0;
	IntegerRay::component_t BrightnessValue = 0;
	IntegerRay::component_t FilterEnd = 0;
	IntegerRay::component_t WhiteBlend = 0;
	LightStateEnum LightState = LightStateEnum::Off;

#if defined(DEBUG_ANIMATION)
	uint32_t DebugTransitionTimestamp = 0;
	uint32_t DebugLastLogTimestamp = 0;
#endif

	IntegerRay::Animation::LambdaAnimator<IntegerRay::component_t> AnimatorSaturation{};
	IntegerRay::Animation::LambdaAnimator<IntegerRay::component_t> AnimatorBrightness{};
	IntegerRay::Animation::LambdaAnimator<IntegerRay::component_t> AnimatorFilterEnd{};
	IntegerRay::Animation::LambdaAnimator<IntegerRay::component_t> AnimatorWhiteBlend{};

public:
	StripDrawer()
		: IntegerRay::IRayDrawer()
	{
		SetupAnimators();
	}


	~StripDrawer() = default;

	void SetLightState(const LightStateEnum lightState)
	{
		if (LightState != lightState)
		{
#if defined(DEBUG_ANIMATION)
			DebugTransitionTimestamp = micros();
			DebugLastLogTimestamp = 0;
			Serial.print(F("Strip state="));
			Serial.print(static_cast<uint8_t>(lightState));
			Serial.print(F(" start(s,b,f,w)="));
			Serial.print(SaturationValue);
			Serial.print(',');
			Serial.print(BrightnessValue);
			Serial.print(',');
			Serial.print(FilterEnd);
			Serial.print(',');
			Serial.println(WhiteBlend);
#endif
			const bool wasOff = (LightState == LightStateEnum::Off);

			AnimatorSaturation.Stop();
			AnimatorBrightness.Stop();
			AnimatorWhiteBlend.Stop();

			switch (lightState)
			{
			case LightStateEnum::ColoredDim:
				AnimatorSaturation.SetDuration(CouchLight::AnimationToColoredDim::Duration);
				AnimatorBrightness.SetDuration(CouchLight::AnimationToColoredDim::Duration);
				AnimatorWhiteBlend.SetDuration(CouchLight::AnimationToColoredDim::Duration);

				AnimatorSaturation.Start(CouchLight::AnimationToColoredDim::SaturationValue, SaturationValue);
				AnimatorBrightness.Start(CouchLight::AnimationToColoredDim::BrightnessValue, BrightnessValue);
				AnimatorWhiteBlend.Start(0, WhiteBlend);
				break;
			case LightStateEnum::ColoredBright:
				AnimatorSaturation.SetDuration(CouchLight::AnimationToColoredBright::Duration);
				AnimatorBrightness.SetDuration(CouchLight::AnimationToColoredBright::Duration);
				AnimatorWhiteBlend.SetDuration(CouchLight::AnimationToColoredBright::Duration);

				AnimatorSaturation.Start(CouchLight::AnimationToColoredBright::SaturationValue, SaturationValue);
				AnimatorBrightness.Start(CouchLight::AnimationToColoredBright::BrightnessValue, BrightnessValue);
				AnimatorWhiteBlend.Start(0, WhiteBlend);
				break;
			case LightStateEnum::WhiteLighting:
				AnimatorSaturation.SetDuration(CouchLight::AnimationToWhiteLighting::Duration);
				AnimatorBrightness.SetDuration(CouchLight::AnimationToWhiteLighting::Duration);
				AnimatorWhiteBlend.SetDuration(CouchLight::AnimationToWhiteLighting::Duration);

				AnimatorSaturation.Start(CouchLight::AnimationToWhiteLighting::SaturationValue, SaturationValue);
				AnimatorBrightness.Start(CouchLight::AnimationToWhiteLighting::BrightnessValue, BrightnessValue);
				AnimatorWhiteBlend.Start(IntegerRay::ColorModel::Internal::COMPONENT_MAX, WhiteBlend);
				break;
			case LightStateEnum::Off:
			default:
				AnimatorFilterEnd.Stop();
				AnimatorSaturation.SetDuration(CouchLight::AnimationToOff::Duration);
				AnimatorBrightness.SetDuration(CouchLight::AnimationToOff::Duration);
				AnimatorWhiteBlend.SetDuration(CouchLight::AnimationToOff::Duration);
				AnimatorFilterEnd.SetDuration(CouchLight::AnimationToOff::Duration);
				AnimatorSaturation.Start(CouchLight::AnimationToOff::SaturationValue, SaturationValue);
				AnimatorBrightness.Start(CouchLight::AnimationToOff::BrightnessValue, BrightnessValue);
				AnimatorWhiteBlend.Start(0, WhiteBlend);
				AnimatorFilterEnd.Start(0, FilterEnd);
				break;
			}

			if (wasOff && lightState != LightStateEnum::Off)
			{
				AnimatorFilterEnd.Stop();
				AnimatorFilterEnd.SetDuration(CouchLight::AnimationToOn::Duration);
				AnimatorFilterEnd.Start(IntegerRay::ColorModel::Internal::COMPONENT_MAX, FilterEnd);
			}

			LightState = lightState;
		}
	}

	virtual bool OnDraw(IntegerRay::IRayBuffer& buffer, const uint32_t timestamp, const uint16_t /*frameCounter*/) override
	{
		AnimatorFilterEnd.Step(timestamp);
		AnimatorWhiteBlend.Step(timestamp);
		AnimatorSaturation.Step(timestamp);
		AnimatorBrightness.Step(timestamp);

		const IntegerSignal::FixedPoint::ScalarFraction::ufraction32_t progress = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::GetScalar<uint32_t>(
			(timestamp % CouchLight::AnimationHueRotate::Duration),
			CouchLight::AnimationHueRotate::Duration);
		const IntegerSignal::Trigonometry::angle_t hueOffset = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::Fraction(progress, IntegerSignal::Trigonometry::ANGLE_RANGE);

		const uint16_t count = buffer.GetCount();

#if defined(DEBUG_ANIMATION)
		if ((timestamp - DebugTransitionTimestamp) <= CouchLight::AnimationToOn::Duration
			&& (DebugLastLogTimestamp == 0 || (timestamp - DebugLastLogTimestamp) >= 100000))
		{
			DebugLastLogTimestamp = timestamp;
			Serial.print(F("Strip t="));
			Serial.print(timestamp - DebugTransitionTimestamp);
			Serial.print(F(" (s,b,f,w)="));
			Serial.print(SaturationValue);
			Serial.print(',');
			Serial.print(BrightnessValue);
			Serial.print(',');
			Serial.print(FilterEnd);
			Serial.print(',');
			Serial.print(WhiteBlend);

			const IntegerSignal::Trigonometry::angle_t p0Hue = hueOffset;
			IntegerRay::color_t p0Color = IntegerSignal::RgbFraction16::ColorHsv(p0Hue, SaturationValue, BrightnessValue);
			const IntegerRay::component_t p0Filter = GetFilterAt(0, count);
			p0Color.red = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(p0Filter, p0Color.red);
			p0Color.green = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(p0Filter, p0Color.green);
			p0Color.blue = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(p0Filter, p0Color.blue);
			if (WhiteBlend > 0)
			{
				p0Color = IntegerSignal::RgbFraction16::ColorInterpolate(p0Color, CouchLight::AnimationToWhiteLighting::ColorValue, WhiteBlend);
			}
			const uint32_t grbw = IntegerRay::ColorModel::Output::Grbw::Pack(p0Color);
			Serial.print(F(" p0(G,R,B,W)="));
			Serial.print(uint8_t(grbw >> 24));
			Serial.print(',');
			Serial.print(uint8_t(grbw >> 16));
			Serial.print(',');
			Serial.print(uint8_t(grbw >> 8));
			Serial.print(',');
			Serial.println(uint8_t(grbw));
		}
#endif
		for (uint16_t index = 0; index < count; ++index)
		{
			const IntegerSignal::FixedPoint::ScalarFraction::ufraction16_t indexProgress = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint32_t>(
				index, count - 1);

			const IntegerSignal::Trigonometry::angle_t hue = hueOffset + IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(indexProgress, CouchLight::AnimationHueRotate::HueShiftRange);
			IntegerRay::color_t color = IntegerSignal::RgbFraction16::ColorHsv(hue, SaturationValue, BrightnessValue);

			const IntegerRay::component_t filter = GetFilterAt(index, count);
			color.red = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(filter, color.red);
			color.green = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(filter, color.green);
			color.blue = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(filter, color.blue);

			if (WhiteBlend > 0)
			{
				color = IntegerSignal::RgbFraction16::ColorInterpolate(color, CouchLight::AnimationToWhiteLighting::ColorValue, WhiteBlend);
			}

			buffer.Set(color, index);
		}

		return true; // All pixels animated in a single pass.
	}

private:
	void SetupAnimators()
	{
		AnimatorSaturation.SetStepCallback(this,
			[](void* context, const uint32_t /*elapsed*/, const IntegerRay::component_t value, const IntegerRay::Animation::scalar_t /*progress*/)
			{
				static_cast<StripDrawer*>(context)->SaturationValue = value;
			}
		);
		AnimatorBrightness.SetStepCallback(this,
			[](void* context, const uint32_t /*elapsed*/, const IntegerRay::component_t value, const IntegerRay::Animation::scalar_t /*progress*/)
			{
				static_cast<StripDrawer*>(context)->BrightnessValue = value;
			}
		);
		AnimatorFilterEnd.SetStepCallback(this,
			[](void* context, const uint32_t /*elapsed*/, const IntegerRay::component_t value, const IntegerRay::Animation::scalar_t /*progress*/)
			{
				static_cast<StripDrawer*>(context)->FilterEnd = value;
			}
		);
		AnimatorWhiteBlend.SetStepCallback(this,
			[](void* context, const uint32_t /*elapsed*/, const IntegerRay::component_t value, const IntegerRay::Animation::scalar_t /*progress*/)
			{
				static_cast<StripDrawer*>(context)->WhiteBlend = value;
			}
		);
	}

	IntegerRay::component_t GetFilterAt(const uint16_t index, const uint16_t count)
	{
		const IntegerSignal::FixedPoint::ScalarFraction::ufraction16_t indexProgress = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint32_t>(
			index, count - 1);

		return IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Interpolate(indexProgress, IntegerRay::ColorModel::Internal::COMPONENT_MAX, FilterEnd);
	}
};


#endif

