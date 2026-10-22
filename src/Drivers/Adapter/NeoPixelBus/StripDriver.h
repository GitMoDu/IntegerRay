#ifndef _INTEGER_RAY_ADAPTERS_NEOPIXELBUS_STRIP_DRIVER_h
#define _INTEGER_RAY_ADAPTERS_NEOPIXELBUS_STRIP_DRIVER_h

#include <stddef.h>
#include <stdint.h>

#include "../../../Model/Interface.h"

namespace IntegerRay
{
	namespace Adapters
	{
		namespace NeoPixelBus
		{
			template<typename PixelBus, typename PixelColor, typename OutputFormat, uint16_t LedCount,
				uint8_t RedIndex, uint8_t GreenIndex, uint8_t BlueIndex, int8_t WhiteIndex = -1>
			class StripDriver : public IntegerRay::IBufferedStripDriver
			{
			private:
				static constexpr uint8_t ChannelCount = OutputFormat::CHANNEL_COUNT;
				static_assert(LedCount > 0, "The strip must contain at least one LED.");
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
				static_assert(WhiteIndex < 0 || (WhiteIndex != RedIndex && WhiteIndex != GreenIndex
					&& WhiteIndex != BlueIndex),
					"White channel index must not overlap RGB channels.");

				using pixel_format_tag_t = typename IntegerSignal::TypeTraits::TypeConditional::conditional_type<
					IntegerSignal::TypeTraits::TypeDispatch::TrueType,
					IntegerSignal::TypeTraits::TypeDispatch::FalseType,
					(WhiteIndex < 0)>::type;

				PixelBus& Strip;
				bool Started = false;

				PixelColor MakePixelColor(const uint32_t color,
					IntegerSignal::TypeTraits::TypeDispatch::TrueType)
				{
					const uint8_t red = static_cast<uint8_t>(color >> ((ChannelCount - 1 - RedIndex) << 3));
					const uint8_t green = static_cast<uint8_t>(color >> ((ChannelCount - 1 - GreenIndex) << 3));
					const uint8_t blue = static_cast<uint8_t>(color >> ((ChannelCount - 1 - BlueIndex) << 3));
					return PixelColor(red, green, blue);
				}

				PixelColor MakePixelColor(const uint32_t color,
					IntegerSignal::TypeTraits::TypeDispatch::FalseType)
				{
					const uint8_t red = static_cast<uint8_t>(color >> ((ChannelCount - 1 - RedIndex) << 3));
					const uint8_t green = static_cast<uint8_t>(color >> ((ChannelCount - 1 - GreenIndex) << 3));
					const uint8_t blue = static_cast<uint8_t>(color >> ((ChannelCount - 1 - BlueIndex) << 3));
					const uint8_t white = static_cast<uint8_t>(color >> ((ChannelCount - 1 - WhiteIndex) << 3));
					return PixelColor(red, green, blue, white);
				}

			public:
				using color_t = OutputFormat;

				explicit StripDriver(PixelBus& strip)
					: Strip(strip)
				{}

				uint16_t GetCount() override
				{
					return Strip.PixelCount() == LedCount ? LedCount : 0;
				}

				bool Setup()
				{
					if (Strip.PixelCount() != LedCount)
						return false;
					Strip.Begin();
					Started = true;
					return true;
				}

				bool IsReady() override
				{
					return Started && Strip.CanShow();
				}

				bool SetPixel(const uint16_t index, const uint32_t color) override
				{
					if (!Started || index >= LedCount || Strip.PixelCount() != LedCount)
						return false;
					Strip.SetPixelColor(index, MakePixelColor(color, pixel_format_tag_t{}));
					return true;
				}

				bool Push() override
				{
					if (!IsReady() || Strip.PixelCount() != LedCount)
						return false;
					Strip.Show();
					return true;
				}
			};
		}
	}
}

#endif
