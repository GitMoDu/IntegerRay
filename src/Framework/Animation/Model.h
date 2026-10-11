#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_MODEL_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_MODEL_h

#include "../../Model/Interface.h"

namespace IntegerRay
{
	namespace Animation
	{
		using scalar_t = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::scalar_t;
		using IAnimatorCurve = IntegerSignal::Curves::ICurve<scalar_t>;

		static constexpr scalar_t SCALAR_UNIT = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::SCALAR_UNIT;
	}
}

#endif
