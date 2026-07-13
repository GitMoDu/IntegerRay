#ifndef _INTEGER_RAY_DRIVERS_WS2812B_AVR_SPI_DRIVER_h
#define _INTEGER_RAY_DRIVERS_WS2812B_AVR_SPI_DRIVER_h

#if defined(ARDUINO_ARCH_AVR)

#include <Arduino.h>
#include "../../../Model/Interface.h"

namespace IntegerRay
{
	namespace Drivers
	{
		namespace Ws2812b
		{
			namespace Avr
			{
#if F_CPU == 16000000UL
				/// <summary>
				/// 8 MHz SPI driver for WS2812B LEDs on 16 MHz AVR boards. 
				/// Interrupts are disabled during only most of the transmission.
				/// Much closer to spec than the 2 MHz driver, but only available on 16 MHz AVRs.
				/// </summary>
				/// <typeparam name="LedCount">Number of LEDs in the strip.</typeparam>
				template<uint16_t LedCount>
				class Spi8MhzRgbDriver : public IStripDriver
				{
				public:
					using color_t = IntegerRay::ColorModel::Output::Grb;
					using component_t = IntegerRay::ColorModel::Output::component_t;

				private:
					// At 8 MHz SPI, each WS2812B bit is represented by one SPI byte.
					static constexpr uint8_t EncodedZero = 0b11000000;
					static constexpr uint8_t EncodedOne = 0b11111000;
					static constexpr uint16_t ResetMicros = 60;

					static constexpr uint8_t ChannelCount = color_t::CHANNEL_COUNT;
					static constexpr size_t NativeByteCount = size_t(LedCount) * ChannelCount;

				private:
					uint32_t WaitStart = 0;
					bool Busy = false;

				public:
					Spi8MhzRgbDriver()
						: IStripDriver()
					{}

					~Spi8MhzRgbDriver() = default;

					bool Setup()
					{
						pinMode(SS, OUTPUT);
						digitalWrite(SS, HIGH);
						digitalWrite(SCK, LOW);
						digitalWrite(MOSI, LOW);
						pinMode(SCK, OUTPUT);
						pinMode(MOSI, OUTPUT);

						// Enable master mode 1, most-significant-bit first, at F_CPU / 2.
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

						for (size_t index = 0; index < NativeByteCount; ++index)
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

						// Reading SPSR followed by SPDR clears SPIF. Without this
						// read, the next transfer can observe the previous completion
						// flag and overwrite SPDR before the byte is transmitted.
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

				/// <summary>
				/// 2 MHz SPI driver for WS2812B LEDs on 8 MHz and 16 MHz AVR boards.
				/// Slightly off-spec, but works well enough for most strips.
				/// Interrupts are disabled during only most of the transfer.
				/// </summary>
				/// <typeparam name="LedCount">Number of LEDs in the strip.</typeparam>
				template<uint16_t LedCount>
				class Spi2MhzRgbDriver : public IStripDriver
				{
				public:
					using color_t = IntegerRay::ColorModel::Output::Grb;
					using component_t = IntegerRay::ColorModel::Output::component_t;

				private:
					static_assert(
						F_CPU == 8000000UL || F_CPU == 16000000UL,
						"IntegerRay WS2812 AVR 2 MHz SPI: supported CPU clocks are 8 MHz and 16 MHz.");

					static constexpr uint8_t ChannelCount =
						IntegerRay::ColorModel::Output::Grb::CHANNEL_COUNT;
					static constexpr size_t NativeByteCount =
						size_t(LedCount) * ChannelCount;
					static constexpr uint8_t EncodedZero = 0b10000000;
					static constexpr uint8_t EncodedOne = 0b11000000;
					static constexpr uint16_t ResetMicros = 60;

				private:
					uint32_t WaitStart = 0;
					bool Busy = false;

				public:
					Spi2MhzRgbDriver() : IStripDriver() {}

					~Spi2MhzRgbDriver() = default;

					bool Setup()
					{
						pinMode(SS, OUTPUT);
						digitalWrite(SS, HIGH);
						digitalWrite(SCK, LOW);
						digitalWrite(MOSI, LOW);
						pinMode(SCK, OUTPUT);
						pinMode(MOSI, OUTPUT);

						// Enable mode 1, most-significant-bit first, at 2 MHz.
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
						if (length < NativeByteCount || !CanPushBuffer())
						{
							return;
						}

						const uint8_t sregPrevious = SREG;
						cli();

						for (size_t index = 0; index < NativeByteCount; index++)
						{
							SendEncodedByte(buffer[index]);
						}

						SREG = sregPrevious;

						Busy = true;
						WaitStart = micros();
					}

				private:
					void SendSpiByte(
						const uint8_t value)
					{
						SPDR = value;
						while (!(SPSR & _BV(SPIF)))
						{
						}

						const uint8_t received = SPDR;
						(void)received;
					}

					void SendEncodedByte(
						const uint8_t value)
					{
						for (uint8_t mask = 0x80; mask; mask >>= 1)
						{
							SendSpiByte((value & mask) ? EncodedOne : EncodedZero);
						}
					}
				};
			}
		}
	}
}
#endif
#endif