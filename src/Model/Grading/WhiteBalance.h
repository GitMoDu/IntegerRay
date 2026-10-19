#ifndef _INTEGER_RAY_MODEL_GRADING_WHITE_BALANCE_h
#define _INTEGER_RAY_MODEL_GRADING_WHITE_BALANCE_h

#include "Model.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			/// <summary>Applies independent unity-centered RGB gains before output conversion.</summary>
			/// <typeparam name="Parent">The preceding color grading stage.</typeparam>
			/// <remarks>Each balance is in [-1, 1]: -1 is 0x, zero is unity, and +1 is 2x; boosted components saturate at their maximum.</remarks>
			template<typename Parent = Identity>
			class WhiteBalance : public Parent
			{
			public:
				using balance_t = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::scalar_t;
				static constexpr balance_t BALANCE_UNIT = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::FRACTION_1X;
				static constexpr balance_t BALANCE_UNIT_NEGATIVE = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::FRACTION_1X_NEGATIVE;

			private:
				balance_t RedBalance = 0;
				balance_t GreenBalance = 0;
				balance_t BlueBalance = 0;

			public:
				void SetRedBalance(const balance_t balance)
				{
					RedBalance = LimitBalance(balance);
				}

				void SetGreenBalance(const balance_t balance)
				{
					GreenBalance = LimitBalance(balance);
				}

				void SetBlueBalance(const balance_t balance)
				{
					BlueBalance = LimitBalance(balance);
				}

				void SetBalance(const balance_t red, const balance_t green, const balance_t blue)
				{
					SetRedBalance(red);
					SetGreenBalance(green);
					SetBlueBalance(blue);
				}

				Internal::color_t Grade(const Internal::color_t input, const uint16_t pixelIndex) const
				{
					Internal::color_t color = Parent::Grade(input, pixelIndex);
					color.red = ApplyBalance(color.red, RedBalance);
					color.green = ApplyBalance(color.green, GreenBalance);
					color.blue = ApplyBalance(color.blue, BlueBalance);
					return color;
				}

			private:
				static balance_t LimitBalance(const balance_t balance)
				{
					return IntegerSignal::MaxValue(BALANCE_UNIT_NEGATIVE,
						IntegerSignal::MinValue(balance, BALANCE_UNIT));
				}

				static Internal::component_t ApplyBalance(const Internal::component_t component, const balance_t balance)
				{
					using Fraction16 = IntegerSignal::FixedPoint::ScalarFraction::Fraction16;
					const int32_t value = int32_t(component);
					const int32_t scaled = value + Fraction16::Fraction(balance, value);
					const int32_t bounded = IntegerSignal::MaxValue(int32_t(0),
						IntegerSignal::MinValue(scaled, int32_t(Internal::COMPONENT_MAX)));
					return Internal::component_t(bounded);
				}
			};
		}
	}
}

#endif
