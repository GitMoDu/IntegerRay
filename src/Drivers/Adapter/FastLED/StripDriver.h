#ifndef _INTEGER_RAY_ADAPTERS_FASTLED_STRIP_DRIVER_h
#define _INTEGER_RAY_ADAPTERS_FASTLED_STRIP_DRIVER_h

#include <FastLED.h>

#include "../../../Model/Interface.h"

namespace IntegerRay
{
	namespace Adapters
	{
		namespace FastLED
		{
			template<typename OutputFormat, uint16_t LedCount>
			class StripDriver : public IntegerRay::IExternalBufferStripDriver
			{
			private:
				static constexpr uint8_t ChannelCount = OutputFormat::CHANNEL_COUNT;
				static_assert(LedCount > 0, "The strip must contain at least one LED.");
				static_assert(ChannelCount == 3, "FastLED CRGB output supports three-channel formats.");

				CLEDController* Controller = nullptr;
				const uint8_t* Buffer = nullptr;
				size_t BufferSize = 0;

			public:
				using color_t = OutputFormat;

				void SetController(CLEDController& controller)
				{
					Controller = &controller;
				}

				uint16_t GetCount() override
				{
					return LedCount;
				}

				bool Setup()
				{
					return Controller != nullptr;
				}

				bool IsReady() override
				{
					return Controller != nullptr;
				}

				bool SetBuffer(const uint8_t* buffer, const size_t size) override
				{
					if (Controller == nullptr || buffer == nullptr || size != size_t(LedCount) * ChannelCount)
						return false;
					Buffer = buffer;
					BufferSize = size;
					Controller->setLeds(reinterpret_cast<CRGB*>(const_cast<uint8_t*>(buffer)), LedCount);
					return true;
				}

				bool Push() override
				{
					if (Controller == nullptr || Buffer == nullptr
						|| BufferSize != size_t(LedCount) * ChannelCount)
						return false;
					Controller->showLeds(UINT8_MAX);
					Buffer = nullptr;
					BufferSize = 0;
					return true;
				}
			};
		}
	}
}

#endif
