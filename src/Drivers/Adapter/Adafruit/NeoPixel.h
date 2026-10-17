#ifndef _INTEGER_RAY_ADAPTERS_ADAFRUIT_NEO_PIXEL_h
#define _INTEGER_RAY_ADAPTERS_ADAFRUIT_NEO_PIXEL_h

#include <stddef.h>
#include <stdint.h>

#include "../../../Model/Interface.h"

namespace IntegerRay
{
	namespace Adapters
	{
		namespace Adafruit
		{
			template<typename NeoPixel, typename OutputFormat, uint16_t LedCount,
				uint8_t RedIndex, uint8_t GreenIndex, uint8_t BlueIndex,
				int8_t WhiteIndex = -1>
			class NeoPixelDriver : public IntegerRay::IStripDriver
			{
			private:
				static constexpr uint8_t ChannelCount = OutputFormat::CHANNEL_COUNT;
				static_assert(ChannelCount == (WhiteIndex < 0 ? 3 : 4),
					"Output format channel count must match the selected RGB or RGBW channel indices.");
				static_assert(RedIndex < ChannelCount && GreenIndex < ChannelCount && BlueIndex < ChannelCount,
					"RGB channel indices must be within the pixel format.");
				static_assert(WhiteIndex < static_cast<int8_t>(ChannelCount),
					"White channel index must be absent or within the pixel format.");
				static_assert(WhiteIndex >= -1,
					"White channel index must be -1 when absent or a valid channel index.");
				static_assert(RedIndex != GreenIndex && RedIndex != BlueIndex && GreenIndex != BlueIndex,
					"RGB channel indices must be unique.");
				static_assert(WhiteIndex < 0 || (WhiteIndex != RedIndex && WhiteIndex != GreenIndex && WhiteIndex != BlueIndex),
					"White channel index must not overlap RGB channels.");

				NeoPixel& Strip;

			public:
				using color_t = OutputFormat;

				explicit NeoPixelDriver(NeoPixel& strip)
					: Strip(strip)
				{}

				uint16_t GetCount() override
				{
					return Strip.numPixels() == LedCount ? LedCount : 0;
				}

				bool Setup()
				{
					return Strip.begin();
				}

				bool IsReady() override
				{
					return Strip.canShow();
				}

				bool SetPixel(const uint16_t index, const uint32_t color) override
				{
					if (index >= LedCount || Strip.numPixels() != LedCount)
						return false;
					const uint8_t red = static_cast<uint8_t>(color >> ((ChannelCount - 1 - RedIndex) << 3));
					const uint8_t green = static_cast<uint8_t>(color >> ((ChannelCount - 1 - GreenIndex) << 3));
					const uint8_t blue = static_cast<uint8_t>(color >> ((ChannelCount - 1 - BlueIndex) << 3));
					if (WhiteIndex < 0)
						Strip.setPixelColor(index, red, green, blue);
					else
					{
						const uint8_t white = static_cast<uint8_t>(color >> ((ChannelCount - 1 - WhiteIndex) << 3));
						Strip.setPixelColor(index, red, green, blue, white);
					}
					return true;
				}

				bool Push() override
				{
					if (!IsReady() || Strip.numPixels() != LedCount)
						return false;
					Strip.show();
					return true;
				}
			};
		}
	}
}

#endif
