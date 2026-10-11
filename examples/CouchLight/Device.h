#ifndef _DEVICE_h
#define _DEVICE_h

#include <Arduino.h>
#include <SPI.h>

namespace Device
{
	namespace Button
	{
		//static constexpr uint8_t Pin = 8;
		static constexpr uint8_t Pin = PA3;
	}

	namespace StripTop
	{
		static constexpr uint16_t LedCount = 207;
		auto Spi = SPIClass(2);
		static constexpr uint8_t Pin = PB15;
		using DriverType = IntegerRay::Drivers::Sk6812::Stm32::SpiDmaRgbwDriver<LedCount>;
	}

	namespace StripBottom
	{
		static constexpr uint16_t LedCount = 209;
		auto& Spi = SPI;
		static constexpr uint8_t Pin = PA7;
		using DriverType = IntegerRay::Drivers::Sk6812::Stm32::SpiDmaRgbwDriver<LedCount>;
	}

	namespace Display
	{
		static constexpr uint32_t TargetPeriodMicros = 4000;


	}
}

#endif

