#ifndef _INTEGER_RAY_MODEL_GRADING_CONTRAST_h
#define _INTEGER_RAY_MODEL_GRADING_CONTRAST_h

#include "Model.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			template<typename Parent = Identity>
			class Contrast : public Parent
			{
			public:
				using gain_t = IntegerSignal::FixedPoint::FactorScale::Scale16::factor_t;
				static constexpr gain_t GAIN_UNIT = IntegerSignal::FixedPoint::FactorScale::Scale16::SCALE_1X;

			private:
				gain_t Gain = GAIN_UNIT;

				static Internal::component_t ApplyGain(const Internal::component_t component, const gain_t gain)
				{
					static constexpr Internal::component_t Midpoint = Internal::COMPONENT_MAX / 2;
					if (component >= Midpoint)
					{
						const Internal::component_t distance = component - Midpoint;
						const Internal::component_t scaledDistance = IntegerSignal::FixedPoint::FactorScale::Scale(gain, distance);
						const Internal::component_t upperRange = Internal::COMPONENT_MAX - Midpoint;
						return Midpoint + IntegerSignal::MinValue(scaledDistance, upperRange);
					}

					const Internal::component_t distance = Midpoint - component;
					const Internal::component_t scaledDistance = IntegerSignal::FixedPoint::FactorScale::Scale(gain, distance);
					return Midpoint - IntegerSignal::MinValue(scaledDistance, Midpoint);
				}

			public:
				void SetGain(const gain_t gain)
				{
					Gain = gain;
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
