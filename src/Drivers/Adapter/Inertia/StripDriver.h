#ifndef _INTEGER_RAY_ADAPTERS_INERTIA_STRIP_DRIVER_h
#define _INTEGER_RAY_ADAPTERS_INERTIA_STRIP_DRIVER_h

#include "../../../Model/Interface.h"

namespace IntegerRay
{
	namespace Adapters
	{
		namespace Inertia
		{
			template<typename Driver, typename RayFormat, typename InertiaFormat, uint16_t LedCount>
			class StripDriver : public IntegerRay::IExternalBufferStripDriver
			{
			private:
				static constexpr uint8_t ChannelCount = InertiaFormat::ChannelCount;
				static_assert(RayFormat::CHANNEL_COUNT == InertiaFormat::ChannelCount,
					"IntegerRay and Inertia strip formats must have the same channel count.");
				static_assert(LedCount > 0, "The strip must contain at least one LED.");

				Driver& WrappedDriver;
				const uint8_t* Buffer = nullptr;
				size_t BufferSize = 0;

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

				bool SetBuffer(const uint8_t* buffer, const size_t size) override
				{
					if (buffer == nullptr || size != size_t(LedCount) * ChannelCount)
						return false;
					Buffer = buffer;
					BufferSize = size;
					return true;
				}

				bool Push() override
				{
					if (Buffer == nullptr || BufferSize != size_t(LedCount) * ChannelCount || !IsReady())
						return false;
					if (!WrappedDriver.Push(Buffer, BufferSize))
						return false;
					Buffer = nullptr;
					BufferSize = 0;
					return true;
				}
			};
		}
	}
}

#endif
