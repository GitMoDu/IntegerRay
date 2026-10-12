#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_LAMBDA_ANIMATOR_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_LAMBDA_ANIMATOR_h

#include "AnimatorCore.h"

namespace IntegerRay
{
	namespace Animation
	{
		template<typename field_t>
		class LambdaAnimator : public AnimatorCore<field_t>
		{
		private:
			using Base = AnimatorCore<field_t>;
		public:
			using StepCallback = void(*)(void* context, const uint32_t elapsed, const field_t value, const scalar_t progress);
			using LifecycleCallback = void(*)(void* context);

		private:
			StepCallback StepCallbackFunction = nullptr;
			void* StepContext = nullptr;
			LifecycleCallback StartCallbackFunction = nullptr;
			void* StartContext = nullptr;
			LifecycleCallback EndCallbackFunction = nullptr;
			void* EndContext = nullptr;

		protected:
			void OnValueChanged(const uint32_t elapsed, const field_t value, const scalar_t progress) override
			{
				if (StepCallbackFunction != nullptr)
				{
					StepCallbackFunction(StepContext, elapsed, value, progress);
				}
			}

			void OnAnimationStarted() override
			{
				if (StartCallbackFunction != nullptr)
				{
					StartCallbackFunction(StartContext);
				}
			}

			void OnAnimationEnded() override
			{
				if (EndCallbackFunction != nullptr)
				{
					EndCallbackFunction(EndContext);
				}
			}

		public:
			LambdaAnimator()
				: Base()
			{}

			LambdaAnimator(IAnimatorCurve* curve)
				: Base(curve)
			{}

			LambdaAnimator(IAnimatorCurve& curve)
				: Base(curve)
			{}

			void SetStepCallback(void* context, StepCallback callback)
			{
				StepContext = context;
				StepCallbackFunction = callback;
			}

			void SetStartCallback(void* context, LifecycleCallback callback)
			{
				StartContext = context;
				StartCallbackFunction = callback;
			}

			void SetEndCallback(void* context, LifecycleCallback callback)
			{
				EndContext = context;
				EndCallbackFunction = callback;
			}

		};
	}
}

#endif
