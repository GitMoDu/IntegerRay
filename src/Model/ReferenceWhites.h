#ifndef _INTEGER_RAY_MODEL_REFERENCE_WHITES_h
#define _INTEGER_RAY_MODEL_REFERENCE_WHITES_h

#include "ColorModel.h"

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace ReferenceWhites
		{
			using component_t = Internal::component_t;
			using color_t = Internal::color_t;
			using hsv_t = IntegerSignal::Hsv::hsv_fraction_t;

			enum class id_t : uint8_t
			{
				IlluminantA,
				IlluminantD50,
				IlluminantD55,
				IlluminantD65,
				IlluminantD75,
				Temperature2700K,
				Temperature3000K,
				Temperature4000K,
				Temperature5000K,
				Temperature6500K
			};

			/// <summary>
			/// Display-oriented approximation of a reference white, stored as UQ0.15 RGB and HSV.
			/// RGB values are sRGB approximations; HSV is derived from the listed RGB value.
			/// </summary>
			struct reference_white_t
			{
				id_t id;
				uint16_t kelvin;
				color_t rgb;
				hsv_t hsv;
			};

			constexpr reference_white_t Make(const id_t id, const uint16_t kelvin,
				const uint8_t red, const uint8_t green, const uint8_t blue,
				const uint16_t hueNumerator, const uint16_t hueDenominator,
				const uint16_t saturationNumerator, const uint16_t saturationDenominator)
			{
				return {
					id,
					kelvin,
					Internal::FromRGB(red, green, blue),
					{
						IntegerSignal::Trigonometry::angle_t(
							(uint32_t(hueNumerator) * IntegerSignal::Trigonometry::ANGLE_RANGE) / hueDenominator),
						IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint16_t>(
							 saturationNumerator, saturationDenominator),
						Internal::COMPONENT_MAX
					}
				};
			}

			static constexpr reference_white_t IlluminantA = Make(id_t::IlluminantA, 2856, 255, 180, 107, 73, 888, 148, 255);
			static constexpr reference_white_t IlluminantD50 = Make(id_t::IlluminantD50, 5003, 255, 244, 230, 14, 150, 25, 255);
			static constexpr reference_white_t IlluminantD55 = Make(id_t::IlluminantD55, 5503, 255, 248, 239, 9, 96, 16, 255);
			static constexpr reference_white_t IlluminantD65 = Make(id_t::IlluminantD65, 6504, 255, 255, 255, 0, 1, 0, 1);
			static constexpr reference_white_t IlluminantD75 = Make(id_t::IlluminantD75, 7504, 240, 243, 255, 19, 30, 15, 255);

			static constexpr reference_white_t Temperature2700K = Make(id_t::Temperature2700K, 2700, 255, 180, 107, 73, 888, 148, 255);
			static constexpr reference_white_t Temperature3000K = Make(id_t::Temperature3000K, 3000, 255, 196, 137, 1, 12, 118, 255);
			static constexpr reference_white_t Temperature4000K = Make(id_t::Temperature4000K, 4000, 255, 219, 186, 11, 138, 69, 255);
			static constexpr reference_white_t Temperature5000K = Make(id_t::Temperature5000K, 5000, 255, 236, 216, 10, 117, 39, 255);
			static constexpr reference_white_t Temperature6500K = Make(id_t::Temperature6500K, 6500, 255, 255, 255, 0, 1, 0, 1);

			static constexpr reference_white_t StandardIlluminants[] = {
				IlluminantA,
				IlluminantD50,
				IlluminantD55,
				IlluminantD65,
				IlluminantD75
			};

			static constexpr reference_white_t TemperaturePresets[] = {
				Temperature2700K,
				Temperature3000K,
				Temperature4000K,
				Temperature5000K,
				Temperature6500K
			};
		}
	}
}

#endif
