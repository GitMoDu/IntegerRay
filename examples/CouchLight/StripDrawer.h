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
	IntegerRay::Animation::FieldAnimator<IntegerRay::component_t> AnimatorSaturation;
	IntegerRay::Animation::FieldAnimator<IntegerRay::component_t> AnimatorBrightness;
	IntegerRay::Animation::FieldAnimator<IntegerRay::component_t> AnimatorFilterEnd;
	IntegerRay::Animation::FieldAnimator<IntegerRay::component_t> AnimatorWhiteBlend;

public:
	StripDrawer()
		: IntegerRay::IRayDrawer()
		, AnimatorSaturation(SaturationValue)
		, AnimatorBrightness(BrightnessValue)
		, AnimatorFilterEnd(FilterEnd)
		, AnimatorWhiteBlend(WhiteBlend)
	{}


	~StripDrawer() = default;

	void SetLightState(const LightStateEnum lightState)
	{
		if (LightState != lightState)
		{
			const bool wasOff = (LightState == LightStateEnum::Off);

			AnimatorSaturation.Stop();
			AnimatorBrightness.Stop();
			AnimatorWhiteBlend.Stop();

			switch (lightState)
			{
			case LightStateEnum::ColoredDim:
				AnimatorSaturation.AnimateTo(CouchLight::AnimationToColoredDim::Duration, CouchLight::AnimationToColoredDim::SaturationValue);
				AnimatorBrightness.AnimateTo(CouchLight::AnimationToColoredDim::Duration, CouchLight::AnimationToColoredDim::BrightnessValue);
				AnimatorWhiteBlend.AnimateTo(CouchLight::AnimationToColoredDim::Duration, 0);
				break;
			case LightStateEnum::ColoredBright:
				AnimatorSaturation.AnimateTo(CouchLight::AnimationToColoredBright::Duration, CouchLight::AnimationToColoredBright::SaturationValue);
				AnimatorBrightness.AnimateTo(CouchLight::AnimationToColoredBright::Duration, CouchLight::AnimationToColoredBright::BrightnessValue);
				AnimatorWhiteBlend.AnimateTo(CouchLight::AnimationToColoredBright::Duration, 0);
				break;
			case LightStateEnum::WhiteLighting:
				AnimatorSaturation.AnimateTo(CouchLight::AnimationToWhiteLighting::Duration, CouchLight::AnimationToWhiteLighting::SaturationValue);
				AnimatorBrightness.AnimateTo(CouchLight::AnimationToWhiteLighting::Duration, CouchLight::AnimationToWhiteLighting::BrightnessValue);
				AnimatorWhiteBlend.AnimateTo(CouchLight::AnimationToWhiteLighting::Duration, IntegerRay::ColorModel::Internal::COMPONENT_MAX);
				break;
			case LightStateEnum::Off:
			default:
				AnimatorFilterEnd.Stop();
				AnimatorSaturation.AnimateTo(CouchLight::AnimationToOff::Duration, CouchLight::AnimationToOff::SaturationValue);
				AnimatorBrightness.AnimateTo(CouchLight::AnimationToOff::Duration, CouchLight::AnimationToOff::BrightnessValue);
				AnimatorWhiteBlend.AnimateTo(CouchLight::AnimationToOff::Duration, 0);
				AnimatorFilterEnd.AnimateTo(CouchLight::AnimationToOff::Duration, 0);
				break;
			}

			if (wasOff && lightState != LightStateEnum::Off)
			{
				AnimatorFilterEnd.Stop();
				AnimatorFilterEnd.AnimateTo(CouchLight::AnimationToOn::Duration, IntegerRay::ColorModel::Internal::COMPONENT_MAX);
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
	IntegerRay::component_t GetFilterAt(const uint16_t index, const uint16_t count)
	{
		const IntegerSignal::FixedPoint::ScalarFraction::ufraction16_t indexProgress = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint32_t>(
			index, count - 1);

		return IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Interpolate(indexProgress, IntegerRay::ColorModel::Internal::COMPONENT_MAX, FilterEnd);
	}
};


#endif

