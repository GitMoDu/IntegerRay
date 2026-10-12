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
				typename Curve = IntegerSignal::Curves::Template::Curve<uint16_t>,
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t DefaultStrength =
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::FRACTION_1X>
			class Gamma : public Parent
			{
			private:
				using component_t = Internal::component_t;
				using UFraction16 = IntegerSignal::FixedPoint::ScalarFraction::UFraction16;
				using strength_t = typename UFraction16::scalar_t;

				strength_t Strength = DefaultStrength;

			public:
				static constexpr strength_t STRENGTH_UNIT = UFraction16::FRACTION_1X;

				void SetStrength(const strength_t strength)
				{
					Strength = IntegerSignal::MinValue(strength, STRENGTH_UNIT);
				}

				Internal::color_t Grade(const Internal::color_t input, const uint16_t pixelIndex) const
				{
					Internal::color_t color = Parent::Grade(input, pixelIndex);
					color.red = Apply(color.red, Strength);
					color.green = Apply(color.green, Strength);
					color.blue = Apply(color.blue, Strength);
					return color;
				}

			private:
				static constexpr component_t Apply(const component_t value, const strength_t strength)
				{
					return UFraction16::Interpolate(strength, value,
						UFraction16::GetScalar<uint16_t>(
							Curve::Get(UFraction16::Fraction<uint16_t>(value, UINT16_MAX)), UINT16_MAX));
				}
			};

			template<typename Parent = Identity,
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t DefaultStrength =
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::FRACTION_1X>
			using GammaRoot2 = Gamma<Parent, IntegerSignal::Curves::Root2U16<>, DefaultStrength>;

			template<typename Parent = Identity,
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t DefaultStrength =
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::FRACTION_1X>
			using GammaPower2 = Gamma<Parent, IntegerSignal::Curves::Power2U16<>, DefaultStrength>;

			template<typename Parent = Identity,
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::scalar_t DefaultStrength =
				IntegerSignal::FixedPoint::ScalarFraction::UFraction16::FRACTION_1X>
			using GammaPower3 = Gamma<Parent, IntegerSignal::Curves::Power3U16<>, DefaultStrength>;
		}
	}
}

#endif
