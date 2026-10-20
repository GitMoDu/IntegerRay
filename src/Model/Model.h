#ifndef _INTEGER_RAY_MODEL_h
#define _INTEGER_RAY_MODEL_h

// IntegerRay depends on IntegerSignal https://github.com/GitMoDu/IntegerSignal
#include <IntegerSignal.h>
#include "ColorModel.h"

namespace IntegerRay
{
	namespace Engine
	{
		enum class ConversionModeEnum : uint8_t
		{
			SeparateOutputBuffer,
			ReuseDrawBuffer,
			EnumCount
		};

		template<uint16_t Count, uint8_t ChannelCount>
		struct SeparateOutputNativeBuffer
		{
			uint8_t Buffer[size_t(Count) * ChannelCount]{};

			uint8_t* Get(ColorModel::Internal::color_t*) { return Buffer; }
		};

		template<uint16_t Count, uint8_t ChannelCount>
		struct ReusedDrawNativeBuffer
		{
			static_assert(sizeof(ColorModel::Internal::color_t) >= ChannelCount,
				"Reusing the draw buffer requires each HDR pixel to be at least as large as its native output pixel.");

			uint8_t* Get(ColorModel::Internal::color_t* drawBuffer)
			{
				return reinterpret_cast<uint8_t*>(drawBuffer);
			}
		};

		struct EmptyNativeBuffer
		{
			uint8_t* Get(ColorModel::Internal::color_t*) { return nullptr; }
		};
	}
}
#endif