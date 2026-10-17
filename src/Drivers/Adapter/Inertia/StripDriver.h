#ifndef _INTEGER_RAY_ADAPTERS_INERTIA_STRIP_DRIVER_h
#define _INTEGER_RAY_ADAPTERS_INERTIA_STRIP_DRIVER_h

#include <stdint.h>
#include "../../../Model/Interface.h"

namespace IntegerRay
{
	namespace Adapters
	{
		namespace Inertia
		{
			template<typename Driver, typename RayFormat, typename InertiaFormat, uint16_t LedCount>
			class StripDriver : public IntegerRay::IStripDriver
			{
			private:
				static constexpr uint8_t ChannelCount = InertiaFormat::ChannelCount;
				using inertia_color_t = typename InertiaFormat::color_t;
				static_assert(RayFormat::CHANNEL_COUNT == InertiaFormat::ChannelCount,
					"IntegerRay and Inertia strip formats must have the same channel count.");
				static_assert(sizeof(inertia_color_t) == ChannelCount,
					"Inertia strip colors must be tightly packed for native-buffer transfer.");
				static_assert(LedCount > 0, "The strip must contain at least one LED.");

				Driver& WrappedDriver;

			public:
				using color_t = RayFormat;

				explicit StripDriver(Driver& driver)
					: WrappedDriver(driver)
				{}

				uint16_t GetCount() override
				{
					return LedCount;
				}

				bool IsReady() override
				{
					return WrappedDriver.IsReady();
				}

				bool Setup()
				{
					return WrappedDriver.Start();
				}

				void Stop()
				{
					WrappedDriver.Stop();
				}

				bool SetPixel(const uint16_t index, const uint32_t color) override
				{
					if (index >= LedCount || !IsReady() || WrappedDriver.GetColorCount() != LedCount)
						return false;
					inertia_color_t* colors = WrappedDriver.GetColorBuffer();
					if (colors == nullptr)
						return false;
					for (uint8_t channel = 0; channel < ChannelCount; ++channel)
					{
						const uint8_t shift = (ChannelCount - 1 - channel) << 3;
						colors[index].Channels[channel] = static_cast<uint8_t>(color >> shift);
					}
					return true;
				}

				bool Push() override
				{
					return IsReady()
						&& WrappedDriver.GetColorCount() == LedCount
						&& WrappedDriver.Push();
				}
			};
		}
	}
}

#endif
