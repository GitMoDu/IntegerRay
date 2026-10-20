#ifndef _SIMPLEANIMATOR_h
#define _SIMPLEANIMATOR_h

#include <IntegerRay.h>

class SimpleAnimator : public IntegerRay::IRayDrawer
{
private:
	using hue_t = IntegerSignal::Trigonometry::angle_t;

	static constexpr IntegerRay::component_t Lightness = IntegerRay::COMPONENT_MAX / 3;
	// Oklch chroma is not bounded by one RGB component unit. A conservative
	// value reduces per-channel clipping across the full hue wheel.
	static constexpr IntegerRay::component_t Chroma = IntegerRay::COMPONENT_MAX / 12;
	static constexpr uint32_t HueRotationDurationMicros = 2000000;
	IntegerRay::Animation::TemplateAnimatedField<hue_t> HueOffset;

public:
	SimpleAnimator()
		: IntegerRay::IRayDrawer()
	{
		HueOffset.SetLooping(true);
		HueOffset.AnimateFromTo(HueRotationDurationMicros, 0, IntegerSignal::Trigonometry::ANGLE_RANGE);
	}

	~SimpleAnimator() = default;

	virtual bool OnDraw(IntegerRay::IRayBuffer& buffer, const uint32_t timestamp, const uint16_t /*frameCounter*/) override
	{
		HueOffset.Step(timestamp);

		const uint16_t count = buffer.GetCount();

		for (uint16_t index = 0; index < count; index++)
		{
			// Divide the full modular hue turn into evenly spaced positions, then rotate over time.
			const IntegerSignal::FixedPoint::ScalarFraction::ufraction16_t position =
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint32_t>(index, count);
			const IntegerSignal::Trigonometry::angle_t spatialHue =
				IntegerSignal::FixedPoint::ScalarFraction::Fraction(position, IntegerSignal::Trigonometry::ANGLE_RANGE);
			const IntegerSignal::Trigonometry::angle_t hue =
				IntegerSignal::Trigonometry::angle_t(HueOffset.Value + spatialHue);
			const IntegerRay::color_t color = IntegerSignal::RgbFraction16::ColorOklch(Lightness, Chroma, hue);
			buffer.Set(color, index);
		}

		return true; // All pixels animated in a single pass.
	}
};


#endif

