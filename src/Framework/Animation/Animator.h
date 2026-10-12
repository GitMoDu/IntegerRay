#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_ANIMATOR_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_ANIMATOR_h

#include "AnimatorCore.h"

namespace IntegerRay
{
	namespace Animation
	{
		template<typename field_t>
		class Animator : public AnimatorCore<field_t>
		{
		private:
			using Base = AnimatorCore<field_t>;
			IAnimatorUpdateListener<field_t>* UpdateListener = nullptr;

		protected:
			void OnValueChanged(const uint32_t elapsed, const field_t value, const scalar_t progress) override
			{
				if (UpdateListener != nullptr)
				{
					UpdateListener->OnAnimationUpdate(*this, elapsed, value, progress);
				}
			}

		public:
			Animator()
				: Base()
			{}

			Animator(IAnimatorCurve* curve)
				: Base(curve)
			{}

			Animator(IAnimatorCurve& curve)
				: Base(curve)
			{}

			void SetUpdateListener(IAnimatorUpdateListener<field_t>& listener)
			{
				UpdateListener = &listener;
			}

			void SetUpdateListener(IAnimatorUpdateListener<field_t>* listener)
			{
				UpdateListener = listener;
			}
		};
	}
}

#endif
