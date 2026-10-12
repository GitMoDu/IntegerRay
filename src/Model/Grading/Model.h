#ifndef _INTEGER_RAY_MODEL_GRADING_MODEL_h
#define _INTEGER_RAY_MODEL_GRADING_MODEL_h

#include "../ColorModel.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			struct Identity
			{
				static constexpr Internal::color_t Grade(const Internal::color_t color, const uint16_t)
				{
					return color;
				}
			};

			template<typename Pipeline>
			class ColorGraderAdapter : public Pipeline, public IColorGrader
			{
			public:
				ColorGraderAdapter()
					: IColorGrader()
					, Pipeline()
				{}

				Internal::color_t Grade(const Internal::color_t color, const uint16_t pixelIndex) const override
				{
					return Pipeline::Grade(color, pixelIndex);
				}
			};
		}
	}
}

#endif
