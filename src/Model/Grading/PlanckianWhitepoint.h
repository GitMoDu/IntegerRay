#ifndef _INTEGER_RAY_MODEL_GRADING_PLANCKIAN_WHITEPOINT_h
#define _INTEGER_RAY_MODEL_GRADING_PLANCKIAN_WHITEPOINT_h

#include "Model.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			/// <summary>Applies a whitepoint shift along the Planckian locus using linear sRGB channel gains.</summary>
			/// <typeparam name="Parent">The preceding color grading stage.</typeparam>
			/// <remarks>The locus is an analytic approximation; the resulting gains assume standard sRGB primaries.</remarks>
			template<typename Parent = Identity>
			class PlanckianWhitepoint : public Parent
			{
			public:
				using gain_t = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t;
				using fraction_t = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::scalar_t;
				static constexpr gain_t GAIN_UNIT = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::FRACTION_1X;
				static constexpr uint16_t MINIMUM_TEMPERATURE_KELVIN = Whitepoint::Planckian::MINIMUM_TEMPERATURE_KELVIN;
				static constexpr uint16_t NEUTRAL_TEMPERATURE_KELVIN = Whitepoint::Planckian::NEUTRAL_TEMPERATURE_KELVIN;
				static constexpr uint16_t MAXIMUM_TEMPERATURE_KELVIN = Whitepoint::Planckian::MAXIMUM_TEMPERATURE_KELVIN;
				static constexpr int16_t MINIMUM_SHIFT_PERCENT = -100;
				static constexpr int16_t MAXIMUM_SHIFT_PERCENT = 100;
				static constexpr uint32_t MiredScale = 1000000000UL;

			private:
				gain_t RedGain = GAIN_UNIT;
				gain_t GreenGain = GAIN_UNIT;
				gain_t BlueGain = GAIN_UNIT;
				uint16_t TemperatureKelvin = NEUTRAL_TEMPERATURE_KELVIN;

				using UFraction16 = IntegerSignal::FixedPoint::ScalarFraction::UFraction16;

				static uint32_t RelativeChannel(const gain_t target, const gain_t neutral)
				{
					return (uint32_t(target) * uint32_t(GAIN_UNIT)) / neutral;
				}

				static uint8_t ToPercent(const gain_t gain)
				{
					return uint8_t((uint32_t(gain) * 100UL + uint32_t(GAIN_UNIT) / 2)
						/ uint32_t(GAIN_UNIT));
				}

			public:
				static uint16_t TemperatureForShift(const int16_t shiftPercent)
				{
					const int16_t bounded = IntegerSignal::MaxValue(MINIMUM_SHIFT_PERCENT,
						IntegerSignal::MinValue(shiftPercent, MAXIMUM_SHIFT_PERCENT));
					const uint8_t amount = uint8_t(IntegerSignal::AbsValue(bounded));
					const gain_t fraction = UFraction16::GetScalar<uint8_t>(amount, uint8_t(100));
					const uint32_t neutralMired = MiredScale / NEUTRAL_TEMPERATURE_KELVIN;
					const uint32_t endpointMired = MiredScale
						/ (bounded < 0 ? MAXIMUM_TEMPERATURE_KELVIN : MINIMUM_TEMPERATURE_KELVIN);
					const uint32_t mired = UFraction16::Interpolate(fraction, neutralMired, endpointMired);
					return uint16_t((MiredScale + mired / 2) / mired);
				}

				void SetShiftPercent(const int16_t shiftPercent)
				{
					SetTemperatureKelvin(TemperatureForShift(shiftPercent));
				}

				void SetTemperatureKelvin(const uint16_t temperature)
				{
					TemperatureKelvin = IntegerSignal::MaxValue(MINIMUM_TEMPERATURE_KELVIN,
						IntegerSignal::MinValue(temperature, MAXIMUM_TEMPERATURE_KELVIN));

					gain_t neutralRed;
					gain_t neutralGreen;
					gain_t neutralBlue;
					gain_t targetRed;
					gain_t targetGreen;
					gain_t targetBlue;
					Whitepoint::Planckian::GetLinearRgb(NEUTRAL_TEMPERATURE_KELVIN, neutralRed, neutralGreen, neutralBlue);
					Whitepoint::Planckian::GetLinearRgb(TemperatureKelvin, targetRed, targetGreen, targetBlue);

					const uint32_t redRatio = RelativeChannel(targetRed, neutralRed);
					const uint32_t greenRatio = RelativeChannel(targetGreen, neutralGreen);
					const uint32_t blueRatio = RelativeChannel(targetBlue, neutralBlue);
					const uint32_t maximum = IntegerSignal::MaxValue(redRatio,
						IntegerSignal::MaxValue(greenRatio, blueRatio));
					using UFraction16 = IntegerSignal::FixedPoint::ScalarFraction::UFraction16;
					RedGain = UFraction16::GetScalar<uint32_t>(redRatio, maximum);
					GreenGain = UFraction16::GetScalar<uint32_t>(greenRatio, maximum);
					BlueGain = UFraction16::GetScalar<uint32_t>(blueRatio, maximum);
				}

				uint16_t GetTemperatureKelvin() const
				{
					return TemperatureKelvin;
				}

				uint8_t GetRedGainPercent() const
				{
					return ToPercent(RedGain);
				}

				uint8_t GetGreenGainPercent() const
				{
					return ToPercent(GreenGain);
				}

				uint8_t GetBlueGainPercent() const
				{
					return ToPercent(BlueGain);
				}

				Internal::color_t Grade(const Internal::color_t input, const uint16_t pixelIndex) const
				{
					Internal::color_t color = Parent::Grade(input, pixelIndex);
					using UFraction16 = IntegerSignal::FixedPoint::ScalarFraction::UFraction16;
					color.red = UFraction16::Fraction(RedGain, color.red);
					color.green = UFraction16::Fraction(GreenGain, color.green);
					color.blue = UFraction16::Fraction(BlueGain, color.blue);
					return color;
				}
			};
		}
	}
}

#endif
