#ifndef _INTEGER_RAY_ENGINE_RAY_ENGINE_h
#define _INTEGER_RAY_ENGINE_RAY_ENGINE_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "../Model/Model.h"
#include "../Model/Interface.h"

namespace IntegerRay
{
	namespace Engine
	{
		

		/// <summary>
		/// Template-based engine for driving LED strips with a specified color format, count, and buffer mode.
		/// </summary>
		/// <typeparam name="format_t">The color format type.</typeparam>
		/// <typeparam name="Count">The number of LEDs in the strip.</typeparam>
		/// <typeparam name="Conversion">The compile-time output-buffer conversion mode.</typeparam>
		/// <typeparam name="DriverType">The concrete strip driver and its output-buffer interface.</typeparam>
		template<typename format_t, uint16_t Count, ConversionModeEnum Conversion, typename DriverType>
		class TemplateRayEngine : private TS::Task
		{
			enum class StateEnum
			{
				Initializing,	// Clears the driver output and starts the frame cycle.
				InitializingPush,	// Pushes the staged initial output frame.
				DrawingClear,	// Clears the drawing buffer before rendering the next frame.
				Drawing,		// Renders the animation frame into the drawing buffer.
				WaitingForPush,	// Waits for driver readiness and target frame time.
				Converting,		// Converts the drawing buffer into driver output.
				Pushing,		// Submits the completely converted frame to the driver.
				Disabled		// Engine is disabled, no animation or buffer updates will occur.
			};

			// Internal buffer for the engine to draw to.
			TemplateRayBuffer<Count> DrawBuffer{};
			using frame_buffer_t = typename IntegerSignal::TypeTraits::TypeConditional::conditional_type<
				SeparateOutputNativeBuffer<Count, format_t::CHANNEL_COUNT>,
				ReusedDrawNativeBuffer<Count, format_t::CHANNEL_COUNT>,
				Conversion == ConversionModeEnum::SeparateOutputBuffer>::type;
			using output_buffer_t = typename IntegerSignal::TypeTraits::TypeConditional::conditional_type<
				frame_buffer_t,
				EmptyNativeBuffer,
				!DriverType::HasNativeBuffer>::type;
			output_buffer_t OutputBuffer{};

			static_assert(Conversion == ConversionModeEnum::SeparateOutputBuffer || Conversion == ConversionModeEnum::ReuseDrawBuffer,
				"Unsupported output conversion mode.");
			static_assert(Conversion != ConversionModeEnum::ReuseDrawBuffer || !DriverType::HasNativeBuffer,
				"Reusing the draw buffer requires a driver that consumes an external frame buffer.");

			uint32_t LastSync = 0;
			uint32_t AnimationTimestamp = 0;

			// Current drawer for the engine to use for drawing to the buffer.
			IRayDrawer* Drawer = nullptr;

			// Current state of the engine.
			StateEnum State = StateEnum::Disabled;
			uint16_t FrameCounter = 0;

			DriverType& Driver;
			uint32_t TargetDuration;
			const ColorModel::Grading::IColorGrader* Grader = nullptr;

		public:
			TemplateRayEngine(TS::Scheduler& scheduler,
				DriverType& driver, const uint32_t targetPeriodMicros = 10000)
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

			void SetColorGrader(const ColorModel::Grading::IColorGrader* grader)
			{
				Grader = grader;
			}

