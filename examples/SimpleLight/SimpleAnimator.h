#ifndef _SIMPLEANIMATOR_h
#define _SIMPLEANIMATOR_h

#include <IntegerRay.h>

class SimpleAnimator : public IntegerRay::IRayDrawer
{
private:
	static constexpr IntegerRay::component_t Saturation = (uint32_t(IntegerRay::ColorModel::Internal::COMPONENT_MAX) * 9) / 10;
	static constexpr IntegerRay::component_t Brightness = IntegerRay::ColorModel::Internal::COMPONENT_MAX / 16;

public:
	SimpleAnimator() : IntegerRay::IRayDrawer() {}

	~SimpleAnimator() = default;

	virtual bool OnDraw(IntegerRay::IRayBuffer& buffer, const uint32_t timestamp, const uint16_t /*frameCounter*/) override
	{
		const uint16_t count = buffer.GetCount();
		const auto hueOffset = uint16_t(timestamp >> 8);

		for (uint16_t index = 0; index < count; index++)
		{
			// Continuously rotate hue across the strip with a time-based offset and pixel index.
			const auto hue = uint16_t(hueOffset + (index * (10000 / count)));
			const IntegerRay::color_t color = IntegerSignal::RgbFraction16::ColorHsv(hue, Saturation, Brightness);
			buffer.Set(color, index);
		}

		return true; // All pixels animated in a single pass.
	}
};


#endif

