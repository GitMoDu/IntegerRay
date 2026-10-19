#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_ANIMATOR_CORE_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_ANIMATOR_CORE_h

#include "Model.h"

namespace IntegerRay
{
	namespace Animation
	{
		template<typename field_t>
		class AnimatorCore
		{
		private:
			enum class StateEnum
			{
				Off,
				WaitingForFirstStep,
				Animating
			};

			uint32_t StartTimestamp = 0;
			uint32_t Duration = 0;

			field_t StartValue{};
			field_t EndValue{};

			StateEnum State = StateEnum::Off;
			bool Looping = false;

			IAnimatorCurve* Curve;
			IAnimatorLifecycleListener* LifecycleListener = nullptr;

			scalar_t GetProgress(const scalar_t progress) const
			{
				return Curve != nullptr ? Curve->Get(progress) : progress;
			}

			void OnStart()
			{
				OnAnimationStarted();
				if (LifecycleListener != nullptr)
				{
					LifecycleListener->OnAnimationStart();
				}
				OnValueChanged(0, StartValue, 0);
			}

			void OnEnd()
			{
				OnValueChanged(Duration, EndValue, SCALAR_UNIT);
				OnAnimationEnded();
				if (LifecycleListener != nullptr)
				{
					LifecycleListener->OnAnimationEnd();
				}
			}

		protected:
			virtual void OnValueChanged(const uint32_t elapsed, const field_t value, const scalar_t progress) = 0;

			virtual void OnAnimationStarted()
			{}

			virtual void OnAnimationEnded()
			{}

		public:
			AnimatorCore()
				: Curve(nullptr)
			{}

			AnimatorCore(IAnimatorCurve* curve)
				: Curve(curve)
			{}

			AnimatorCore(IAnimatorCurve& curve)
				: Curve(&curve)
			{}

			virtual ~AnimatorCore() = default;

			void SetDuration(const uint32_t duration)
			{
				Duration = duration;
			}

			void SetLooping(const bool looping)
			{
				Looping = looping;
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

			void SetLifecycleListener(IAnimatorLifecycleListener& listener)
			{
				LifecycleListener = &listener;
			}

			void SetLifecycleListener(IAnimatorLifecycleListener* listener)
			{
				LifecycleListener = listener;
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
							const auto curvedProgress = GetProgress(progress);
							const auto value = IntegerSignal::FixedPoint::ScalarFraction::UFraction32::Interpolate(curvedProgress, StartValue, EndValue);
							OnValueChanged(elapsed, value, progress);
						}
						else
						{
							State = Looping ? StateEnum::WaitingForFirstStep : StateEnum::Off;
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
		};
	}
}

#endif
