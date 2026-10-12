#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_MODEL_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_MODEL_h

#include "../../Model/Interface.h"

namespace IntegerRay
{
	namespace Animation
	{
		using scalar_t = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::scalar_t;

		static constexpr scalar_t SCALAR_UNIT = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::SCALAR_UNIT;

		using IAnimatorCurve = IntegerSignal::Curves::ICurve<scalar_t>;

		template<typename field_t>
		class Animator;

		template<typename field_t>
		struct IAnimatorUpdateListener
		{
			virtual ~IAnimatorUpdateListener() = default;

			virtual void OnAnimationUpdate(Animator<field_t>& animator, const uint32_t elapsed, const field_t value, const scalar_t progress) = 0;
		};

		struct IAnimatorLifecycleListener
		{
			virtual ~IAnimatorLifecycleListener() = default;

			virtual void OnAnimationStart() = 0;
			virtual void OnAnimationEnd() = 0;
		};
	}
}

#endif
