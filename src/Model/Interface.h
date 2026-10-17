#ifndef _INTEGER_RAY_INTERFACE_h
#define _INTEGER_RAY_INTERFACE_h

#include "ColorModel.h"

namespace IntegerRay
{
	struct IStripDriver
	{
		IStripDriver() = default;
		virtual ~IStripDriver() = default;

		virtual uint16_t GetCount() = 0;

		virtual bool IsReady() = 0;

		virtual bool SetPixel(const uint16_t index, const uint32_t color) = 0;

		virtual bool Push() = 0;
	};

	struct IRayBuffer
	{
		IRayBuffer() = default;
		~IRayBuffer() = default;

		virtual uint16_t GetCount() = 0;

		virtual void Set(const color_t& color, const uint16_t index) = 0;
		virtual void Add(const color_t& color, const uint16_t index) = 0;
		virtual void Subtract(const color_t& color, const uint16_t index) = 0;
		virtual void Multiply(const color_t& color, const uint16_t index) = 0;
		virtual void Blend(const color_t& color, const uint16_t index) = 0;
		virtual void Blend(const color_t& color, const uint16_t index, const component_t blend) = 0;
	};

	struct IRayDrawer
	{
		IRayDrawer() = default;
		virtual ~IRayDrawer() = default;

		virtual bool OnDraw(IRayBuffer& buffer, const uint32_t timestamp, const uint16_t frameCounter) = 0;
	};
}
#endif