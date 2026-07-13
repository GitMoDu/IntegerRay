#ifndef _INTEGER_RAY_DRIVERS_SK6812_AVR_SPI_DRIVER_h
#define _INTEGER_RAY_DRIVERS_SK6812_AVR_SPI_DRIVER_h

#if defined(ARDUINO_ARCH_AVR)

#include <Arduino.h>
#include "../../../Model/Interface.h"

namespace IntegerRay
{
	namespace Drivers
	{
		namespace Sk6812
		{
			namespace Avr
			{
				namespace Detail
				{
#if F_CPU == 16000000UL
					/// <summary>
					/// 8 MHz SPI driver for SK6812 LEDs on 16 MHz AVR boards.
					/// </summary>
					/// <typeparam name="ColorType">Packed SK6812 output color format.</typeparam>
					/// <typeparam name="LedCount">Number of LEDs in the strip.</typeparam>
					template<typename ColorType, uint16_t LedCount>
					class Spi8MhzDriver : public IStripDriver
					{
					private:
						static constexpr uint8_t EncodedZero = 0b11000000;
						static constexpr uint8_t EncodedOne = 0b11111000;
						static constexpr uint16_t ResetMicros = 80;
						static constexpr uint8_t ChannelCount = ColorType::CHANNEL_COUNT;
						static constexpr size_t NativeByteCount =
							size_t(LedCount) * ChannelCount;

					public:
						using color_t = ColorType;
						using component_t = IntegerRay::ColorModel::Output::component_t;

					private:
						uint32_t WaitStart = 0;
						bool Busy = false;

					public:
						Spi8MhzDriver()
							: IStripDriver()
						{}

						~Spi8MhzDriver() = default;

						bool Setup()
						{
							pinMode(SS, OUTPUT);
							digitalWrite(SS, HIGH);
							digitalWrite(SCK, LOW);
							digitalWrite(MOSI, LOW);
							pinMode(SCK, OUTPUT);
							pinMode(MOSI, OUTPUT);

							SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPHA);
							SPSR = _BV(SPI2X);
							const uint8_t status = SPSR;
							const uint8_t received = SPDR;
							(void)status;
							(void)received;

							Busy = false;

							return true;
						}

						virtual bool CanPushBuffer() override
						{
							if (Busy)
							{
								if (micros() - WaitStart > ResetMicros)
								{
									Busy = false;
								}
							}

							return !Busy;
						}

						virtual uint16_t GetCount() override
						{
							return LedCount;
						}

						virtual void PushBuffer(
							const ColorModel::Output::component_t* buffer,
							const size_t length) override
						{
							if (length < NativeByteCount)
							{
								return;
							}

							const uint8_t sregPrevious = SREG;
							cli();

							for (size_t index = 0;
								index < NativeByteCount;
								++index)
							{
								SendEncodedByte(buffer[index]);
							}

							SREG = sregPrevious;

							Busy = true;
							WaitStart = micros();
						}

					private:
						void SendSpiByte(const uint8_t value)
						{
							SPDR = value;
							while (!(SPSR & _BV(SPIF)))
							{
							}

							const uint8_t received = SPDR;
							(void)received;
						}

						void SendEncodedByte(const uint8_t value)
						{
							for (uint8_t mask = 0x80; mask; mask >>= 1)
							{
								if (value & mask)
								{
									SendSpiByte(EncodedOne);
									asm volatile("nop\n\t"
										"nop\n\t");
								}
								else
								{
									SendSpiByte(EncodedZero);
								}
							}
						}
					};
#endif

					template<typename ColorType, uint16_t LedCount>
					class Spi2MhzDriver : public IStripDriver
					{
					private:
						static_assert(
							F_CPU == 8000000UL || F_CPU == 16000000UL,
							"IntegerRay SK6812 AVR 2 MHz SPI: supported CPU clocks are 8 MHz and 16 MHz.");

						static constexpr uint8_t EncodedZero = 0b10000000;
						static constexpr uint8_t EncodedOne = 0b11000000;
						static constexpr uint16_t ResetMicros = 60;
						static constexpr uint8_t ChannelCount = ColorType::CHANNEL_COUNT;
						static constexpr size_t NativeByteCount =
							size_t(LedCount) * ChannelCount;

					public:
						using color_t = ColorType;
						using component_t = IntegerRay::ColorModel::Output::component_t;

					private:
						uint32_t WaitStart = 0;
						bool Busy = false;

					public:
						Spi2MhzDriver()
							: IStripDriver()
						{}

						~Spi2MhzDriver() = default;

						bool Setup()
						{
							pinMode(SS, OUTPUT);
							digitalWrite(SS, HIGH);
							digitalWrite(SCK, LOW);
							digitalWrite(MOSI, LOW);
							pinMode(SCK, OUTPUT);
							pinMode(MOSI, OUTPUT);

#if F_CPU == 16000000UL
							SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPHA) | _BV(SPR0);
							SPSR = _BV(SPI2X);
#else
							SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPHA);
							SPSR = _BV(SPI2X);
#endif
							const uint8_t status = SPSR;
							const uint8_t received = SPDR;
							(void)status;
							(void)received;

							Busy = false;

							return true;
						}

						virtual bool CanPushBuffer() override
						{
							if (Busy)
							{
								if (micros() - WaitStart > ResetMicros)
								{
									Busy = false;
								}
							}

							return !Busy;
						}

						virtual uint16_t GetCount() override
						{
							return LedCount;
						}

						virtual void PushBuffer(
							const ColorModel::Output::component_t* buffer,
							const size_t length) override
						{
							if (length < NativeByteCount)
							{
								return;
							}

							const uint8_t sregPrevious = SREG;
							cli();

							for (size_t index = 0;
								index < NativeByteCount;
								++index)
							{
								SendEncodedByte(buffer[index]);
							}

							SREG = sregPrevious;
							delayMicroseconds(ResetMicros);
						}

					private:
						void SendSpiByte(const uint8_t value)
						{
							SPDR = value;
							while (!(SPSR & _BV(SPIF)))
							{
							}

							const uint8_t received = SPDR;
							(void)received;
						}

						void SendEncodedByte(const uint8_t value)
						{
							for (uint8_t mask = 0x80; mask; mask >>= 1)
							{
								SendSpiByte((value & mask)
									? EncodedOne
									: EncodedZero);
							}
						}
					};
				}

#if F_CPU == 16000000UL
				template<uint16_t LedCount>
				using Spi8MhzRgbDriver = Detail::Spi8MhzDriver<
					IntegerRay::ColorModel::Output::Grb,
					LedCount>;

				template<uint16_t LedCount>
				using Spi8MhzRgbwDriver = Detail::Spi8MhzDriver<
					IntegerRay::ColorModel::Output::Grbw,
					LedCount>;
#endif

				template<uint16_t LedCount>
				using Spi2MhzRgbDriver = Detail::Spi2MhzDriver<
					IntegerRay::ColorModel::Output::Grb,
					LedCount>;

				template<uint16_t LedCount>
				using Spi2MhzRgbwDriver = Detail::Spi2MhzDriver<
					IntegerRay::ColorModel::Output::Grbw,
					LedCount>;
			}
		}
	}
}
#endif
#endif