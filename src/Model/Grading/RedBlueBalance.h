#ifndef _INTEGER_RAY_MODEL_GRADING_RED_BLUE_BALANCE_h
#define _INTEGER_RAY_MODEL_GRADING_RED_BLUE_BALANCE_h

#include "Model.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			template<typename Parent = Identity>
			class RedBlueBalance : public Parent
			{
			public:
				using balance_t = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::scalar_t;
				using gain_t = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t;
				static constexpr balance_t BALANCE_UNIT = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::FRACTION_1X;
				static constexpr balance_t BALANCE_UNIT_NEGATIVE = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::FRACTION_1X_NEGATIVE;
				static constexpr gain_t GAIN_UNIT = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::FRACTION_1X;

			private:
				balance_t Balance = 0;

				static Internal::component_t Increase(const Internal::component_t component, const gain_t amount)
				{
					return IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Interpolate(
						amount, component, Internal::COMPONENT_MAX);
				}

				static Internal::component_t Decrease(const Internal::component_t component, const gain_t amount)
				{
					return IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Interpolate(
						amount, component, Internal::component_t(0));
				}

				static gain_t ToGain(const balance_t balance)
				{
					const int32_t magnitude = balance < 0 ? -int32_t(balance) : int32_t(balance);
					return gain_t(uint32_t(magnitude) << 1);
				}

			public:
				void SetBalance(const balance_t balance)
				{
					Balance = IntegerSignal::MaxValue(BALANCE_UNIT_NEGATIVE,
						IntegerSignal::MinValue(balance, BALANCE_UNIT));
				}

				Internal::color_t Grade(const Internal::color_t input, const uint16_t pixelIndex) const
				{
					Internal::color_t color = Parent::Grade(input, pixelIndex);
					const int32_t balance = Balance;
					if (balance > 0)
					{
						const gain_t amount = ToGain(Balance);
						color.red = Increase(color.red, amount);
						color.blue = Decrease(color.blue, amount);
					}
					else if (balance < 0)
					{
						const gain_t amount = ToGain(Balance);
						color.red = Decrease(color.red, amount);
						color.blue = Increase(color.blue, amount);
					}
					return color;
				}
			};
		}
	}
}

#endif
