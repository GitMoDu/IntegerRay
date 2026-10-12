#ifndef _INTEGER_RAY_MODEL_GRADING_BRIGHTNESS_h
#define _INTEGER_RAY_MODEL_GRADING_BRIGHTNESS_h

#include "Model.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			template<typename Parent = Identity>
			class Brightness : public Parent
			{
			public:
				using gain_t = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t;
				static constexpr gain_t GAIN_UNIT = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::FRACTION_1X;

			private:
				gain_t Gain = GAIN_UNIT;

				static Internal::component_t ApplyGain(const Internal::component_t component, const gain_t gain)
				{
					return IntegerSignal::FixedPoint::ScalarFraction::UFraction16::Fraction(gain, component);
				}

			public:
				void SetGain(const gain_t gain)
				{
					Gain = IntegerSignal::MinValue(gain, GAIN_UNIT);
				}

				Internal::color_t Grade(const Internal::color_t input, const uint16_t pixelIndex) const
				{
					Internal::color_t color = Parent::Grade(input, pixelIndex);
					color.red = ApplyGain(color.red, Gain);
					color.green = ApplyGain(color.green, Gain);
					color.blue = ApplyGain(color.blue, Gain);
					return color;
				}
			};
		}
	}
}

#endif
