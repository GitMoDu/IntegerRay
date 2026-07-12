#ifndef _INTEGER_RAY_ENGINE_RAY_BUFFER_h
#define _INTEGER_RAY_ENGINE_RAY_BUFFER_h


#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "../Model/Interface.h"


namespace IntegerRay
{
	namespace Engine
	{
		template<uint16_t Count>
		struct TemplateRayBuffer : public IRayBuffer
		{
			color_t Buffer[Count]{};

			TemplateRayBuffer() : IRayBuffer() {}

			~TemplateRayBuffer() = default;

			void Clear()
			{
				memset(Buffer, 0, sizeof(Buffer));
			}

			virtual uint16_t GetCount() final { return Count; }

			virtual void Set(const color_t& color, const uint16_t index) final
			{
				if (index < Count)  //TODO: optional safety checks.
				{
					Buffer[index] = color;
				}
			}

			virtual void Add(const color_t& color, const uint16_t index)
			{
				if (index < Count)  //TODO: optional safety checks.
				{
					static constexpr component_t Max = IntegerSignal::RgbFraction16::COMPONENT_MAX;
					color_t& current = Buffer[index];
					current.red = component_t(IntegerSignal::MinValue(uint32_t(current.red) + color.red, uint32_t(Max)));
					current.green = component_t(IntegerSignal::MinValue(uint32_t(current.green) + color.green, uint32_t(Max)));
					current.blue = component_t(IntegerSignal::MinValue(uint32_t(current.blue) + color.blue, uint32_t(Max)));
				}
			}

			virtual void Subtract(const color_t& color, const uint16_t index)
			{
				if (index < Count)  //TODO: optional safety checks.
				{
					color_t& current = Buffer[index];
					current.red = component_t(IntegerSignal::MaxValue(int32_t(current.red) - color.red, int32_t(0)));
					current.green = component_t(IntegerSignal::MaxValue(int32_t(current.green) - color.green, int32_t(0)));
					current.blue = component_t(IntegerSignal::MaxValue(int32_t(current.blue) - color.blue, int32_t(0)));
				}
			}

			virtual void Multiply(const color_t& color, const uint16_t index)
			{
				if (index < Count)  //TODO: optional safety checks.
				{
					color_t& current = Buffer[index];
					current.red = IntegerSignal::FixedPoint::ScalarFraction::Fraction(current.red, color.red);
					current.green = IntegerSignal::FixedPoint::ScalarFraction::Fraction(current.green, color.green);
					current.blue = IntegerSignal::FixedPoint::ScalarFraction::Fraction(current.blue, color.blue);
				}
			}

			virtual void Blend(const color_t& color, const uint16_t index)
			{
				if (index < Count)  //TODO: optional safety checks.
				{
					Buffer[index] = IntegerSignal::RgbFraction16::ColorInterpolate(Buffer[index], color, IntegerSignal::RgbFraction16::COMPONENT_MAX / 2);
				}
			}

			virtual void Blend(const color_t& color, const uint16_t index, const component_t blend)
			{
				if (index < Count)  //TODO: optional safety checks.
				{
					Buffer[index] = IntegerSignal::RgbFraction16::ColorInterpolate(Buffer[index], color, blend);
				}
			}

		};

	}
}



#endif

