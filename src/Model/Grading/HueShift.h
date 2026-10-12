#ifndef _INTEGER_RAY_MODEL_GRADING_HUE_SHIFT_h
#define _INTEGER_RAY_MODEL_GRADING_HUE_SHIFT_h

#include "Model.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			template<typename Parent = Identity>
			class HueShift : public Parent
			{
			private:
				IntegerSignal::Trigonometry::angle_t Offset = 0;

			public:
				void SetOffset(const IntegerSignal::Trigonometry::angle_t offset)
				{
					Offset = offset;
				}

				Internal::color_t Grade(const Internal::color_t input, const uint16_t pixelIndex) const
				{
					const Internal::color_t color = Parent::Grade(input, pixelIndex);
					const uint32_t hueRange = uint32_t(IntegerSignal::Trigonometry::ANGLE_RANGE);
					const uint32_t normalizedOffset = uint32_t(Offset) % hueRange;
					if (normalizedOffset == 0)
					{
						return color;
					}

					const auto maximum = IntegerSignal::MaxValue(color.red, IntegerSignal::MaxValue(color.green, color.blue));
					const auto minimum = IntegerSignal::MinValue(color.red, IntegerSignal::MinValue(color.green, color.blue));
					const auto delta = maximum - minimum;

					if (delta == 0)
					{
						return color;
					}

					const int32_t signedHueRange = int32_t(hueRange);
					const int32_t sixthRange = signedHueRange / 6;
					int32_t hue;

					if (maximum == color.red)
					{
						hue = (int32_t(color.green) - int32_t(color.blue)) * sixthRange / int32_t(delta);
						if (hue < 0)
						{
							hue += signedHueRange;
						}
					}
					else if (maximum == color.green)
					{
						hue = 2 * sixthRange + (int32_t(color.blue) - int32_t(color.red)) * sixthRange / int32_t(delta);
					}
					else
					{
						hue = 4 * sixthRange + (int32_t(color.red) - int32_t(color.green)) * sixthRange / int32_t(delta);
					}

					const auto saturation = IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint32_t>(delta, maximum);
					const auto shiftedHue = IntegerSignal::Trigonometry::angle_t((uint32_t(hue) + normalizedOffset) % hueRange);
					return IntegerSignal::RgbFraction16::ColorHsv(shiftedHue, saturation, maximum);
				}
			};
		}
	}
}

#endif
