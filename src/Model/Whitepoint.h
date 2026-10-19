#ifndef _INTEGER_RAY_MODEL_WHITEPOINT_h
#define _INTEGER_RAY_MODEL_WHITEPOINT_h

#include <IntegerSignal.h>

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Whitepoint
		{
			namespace Temperature
			{
				static constexpr uint16_t WARM_WHITE_2700K = 2700;
				static constexpr uint16_t WARM_WHITE_3000K = 3000;
				static constexpr uint16_t NEUTRAL_WHITE_4000K = 4000;
				static constexpr uint16_t NEUTRAL_WHITE_4500K = 4500;
				static constexpr uint16_t DAYLIGHT_5000K = 5000;
				static constexpr uint16_t COOL_WHITE_6500K = 6500;
				static constexpr uint16_t DEFAULT_KELVIN = NEUTRAL_WHITE_4500K;
			}

			namespace Planckian
			{
				using gain_t = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t;
				using fraction_t = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::scalar_t;
				using UFraction16 = IntegerSignal::FixedPoint::ScalarFraction::UFraction16;
				using Fraction16 = IntegerSignal::FixedPoint::ScalarFraction::Fraction16;

				static constexpr uint16_t MINIMUM_TEMPERATURE_KELVIN = 2700;
				static constexpr uint16_t NEUTRAL_TEMPERATURE_KELVIN = 6500;
				static constexpr uint16_t MAXIMUM_TEMPERATURE_KELVIN = 10000;

				static constexpr fraction_t Coefficient(const int32_t numerator, const int32_t denominator)
				{
					return Fraction16::GetScalar(numerator, denominator);
				}

				static int32_t Apply(const fraction_t fraction, const int32_t value)
				{
					return Fraction16::Fraction(fraction, value);
				}

				static void GetPlanckianXy(const uint16_t temperature, int32_t& x, int32_t& y)
				{
					const uint16_t reciprocalTemperature = UFraction16::GetScalar<uint16_t>(uint16_t(1000), temperature);
					const int32_t v = int32_t(reciprocalTemperature >> 1);
					const int32_t vSquared = Apply(fraction_t(v), v);
					const int32_t vCubed = Apply(fraction_t(v), vSquared);

					if (temperature <= 4000)
					{
						x = Apply(-Coefficient(2661239, 10000000), vCubed)
							- Apply(Coefficient(2343580, 10000000), vSquared)
							+ Apply(Coefficient(8776956, 10000000), v)
							+ Coefficient(17991, 100000);
					}
					else
					{
						x = -3 * vCubed
							- Apply(Coefficient(258469, 10000000), vCubed)
							+ 2 * vSquared
							+ Apply(Coefficient(1070379, 10000000), vSquared)
							+ Apply(Coefficient(2226347, 10000000), v)
							+ Coefficient(24039, 100000);
					}

					const int32_t xSquared = Apply(fraction_t(x), x);
					const int32_t xCubed = Apply(fraction_t(x), xSquared);
					if (temperature < 2222)
					{
						y = -xCubed - Apply(Coefficient(1063814, 10000000), xCubed)
							- xSquared - Apply(Coefficient(3481102, 10000000), xSquared)
							+ 2 * x + Apply(Coefficient(1855583, 10000000), x)
							- Coefficient(20219683, 100000000);
					}
					else if (temperature <= 4000)
					{
						y = -Apply(Coefficient(9549476, 10000000), xCubed)
							- xSquared - Apply(Coefficient(3741859, 10000000), xSquared)
							+ 2 * x + Apply(Coefficient(9137015, 100000000), x)
							- Coefficient(16748867, 100000000);
					}
					else
					{
						y = 3 * xCubed + Apply(Coefficient(817580, 10000000), xCubed)
							- 5 * xSquared - Apply(Coefficient(8733867, 10000000), xSquared)
							+ 3 * x + Apply(Coefficient(75112997, 100000000), x)
							- Coefficient(37001483, 100000000);
					}
				}

				static int32_t Positive(const int32_t value)
				{
					return IntegerSignal::MaxValue(int32_t(0), value);
				}

				static void GetLinearRgb(const uint16_t temperature, gain_t& red, gain_t& green, gain_t& blue)
				{
					const uint16_t boundedTemperature = IntegerSignal::MaxValue(MINIMUM_TEMPERATURE_KELVIN,
						IntegerSignal::MinValue(temperature, MAXIMUM_TEMPERATURE_KELVIN));
					int32_t x;
					int32_t y;
					GetPlanckianXy(boundedTemperature, x, y);

					const int32_t z = Fraction16::FRACTION_1X - x - y;
					const int32_t linearRed = Positive(3 * x + Apply(Coefficient(2406, 10000), x)
						- y - Apply(Coefficient(5372, 10000), y) - Apply(Coefficient(4986, 10000), z));
					const int32_t linearGreen = Positive(-Apply(Coefficient(9689, 10000), x)
						+ y + Apply(Coefficient(8758, 10000), y) + Apply(Coefficient(415, 10000), z));
					const int32_t linearBlue = Positive(Apply(Coefficient(557, 10000), x)
						- Apply(Coefficient(2040, 10000), y) + z + Apply(Coefficient(570, 10000), z));
					const int32_t maximum = IntegerSignal::MaxValue(linearRed,
						IntegerSignal::MaxValue(linearGreen, linearBlue));
					red = UFraction16::GetScalar<uint32_t>(uint32_t(linearRed), uint32_t(maximum));
					green = UFraction16::GetScalar<uint32_t>(uint32_t(linearGreen), uint32_t(maximum));
					blue = UFraction16::GetScalar<uint32_t>(uint32_t(linearBlue), uint32_t(maximum));
				}

				static inline IntegerSignal::RgbFraction16::color_t GetLinearRgb(const uint16_t temperature)
				{
					gain_t red;
					gain_t green;
					gain_t blue;
					GetLinearRgb(temperature, red, green, blue);
					return { red, green, blue };
				}
			}
		}
	}
}

#endif
