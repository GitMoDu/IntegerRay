#ifndef _INTEGER_RAY_MODEL_GRADING_COLOR_TEMPERATURE_h
#define _INTEGER_RAY_MODEL_GRADING_COLOR_TEMPERATURE_h

#include "WhiteBalance.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			/// <summary>Applies a normalized warm/cool shift through the RGB white-balance stage.</summary>
			/// <typeparam name="Parent">The preceding color grading stage.</typeparam>
			/// <remarks>This is a relative color shift, not a physical color-temperature conversion.</remarks>
			template<typename Parent = Identity>
			class ColorTemperature : public WhiteBalance<Parent>
			{
			private:
				using WhiteBalanceType = WhiteBalance<Parent>;
				using balance_t = typename WhiteBalanceType::balance_t;

				using WhiteBalanceType::SetBalance;
				using WhiteBalanceType::SetRedBalance;
				using WhiteBalanceType::SetGreenBalance;
				using WhiteBalanceType::SetBlueBalance;


			public:
				using temperature_t = balance_t;
				static constexpr temperature_t TEMPERATURE_UNIT = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::FRACTION_1X;
				static constexpr temperature_t TEMPERATURE_UNIT_NEGATIVE = IntegerSignal::FixedPoint::ScalarFraction::Fraction16::FRACTION_1X_NEGATIVE;
				static constexpr balance_t MAXIMUM_BALANCE = balance_t(TEMPERATURE_UNIT / 4);

			public:
				void SetTemperature(const temperature_t temperature)
				{
					using Fraction16 = IntegerSignal::FixedPoint::ScalarFraction::Fraction16;
					const temperature_t bounded = LimitTemperature(temperature);
					const balance_t shift = Fraction16::Fraction(bounded, MAXIMUM_BALANCE);
					WhiteBalanceType::SetBalance(shift, balance_t(0), balance_t(-shift));
				}

			private:
				static balance_t LimitTemperature(const balance_t temperature)
				{
					return IntegerSignal::MaxValue(TEMPERATURE_UNIT_NEGATIVE,
						IntegerSignal::MinValue(temperature, TEMPERATURE_UNIT));
				}
			};
		}
	}
}

#endif
