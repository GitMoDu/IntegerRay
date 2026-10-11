#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_CURVES_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_CURVES_h

#include "Model.h"

namespace IntegerRay
{
	namespace Animation
	{
		/// <summary>
		/// Animation progress response curves for animating fields.
		/// </summary>
		namespace Curves
		{
			// Linear curve for animating.
			//using Linear = IntegerSignal::Curves::Template::Curve<scalar_t>;
			struct Linear : public IAnimatorCurve
			{
				Linear() : IAnimatorCurve() {}

				scalar_t Get(const scalar_t input) const final { return input; }
			};

			// No curve is the same as linear, alias provided for clarity.
			using None = Linear;

			// Power^2 curve for animating.
			struct Power2 final : public IAnimatorCurve
			{
				Power2() : IAnimatorCurve() {}
				virtual ~Power2() = default;

				virtual scalar_t Get(const scalar_t input) const final
				{
					return IntegerSignal::Curves::Power2U32<>::Get(input);
				}
			};

			// Power^3 curve for animating.
			struct Power3 final : public IAnimatorCurve
			{
				Power3() : IAnimatorCurve() {}
				virtual ~Power3() = default;
				virtual scalar_t Get(const scalar_t input) const final
				{
					return IntegerSignal::Curves::Power3U32<>::Get(input);
				}
			};

			// Square root curve for animating.
			struct Root2 final : public IAnimatorCurve
			{
				Root2() : IAnimatorCurve() {}
				virtual ~Root2() = default;
				virtual scalar_t Get(const scalar_t input) const final
				{
					return IntegerSignal::Curves::Root2U32<>::Get(input);
				}
			};

			struct Gamma final : public IAnimatorCurve
			{
				Gamma() : IAnimatorCurve() {}
				virtual ~Gamma() = default;

				virtual scalar_t Get(const scalar_t input) const final
				{
					return 0;//TODO: Implement Gamma curve
				}
			};

		}
	}
}

#endif
