#ifndef _INTEGER_RAY_DRIVERS_SK6812_STM32_DMA_DRIVER_h
#define _INTEGER_RAY_DRIVERS_SK6812_STM32_DMA_DRIVER_h

#if defined(ARDUINO_ARCH_STM32F1)
#include <Arduino.h>
#include <SPI.h>
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
					template<typename ColorType, uint16_t LedCount>
					class SpiDmaDriver : public IStripDriver
					{
					public:
						using color_t = ColorType;
						using component_t = IntegerRay::ColorModel::Output::component_t;

					private:
						struct SpiEncoding
						{
							// Each SK6812 data bit is represented by three SPI bits:
							// 100 for zero and 110 for one. At 2.25 MHz, each SPI bit
							// is approximately 444 ns and each encoded data bit is
							// approximately 1.33 us.
							static constexpr uint8_t BitsPerNativeBit = 3;
							static constexpr uint8_t EncodedZero = 0b100;
							static constexpr uint8_t EncodedOne = 0b110;
							static constexpr uint16_t ResetPaddingBytes = 1;
							static constexpr uint32_t SpiBitRate = 2250000;
						};

					private:
						static constexpr uint8_t ChannelCount = color_t::CHANNEL_COUNT;
						static constexpr size_t NativeByteCount = size_t(LedCount) * ChannelCount;
						static constexpr size_t EncodedByteCount = (NativeByteCount * SpiEncoding::BitsPerNativeBit) + (SpiEncoding::ResetPaddingBytes * 2);

						SPIClass& Spi;
						volatile bool TransferActive;
						uint8_t EncodedBuffer[EncodedByteCount];

					public:
						SpiDmaDriver(SPIClass& spi = SPI)
							: IStripDriver()
							, Spi(spi)
							, TransferActive(false)
							, EncodedBuffer()
						{}

						~SpiDmaDriver() = default;

						bool Setup()
						{
							Spi.setClockDivider(ClockDivider(Spi));
							Spi.begin();
							return true;
						}

						void HandleTransmitComplete()
						{
							TransferActive = false;
						}

						virtual bool CanPushBuffer() override
						{
							return !TransferActive;
						}

						virtual uint16_t GetCount() override
						{
							return LedCount;
						}

						virtual void PushBuffer(const ColorModel::Output::component_t* buffer, const size_t length) override
						{
							if (length < NativeByteCount)
							{
								return;
							}

							Encode(buffer, NativeByteCount, EncodedBuffer);
							TransferActive = true;
							Spi.dmaSendAsync(EncodedBuffer, EncodedByteCount);
						}

					private:
						static inline uint32_t PeripheralClock(SPIClass& spi)
						{
							switch (rcc_dev_clk(spi.c_dev()->clk_id))
							{
							case RCC_APB2:
								return STM32_PCLK2;

							case RCC_APB1:
								return STM32_PCLK1;

							default:
								return STM32_PCLK2;
							}
						}

						static inline uint32_t ClockDivider(SPIClass& spi)
						{
							// This SPI API expresses divisors as 2^(index + 1).
							// Select the fastest supported divisor that does not exceed
							// the nominal SK6812 encoding clock.
							uint32_t clock = PeripheralClock(spi) / 2;
							uint8_t index = 0;

							while (index < 7 && SpiEncoding::SpiBitRate < clock)
							{
								clock /= 2;
								++index;
							}

							return index << 3;
						}

						static inline void EncodeByte(uint8_t*& output, const uint8_t value)
						{
							uint32_t encoded = 0;

							for (uint8_t mask = 0x80; mask; mask >>= 1)
							{
								encoded <<= SpiEncoding::BitsPerNativeBit;
								encoded |= (value & mask) ? SpiEncoding::EncodedOne : SpiEncoding::EncodedZero;
							}

							*output++ = uint8_t(encoded >> 16);
							*output++ = uint8_t(encoded >> 8);
							*output++ = uint8_t(encoded);
						}

						static inline void Encode(const ColorModel::Output::component_t* input, const size_t inputLength, uint8_t* output)
						{
							*output++ = 0;

							for (size_t index = 0; index < inputLength; ++index)
							{
								EncodeByte(output, input[index]);
							}

							*output++ = 0;
						}
					};
				}

				template<uint16_t LedCount>
				using SpiDmaRgbDriver = Detail::SpiDmaDriver<IntegerRay::ColorModel::Output::Grb, LedCount>;

				template<uint16_t LedCount>
				using SpiDmaRgbwDriver = Detail::SpiDmaDriver<IntegerRay::ColorModel::Output::Grbw, LedCount>;
			}
		}
	}
}



#endif
#endif
