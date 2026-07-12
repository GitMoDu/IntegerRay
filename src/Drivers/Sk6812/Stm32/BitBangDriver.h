#ifndef _INTEGER_RAY_DRIVERS_SK6812_STM32_BIT_BANG_DRIVER_h
#define _INTEGER_RAY_DRIVERS_SK6812_STM32_BIT_BANG_DRIVER_h

#if defined(ARDUINO_ARCH_STM32F1) || defined(ARDUINO_ARCH_STM32)
#include <Arduino.h>
#include "../../../Model/Interface.h"


namespace IntegerRay
{
	namespace Drivers
	{
		namespace Sk6812
		{
			namespace Stm32
			{
				namespace Detail
				{
					constexpr uint32_t NanosecondsToCycles(const uint32_t nanoseconds)
					{
						// round(F_CPU * nanoseconds / 1,000,000,000): the product
						// stays 64-bit because F_CPU is expressed in cycles per second.
						return static_cast<uint32_t>((uint64_t(F_CPU) * nanoseconds + 500000000ULL) / 1000000000ULL);
					}

					struct ProtocolTiming
					{
						// Nominal SK6812 timing: T0H = 0.35 us, T1H = 0.90 us,
						// and a bit period of 1.25 us. These values are selected
						// from the ranges in the SK6812 timing characteristics.
						static constexpr uint32_t ZeroHighNs = 350;
						static constexpr uint32_t OneHighNs = 900;
						static constexpr uint32_t BitPeriodNs = 1250;

						static constexpr uint32_t ZeroHighCycles = NanosecondsToCycles(ZeroHighNs);
						static constexpr uint32_t OneHighCycles = NanosecondsToCycles(OneHighNs);
						static constexpr uint32_t BitPeriodCycles = NanosecondsToCycles(BitPeriodNs);
					};

					static_assert(ProtocolTiming::ZeroHighCycles > 0, "IntegerRay SK6812 STM32 bitbang: F_CPU is too low.");
					static_assert(ProtocolTiming::BitPeriodNs > ProtocolTiming::OneHighNs, "IntegerRay SK6812 STM32 bitbang: invalid SK6812 timing constants.");

					struct DwtCycleTimer
					{
						// ARMv7-M DWT_CTRL, DWT_CYCCNT, and DEMCR addresses from the
						// ARM Cortex-M3 Technical Reference Manual, Debug and Trace
						// section. The STM32F1 reference manual documents the same
						// Cortex-M3 debug block in its Core peripherals section.
						static volatile uint32_t& DwtControl()
						{
							return *reinterpret_cast<volatile uint32_t*>(0xE0001000);
						}

						static volatile uint32_t& CycleCount()
						{
							return *reinterpret_cast<volatile uint32_t*>(0xE0001004);
						}

						static volatile uint32_t& DebugControl()
						{
							return *reinterpret_cast<volatile uint32_t*>(0xE000EDFC);
						}

						static void Enable()
						{
							DebugControl() |= (1UL << 24);
							CycleCount() = 0;
							DwtControl() |= 1UL;
						}

						static uint32_t Read()
						{
							return CycleCount();
						}

						static void WaitFor(const uint32_t target)
						{
							while (int32_t(Read() - target) < 0)
							{
							}
						}
					};

					struct GpioWriter
					{
						volatile uint32_t* SetResetRegister;
						uint32_t SetValue;
						uint32_t ResetValue;

						GpioWriter()
							: SetResetRegister(nullptr)
							, SetValue(0)
							, ResetValue(0)
						{}

						GpioWriter(volatile uint32_t* setResetRegister, const uint32_t setValue, const uint32_t resetValue)
							: SetResetRegister(setResetRegister)
							, SetValue(setValue)
							, ResetValue(resetValue)
						{}

						void High() const
						{
							*SetResetRegister = SetValue;
						}

						void Low() const
						{
							*SetResetRegister = ResetValue;
						}
					};

					template<uint8_t Pin>
					GpioWriter ResolveGpio()
					{
						pinMode(Pin, OUTPUT);

#if defined(STM32_CORE_VERSION)
						GPIO_TypeDef* port = digitalPinToPort(Pin);
						const uint32_t mask = digitalPinToBitMask(Pin);

						return GpioWriter(&port->BSRR, mask, mask << 16);
#elif defined(ARDUINO_ARCH_STM32F1)
						gpio_dev* port = PIN_MAP[Pin].gpio_device;
						const uint32_t mask = 1UL << PIN_MAP[Pin].gpio_bit;

						return GpioWriter(&port->regs->BSRR, mask, mask << 16);
#else
#error "IntegerRay SK6812 STM32 bitbang: unsupported STM32 Arduino core."
#endif
					}

					static inline void Transmit(const uint8_t* data, const uint16_t length, const GpioWriter& output)
					{
						uint16_t remaining = length;

						noInterrupts();

						while (remaining--)
						{
							const uint8_t value = *data++;

							for (uint8_t mask = 0x80; mask; mask >>= 1)
							{
								const uint32_t start = DwtCycleTimer::Read();
								const uint32_t highCycles = (value & mask) ? ProtocolTiming::OneHighCycles : ProtocolTiming::ZeroHighCycles;

								output.High();
								DwtCycleTimer::WaitFor(start + highCycles);
								output.Low();
								DwtCycleTimer::WaitFor(start + ProtocolTiming::BitPeriodCycles);
							}
						}

						interrupts();
					}

					template<typename ColorType, uint16_t LedCount, uint8_t Pin>
					class BitBangDriver : public IStripDriver
					{
					public:
						using color_t = ColorType;
						using component_t = IntegerRay::ColorModel::Output::component_t;

					private:
						GpioWriter Output;

					public:
						BitBangDriver()
							: IStripDriver()
						{}

						~BitBangDriver() = default;

						bool Setup()
						{
							DwtCycleTimer::Enable();
							Output = ResolveGpio<Pin>();
							Output.Low();
							return true;
						}

						virtual bool CanPushBuffer() override
						{
							return true;
						}

						virtual uint16_t GetCount() override
						{
							return LedCount;
						}

						virtual void PushBuffer(const ColorModel::Output::component_t* buffer, const size_t length) override
						{
							static constexpr uint8_t ChannelCount = color_t::CHANNEL_COUNT;
							static constexpr size_t ByteCount = size_t(LedCount) * ChannelCount;

							if (length < ByteCount)
							{
								return;
							}

							Transmit(buffer, ByteCount, Output);
						}
					};
				}

				template<uint16_t LedCount, uint8_t OutputPin>
				using BitBangRgbDriver = Detail::BitBangDriver<IntegerRay::ColorModel::Output::Rgb, LedCount, OutputPin>;

				template<uint16_t LedCount, uint8_t OutputPin>
				using BitBangRgbwDriver = Detail::BitBangDriver<IntegerRay::ColorModel::Output::Rgbw, LedCount, OutputPin>;

			}
		}
	}
}



#endif
#endif