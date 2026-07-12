#ifndef _INTEGER_RAY_ENGINE_RAY_ENGINE_h
#define _INTEGER_RAY_ENGINE_RAY_ENGINE_h


#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "../Model/Interface.h"
#include "RayBuffer.h"

namespace IntegerRay
{
	namespace Engine
	{
		/// <summary>
		/// Template-based engine for driving LED strips with a specified color format and count.
		/// </summary>
		/// <typeparam name="format_t">The color format type.</typeparam>
		/// <typeparam name="Count">The number of LEDs in the strip.</typeparam>
		template<typename format_t,
			uint16_t Count>
		class TemplateRayEngine : private TS::Task
		{
		public:
			static constexpr size_t NativeBufferSize = size_t(Count) * sizeof(ColorModel::Output::component_t) * format_t::CHANNEL_COUNT;

		private:
			enum class StateEnum
			{
				Initializing,	// Clears all buffers and sets all lights to off/black.
				BufferClear,	// Clears current drawing buffer to black/off. Placed between driver push and wait, to take advantage of asynchronous driver push to clear buffer.
				BufferRender,	// Draws the current buffer to the lights.
				BufferSwap,		// Converts and pushes the current buffer to native color buffer for the driver to push to the lights.
				SyncPush,		// Sync the display to the target frame rate.
				Disabled		// Engine is disabled, no animation or buffer updates will occur.
			};

			// Internal buffer for the engine to draw to.
			TemplateRayBuffer<Count> DrawBuffer{};

			// Output native color buffer for the driver to push to the lights.
			ColorModel::Output::component_t NativeBuffer[NativeBufferSize]{};

			uint32_t LastSync = 0;
			uint32_t AnimationTimestamp = 0;

			// Current drawer for the engine to use for drawing to the buffer.
			IRayDrawer* Drawer = nullptr;

			// Current state of the engine.
			StateEnum State = StateEnum::Disabled;
			uint16_t FrameCounter = 0;

			IStripDriver& Driver;
			uint32_t TargetDuration;

		public:
			TemplateRayEngine(TS::Scheduler& scheduler,
				IStripDriver& driver, const uint32_t targetPeriodMicros = 10000)
				: TS::Task(TASK_IMMEDIATE, TASK_FOREVER, &scheduler, false)
				, Driver(driver)
				, TargetDuration(targetPeriodMicros)
			{}

			~TemplateRayEngine() = default;

			bool Setup()
			{
				if (DrawBuffer.GetCount() != Driver.GetCount())
				{
					return false;
				}

				return true;
			}

			void SetDrawer(IRayDrawer* drawer)
			{
				Drawer = drawer;
			}

			void SetTargetPeriod(const uint32_t targetPeriodMicros)
			{
				TargetDuration = targetPeriodMicros;
			}

			void Start(IRayDrawer* drawer)
			{
				State = StateEnum::Initializing;
				Drawer = drawer;
				TS::Task::enableDelayed(0);
			}

			void Start()
			{
				State = StateEnum::Initializing;
				TS::Task::enableDelayed(0);
			}

			void Stop()
			{
				State = StateEnum::Disabled;
				TS::Task::disable();
			}

		public:
			virtual bool Callback() final
			{
				switch (State)
				{
				case StateEnum::Initializing:
					if (DrawBuffer.GetCount() == Driver.GetCount()
						&& DrawBuffer.GetCount() == Count)
					{
						memset(NativeBuffer, 0, sizeof(NativeBuffer));
						Driver.PushBuffer(NativeBuffer, NativeBufferSize);
						FrameCounter = 0;
						State = StateEnum::BufferClear;
						TS::Task::enableDelayed(0);
						LastSync = micros() - TargetDuration;
					}
					else
					{
						State = StateEnum::Disabled;
					}
					break;
				case StateEnum::BufferClear:
					// Frame start, clear the draw buffer to black/off before starting the animation.
					DrawBuffer.Clear();
					State = StateEnum::BufferRender;
					AnimationTimestamp = micros();
					TS::Task::delay(0);
					break;
				case StateEnum::BufferRender:
					if (Drawer != nullptr)
					{
						if (Drawer->OnDraw(DrawBuffer, AnimationTimestamp, FrameCounter))
						{
							// Drawing completed, move to next state.
							State = StateEnum::BufferSwap;
						}
					}
					else
					{
						// No drawer set, move to next state.
						State = StateEnum::BufferSwap;
					}
					TS::Task::delay(0);
					break;
				case StateEnum::BufferSwap:
					// Convert the animation results in the DrawBuffer to the native color buffer for the driver to push to the lights.
					ConvertBuffer<format_t>(DrawBuffer.Buffer, NativeBuffer, Count);
					State = StateEnum::SyncPush;
					TS::Task::delay(0);
					break;
				case StateEnum::SyncPush:
				{
					if (Driver.CanPushBuffer())
					{
						const uint32_t elapsed = micros() - LastSync;
						if (elapsed >= TargetDuration)
						{
							if (elapsed > (TargetDuration * 2))
							{
								// Frame took longer than the 2x target duration.
								LastSync = micros();
							}
							else
							{
								LastSync += TargetDuration;
							}

							// Push the native color buffer to the lights.
							Driver.PushBuffer(NativeBuffer, NativeBufferSize);
							FrameCounter++;

							// Frame push complete, move to next one.
							State = StateEnum::BufferClear;
							TS::Task::delay(0);
						}
						else
						{
							// Wait for the target duration to elapse before pushing the buffer to the lights.
							TS::Task::delay((TargetDuration - elapsed) / 1000);
						}
					}
					else
					{
						// Wait for the driver to finish pushing the buffer to the lights before syncing to the target duration.
						TS::Task::delay(0);
					}
				}
				break;
				case StateEnum::Disabled:
				default:
					TS::Task::disable();
					break;
				}

				return true;
			}
		};
	}
}

#endif