#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_LAMBDA_ANIMATOR_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_LAMBDA_ANIMATOR_h

#include "Model.h"

namespace IntegerRay
{
	namespace Animation
	{
		template<typename field_t>
		class LambdaAnimator
		{
		public:
			using StepCallback = void(*)(void* context, const uint32_t elapsed, const field_t value, const scalar_t progress);
			using LifecycleCallback = void(*)(void* context);

		private:
			enum class StateEnum
			{
				Off,
				WaitingForFirstStep,
				Animating
			};

		private:
			StepCallback StepCallbackFunction = nullptr;
			void* StepContext = nullptr;
			LifecycleCallback StartCallbackFunction = nullptr;
			void* StartContext = nullptr;
			LifecycleCallback EndCallbackFunction = nullptr;
			void* EndContext = nullptr;

		private:
			uint32_t StartTimestamp = 0;
			uint32_t Duration = 0;

			field_t StartValue{};
			field_t EndValue{};

			StateEnum State = StateEnum::Off;

			IAnimatorCurve* Curve;

		public:
			LambdaAnimator()
				: Curve(nullptr)
			{}
			LambdaAnimator(IAnimatorCurve* curve)
				: Curve(curve)
			{}

			LambdaAnimator(IAnimatorCurve& curve)
				: Curve(&curve)
			{}

			void SetDuration(const uint32_t duration)
			{
				Duration = duration;
			}

			void SetStartValue(const field_t startValue)
			{
				StartValue = startValue;
			}

			void SetEndValue(const field_t endValue)
			{
				EndValue = endValue;
			}

			void SetCurve(IAnimatorCurve& curve)
			{
				Curve = &curve;
			}

			void SetCurve(IAnimatorCurve* curve)
			{
				Curve = curve;
			}

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

			void Stop()
			{
				State = StateEnum::Off;
			}

			void Start(const uint32_t duration)
			{
				Stop();
				Duration = duration;
				State = StateEnum::WaitingForFirstStep;
			}

			void Start()
			{
				Stop();
				State = StateEnum::WaitingForFirstStep;
			}

			void Start(const field_t endValue, const field_t startValue = 0)
			{
				Stop();
				StartValue = startValue;
				EndValue = endValue;
				State = StateEnum::WaitingForFirstStep;
			}

			void Step(const uint32_t timestamp)
			{
				switch (State)
				{
				case StateEnum::WaitingForFirstStep:
					StartTimestamp = timestamp;
					State = StateEnum::Animating;
					OnStart();
					break;
				case StateEnum::Animating:
					if (Duration > 0)
					{
						const auto elapsed = timestamp - StartTimestamp;

						if (elapsed < Duration)
						{
							const auto progress = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::GetScalar<uint32_t>(
								elapsed,
								Duration);
							if (StepCallbackFunction != nullptr)
							{
								const auto curvedProgress = GetProgress(progress);
								const auto value = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::Interpolate(curvedProgress, StartValue, EndValue);

								StepCallbackFunction(StepContext, elapsed, value, progress);
							}
						}
						else
						{
							State = StateEnum::Off;
							OnEnd();
						}
					}
					else
					{
						State = StateEnum::Off;
						OnEnd();
					}
					break;
				case StateEnum::Off:
				default:
					break;
				}
			}

		private:
			scalar_t GetProgress(const scalar_t progress) const
			{
				if (Curve != nullptr)
				{
					return Curve->Get(progress);
				}
				else
				{
					return progress;
				}
			}

			void OnStart()
			{
				if (StartCallbackFunction != nullptr)
				{
					StartCallbackFunction(StartContext);
				}
				if (StepCallbackFunction != nullptr)
				{
					StepCallbackFunction(StepContext, 0, StartValue, 0);
				}
			}

			void OnEnd()
			{
				if (StepCallbackFunction != nullptr)
				{
					StepCallbackFunction(StepContext, Duration, EndValue, SCALAR_UNIT);
				}
				if (EndCallbackFunction != nullptr)
				{
					EndCallbackFunction(EndContext);
				}
			}
		};
	}
}

#endif
