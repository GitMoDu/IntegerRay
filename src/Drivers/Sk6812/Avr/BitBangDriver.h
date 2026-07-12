#ifndef _INTEGER_RAY_DRIVERS_SK6812_AVR_BIT_BANG_DRIVER_h
#define _INTEGER_RAY_DRIVERS_SK6812_AVR_BIT_BANG_DRIVER_h

#if defined(ARDUINO_ARCH_AVR)
#include <Arduino.h>
#include "../../../Model/Interface.h"


// SK6812 timing values from the device timing characteristics:
// T0H = 0.35 us, T1H = 0.90 us, and a nominal bit period of 1.25 us.
// The expressions below convert those values to AVR cycles and allocate
// the cycles around the fixed instructions in the bit loop.
#define INTEGER_RAY_SK6812_DETAIL_W1_NOPS (((((F_CPU / 1000) * 350) / 1000000) - 3) > 0 ? ((((F_CPU / 1000) * 350) / 1000000) - 3) : 0)
#define INTEGER_RAY_SK6812_DETAIL_W2_NOPS ((((((F_CPU / 1000) * 900) + 500000) / 1000000) - 6 - (((F_CPU / 1000) * 350) / 1000000 - 3)) > 0 ? (((((F_CPU / 1000) * 900) + 500000) / 1000000) - 6 - (((F_CPU / 1000) * 350) / 1000000 - 3)) : 0)
#define INTEGER_RAY_SK6812_DETAIL_W3_NOPS (((((((F_CPU / 1000) * 1250) + 500000) / 1000000) - 10 - (((F_CPU / 1000) * 350) / 1000000 - 3) - (((((F_CPU / 1000) * 900) + 500000) / 1000000) - 6 - (((F_CPU / 1000) * 350) / 1000000 - 3))) > 0) ? (((((F_CPU / 1000) * 1250) + 500000) / 1000000) - 10 - (((F_CPU / 1000) * 350) / 1000000 - 3) - (((((F_CPU / 1000) * 900) + 500000) / 1000000) - 6 - (((F_CPU / 1000) * 350) / 1000000 - 3))) : 0)
#define INTEGER_RAY_SK6812_DETAIL_NOP1  "nop      \n\t"
#define INTEGER_RAY_SK6812_DETAIL_NOP2  "rjmp .+0 \n\t"
#define INTEGER_RAY_SK6812_DETAIL_NOP4  INTEGER_RAY_SK6812_DETAIL_NOP2 INTEGER_RAY_SK6812_DETAIL_NOP2
#define INTEGER_RAY_SK6812_DETAIL_NOP8  INTEGER_RAY_SK6812_DETAIL_NOP4 INTEGER_RAY_SK6812_DETAIL_NOP4
#define INTEGER_RAY_SK6812_DETAIL_NOP16 INTEGER_RAY_SK6812_DETAIL_NOP8 INTEGER_RAY_SK6812_DETAIL_NOP8


namespace IntegerRay
{
	namespace Drivers
	{
		namespace Sk6812
		{
			namespace Avr
			{
				namespace DetailSk6812
				{
					constexpr uint16_t Cycles(const uint16_t nanoseconds, const bool round)
					{
						// round(F_CPU * nanoseconds / 1,000,000,000), with the
						// unrounded form used for T0H to avoid exceeding its maximum.
						return (((F_CPU / 1000) * nanoseconds) + (round ? 500000 : 0)) / 1000000;
					}

					constexpr uint8_t ClampedNops(const int16_t cycles)
					{
						return cycles > 0 ? cycles : 0;
					}

					struct Timing
					{
						static constexpr uint16_t ZeroPulseNs = 350;
						static constexpr uint16_t OnePulseNs = 900;
						static constexpr uint16_t TotalPeriodNs = 1250;

						// Fixed instruction costs surrounding the configurable delay:
						// output-low/branch path, output-high/bit-test path, and the
						// complete loop path respectively.
						static constexpr uint8_t FixedLowCycles = 3;
						static constexpr uint8_t FixedHighCycles = 6;
						static constexpr uint8_t FixedTotalCycles = 10;

						static constexpr uint16_t ZeroCycles = Cycles(ZeroPulseNs, false);
						static constexpr uint16_t OneCycles = Cycles(OnePulseNs, true);
						static constexpr uint16_t TotalCycles = Cycles(TotalPeriodNs, true);

						// W1 extends the initial high time, W2 extends the high-time
						// path for one bits, and W3 fills the remainder of the bit.
						static constexpr int16_t W1 = ZeroCycles - FixedLowCycles;
						static constexpr int16_t W2 = OneCycles - FixedHighCycles - W1;
						static constexpr int16_t W3 = TotalCycles - FixedTotalCycles - W1 - W2;

						static constexpr uint8_t W1Nops = ClampedNops(W1);
						static constexpr uint8_t W2Nops = ClampedNops(W2);
						static constexpr uint8_t W3Nops = ClampedNops(W3);
						static constexpr uint16_t LowTimeNs = ((W1Nops + FixedLowCycles) * 1000000) / (F_CPU / 1000);
					};

