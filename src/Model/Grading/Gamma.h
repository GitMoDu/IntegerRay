#ifndef _INTEGER_RAY_MODEL_GRADING_GAMMA_h
#define _INTEGER_RAY_MODEL_GRADING_GAMMA_h

#include "Model.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Grading
		{
			template<typename Parent = Identity,
				typename Curve = IntegerSignal::Curves::Template::Curve<uint16_t>>
				class Gamma : public Parent
			{
			private:
				using component_t = Internal::component_t;
				using UFraction16 = IntegerSignal::FixedPoint::ScalarFraction::UFraction16;

			public:
				Internal::color_t Grade(const Internal::color_t input, const uint16_t pixelIndex) const
				{
					Internal::color_t color = Parent::Grade(input, pixelIndex);
					color.red = Apply(color.red);
					color.green = Apply(color.green);
					color.blue = Apply(color.blue);
					return color;
				}

			private:
				static constexpr component_t Apply(const component_t value)
				{
					return UFraction16::GetScalar(
						Curve::Get(
							UFraction16::Fraction(value, UINT16_MAX)
						), UINT16_MAX);
				}
			};

			template<typename Parent = Identity>
			using GammaRoot2 = Gamma<Parent, IntegerSignal::Curves::Root2U16<>>;

			template<typename Parent = Identity>
			using GammaPower2 = Gamma<Parent, IntegerSignal::Curves::Power2U16<>>;

			template<typename Parent = Identity>
			using GammaPower3 = Gamma<Parent, IntegerSignal::Curves::Power3U16<>>;
		}
	}
}

#endif
