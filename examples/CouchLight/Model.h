#ifndef _MODEL_h
#define _MODEL_h

#include <IntegerRay.h>

namespace CouchLight
{
	enum class LightStateEnum
	{
		Off,
		ColoredDim,
		ColoredBright,
		WhiteLighting,
		EnumCount
	};

	namespace ButtonUi
	{
		static constexpr uint32_t LongPressThresholdMicros = 1000000;
		static constexpr uint32_t ShortPressThresholdMicros = 5000;
	}

	namespace AnimationHueRotate
	{
		static constexpr uint32_t Duration = 1000000 * 35;
		static constexpr IntegerSignal::Trigonometry::angle_t HueShiftRange = IntegerSignal::Trigonometry::GetAngle(190);
	}

	namespace AnimationToColoredDim
	{
		static constexpr uint32_t Duration = 1000000 * 1;

		static constexpr IntegerRay::component_t SaturationValue = (uint32_t(IntegerRay::ColorModel::Internal::COMPONENT_MAX) * 10) / 10;
		static constexpr IntegerRay::component_t BrightnessValue = IntegerRay::ColorModel::Internal::COMPONENT_MAX / 64;
	}

	namespace AnimationToColoredBright
	{
		static constexpr uint32_t Duration = 1000000 * 1;

		static constexpr IntegerRay::component_t SaturationValue = (uint32_t(IntegerRay::ColorModel::Internal::COMPONENT_MAX) * 9) / 10;
		static constexpr IntegerRay::component_t BrightnessValue = IntegerRay::ColorModel::Internal::COMPONENT_MAX / 8;
	}

	namespace AnimationToWhiteLighting
	{
		static constexpr uint32_t Duration = 1000000 * 2;

		// Hue is set to a warm color for white lighting.
		static constexpr IntegerSignal::Trigonometry::angle_t Hue = IntegerSignal::Trigonometry::GetAngle(6);

		// Saturation biases light towards color hue.
		static constexpr IntegerRay::component_t SaturationValue = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint32_t>(65, 100);

		// Brightness is limited due to power supply and heat dissipation constraints of the LED strips.
		static constexpr IntegerRay::component_t BrightnessValue = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint32_t>(50, 100);

		static IntegerRay::color_t ColorValue = IntegerSignal::RgbFraction16::ColorHsv(
			Hue,
			SaturationValue,
			BrightnessValue
		);
	};


	namespace AnimationToOn
	{
		static constexpr uint32_t Duration = 3500000;
		static constexpr uint32_t OnDuration = 500000;
		static constexpr uint32_t OffDuration = 150000;
	};

	namespace AnimationToOff
	{
		static constexpr uint32_t Duration = 1000000 * 3;
		static constexpr uint32_t OffDuration = 500000;

		static constexpr IntegerRay::component_t SaturationValue = AnimationToColoredDim::SaturationValue;
		static constexpr IntegerRay::component_t BrightnessValue = 0;
	};
}
#endif