			void SetColorGrader(const ColorModel::Grading::IColorGrader& grader)
			{
				Grader = &grader;
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

			virtual bool Callback() final
			{
				switch (State)
				{
				case StateEnum::Initializing:
					if (DrawBuffer.GetCount() == Driver.GetCount()
						&& DrawBuffer.GetCount() == Count)
					{
						if (!Driver.IsReady())
						{
							TS::Task::delay(0);
							break;
						}

						if (!StageInitialBuffer(Driver))
						{
							TS::Task::delay(0);
							break;
						}
						State = StateEnum::InitializingPush;
						TS::Task::delay(0);
					}
					else
					{
						State = StateEnum::Disabled;
					}
					break;
				case StateEnum::InitializingPush:
					if (Driver.Push())
					{
						FrameCounter = 0;
						State = StateEnum::DrawingClear;
						TS::Task::enableDelayed(0);
						LastSync = micros() - TargetDuration;
					}
					TS::Task::delay(0);
					break;
				case StateEnum::DrawingClear:
					// Frame start, clear the draw buffer to black/off before starting the animation.
					DrawBuffer.Clear();
					State = StateEnum::Drawing;
					AnimationTimestamp = micros();
					TS::Task::delay(0);
					break;
				case StateEnum::Drawing:
					if (Drawer != nullptr)
					{
						if (Drawer->OnDraw(DrawBuffer, AnimationTimestamp, FrameCounter))
						{
							State = StateEnum::Converting;
						}
					}
					else
					{
						State = StateEnum::Converting;
					}
					TS::Task::delay(0);
					break;
				case StateEnum::Converting:
					if (ConvertAndWriteBuffer(Driver))
						State = StateEnum::WaitingForPush;
					TS::Task::delay(0);
					break;
				case StateEnum::WaitingForPush:
					if (Driver.IsReady())
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

							State = StateEnum::Pushing;
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
					break;
				case StateEnum::Pushing:
					if (Driver.Push())
					{
						FrameCounter++;
						State = StateEnum::DrawingClear;
					}
					TS::Task::delay(0);
					break;
				case StateEnum::Disabled:
				default:
					TS::Task::disable();
					break;
				}

				return true;
			}

		private:
			bool StageInitialBuffer(IBufferedStripDriver& driver)
			{
				const uint32_t black = static_cast<uint32_t>(format_t::Pack(ColorModel::Internal::BLACK));
				bool frameReady = true;
				for (uint16_t index = 0; index < Count; ++index)
					frameReady = driver.SetPixel(index, black) && frameReady;
				return frameReady;
			}

			bool StageInitialBuffer(IExternalBufferStripDriver& driver)
			{
				const size_t nativeByteCount = size_t(Count) * format_t::CHANNEL_COUNT;
				uint8_t* const output = OutputBuffer.Get(DrawBuffer.Buffer);
				memset(output, 0, nativeByteCount);
				return driver.SetBuffer(output, nativeByteCount);
			}

			bool ConvertAndWriteBuffer(IBufferedStripDriver& driver)
			{
				bool frameReady = true;
				for (uint16_t index = 0; index < Count; ++index)
				{
					ColorModel::Internal::color_t color = DrawBuffer.Buffer[index];
					if (Grader != nullptr)
						color = Grader->Grade(color, index);
					const typename format_t::color_t packed = format_t::Pack(color);
					frameReady = driver.SetPixel(index, static_cast<uint32_t>(packed)) && frameReady;
				}
				return frameReady;
			}

			bool ConvertAndWriteBuffer(IExternalBufferStripDriver& driver)
			{
				uint8_t* const output = OutputBuffer.Get(DrawBuffer.Buffer);
				for (uint16_t index = 0; index < Count; ++index)
				{
					ColorModel::Internal::color_t color = DrawBuffer.Buffer[index];
					if (Grader != nullptr)
						color = Grader->Grade(color, index);
					const typename format_t::color_t packed = format_t::Pack(color);
					for (uint8_t channel = 0; channel < format_t::CHANNEL_COUNT; ++channel)
					{
						const uint8_t shift = (format_t::CHANNEL_COUNT - 1 - channel) << 3;
						output[size_t(index) * format_t::CHANNEL_COUNT + channel] = static_cast<uint8_t>(packed >> shift);
					}
				}

				return driver.SetBuffer(output, size_t(Count) * format_t::CHANNEL_COUNT);
			}
		};
	}
}

#endif