					// Keep the zero-bit high pulse within the documented T0H range.
					static_assert(Timing::LowTimeNs <= 550, "IntegerRay SK6812 AVR bitbang: clock speed is too low. Did you set F_CPU correctly?");

					static inline void SendArrayMask(const uint8_t* data, const uint16_t length, const uint8_t pinMask, volatile uint8_t* port)
					{
						uint8_t currentByte;
						uint8_t counter;
						uint8_t maskLow = ~pinMask & *port;
						uint8_t maskHigh = pinMask | *port;
						uint8_t sregPrevious = SREG;
						uint16_t remaining = length;

						cli();

						while (remaining--)
						{
							currentByte = *data++;

							asm volatile(
								"       ldi   %0,8  \n\t"
								"loop%=:            \n\t"
								"       st    X,%3 \n\t"
#if (INTEGER_RAY_SK6812_DETAIL_W1_NOPS & 1)
								INTEGER_RAY_SK6812_DETAIL_NOP1
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W1_NOPS & 2)
								INTEGER_RAY_SK6812_DETAIL_NOP2
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W1_NOPS & 4)
								INTEGER_RAY_SK6812_DETAIL_NOP4
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W1_NOPS & 8)
								INTEGER_RAY_SK6812_DETAIL_NOP8
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W1_NOPS & 16)
								INTEGER_RAY_SK6812_DETAIL_NOP16
#endif
								"       sbrs  %1,7  \n\t"
								"       st    X,%4 \n\t"
								"       lsl   %1    \n\t"
#if (INTEGER_RAY_SK6812_DETAIL_W2_NOPS & 1)
								INTEGER_RAY_SK6812_DETAIL_NOP1
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W2_NOPS & 2)
								INTEGER_RAY_SK6812_DETAIL_NOP2
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W2_NOPS & 4)
								INTEGER_RAY_SK6812_DETAIL_NOP4
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W2_NOPS & 8)
								INTEGER_RAY_SK6812_DETAIL_NOP8
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W2_NOPS & 16)
								INTEGER_RAY_SK6812_DETAIL_NOP16
#endif
								"       brcc skipone%= \n\t"
								"       st   X,%4      \n\t"
								"skipone%=:               "
#if (INTEGER_RAY_SK6812_DETAIL_W3_NOPS & 1)
								INTEGER_RAY_SK6812_DETAIL_NOP1
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W3_NOPS & 2)
								INTEGER_RAY_SK6812_DETAIL_NOP2
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W3_NOPS & 4)
								INTEGER_RAY_SK6812_DETAIL_NOP4
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W3_NOPS & 8)
								INTEGER_RAY_SK6812_DETAIL_NOP8
#endif
#if (INTEGER_RAY_SK6812_DETAIL_W3_NOPS & 16)
								INTEGER_RAY_SK6812_DETAIL_NOP16
#endif
								"       dec   %0    \n\t"
								"       brne  loop%=\n\t"
								: "=&d" (counter)
								: "r" (currentByte), "x" (port), "r" (maskHigh), "r" (maskLow)
								);
						}

						SREG = sregPrevious;
					}

					template<typename ColorType, uint16_t LedCount, uint8_t OutputPin>
					class BitBangDriver : public IStripDriver
					{
					public:
						using color_t = ColorType;
						using component_t = IntegerRay::ColorModel::Output::component_t;

					private:
						volatile uint8_t* Port = nullptr;
						volatile uint8_t* PortRegister = nullptr;
						uint8_t PinMask = 0;

					public:
						BitBangDriver()
							: IStripDriver()
						{}

						~BitBangDriver() = default;

						bool Setup()
						{
							PinMask = digitalPinToBitMask(OutputPin);
							Port = portOutputRegister(digitalPinToPort(OutputPin));
							PortRegister = portModeRegister(digitalPinToPort(OutputPin));
							*PortRegister |= PinMask;
							return true;
						}

						virtual bool CanPushBuffer() override
						{
							// Synchronous driver, always ready to push buffer.
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

							SendArrayMask(buffer, ByteCount, PinMask, Port);
						}
					};
				}

				template<uint16_t LedCount, uint8_t OutputPin>
				using BitBangRgbDriver = DetailSk6812::BitBangDriver<IntegerRay::ColorModel::Output::Grb, LedCount, OutputPin>;

				template<uint16_t LedCount, uint8_t OutputPin>
				using BitBangRgbwDriver = DetailSk6812::BitBangDriver<IntegerRay::ColorModel::Output::Grbw, LedCount, OutputPin>;
			}
		}
	}
}


#undef INTEGER_RAY_SK6812_DETAIL_W1_NOPS
#undef INTEGER_RAY_SK6812_DETAIL_W2_NOPS
#undef INTEGER_RAY_SK6812_DETAIL_W3_NOPS
#undef INTEGER_RAY_SK6812_DETAIL_NOP1
#undef INTEGER_RAY_SK6812_DETAIL_NOP2
#undef INTEGER_RAY_SK6812_DETAIL_NOP4
#undef INTEGER_RAY_SK6812_DETAIL_NOP8
#undef INTEGER_RAY_SK6812_DETAIL_NOP16



#endif
#endif