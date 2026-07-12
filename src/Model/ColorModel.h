#ifndef _INTEGER_RAY_COLOR_MODEL_h
#define _INTEGER_RAY_COLOR_MODEL_h

// IntegerRay depends on IntegerSignal https://github.com/GitMoDu/IntegerSignal
#include <IntegerSignal.h>

namespace IntegerRay
{
	namespace ColorModel
	{
		namespace Internal
		{
			/// <summary>
			/// Internal color model for HDR processing.
			/// 15-bit fractional RGB color type (UQ0.15) from IntegerSignal::RgbFraction16.
			/// </summary>
			using color_t = IntegerSignal::RgbFraction16::color_t;

			/// <summary>
			/// Internal color model component type for HDR processing.
			/// 15-bit fractional RGB color component type (UQ0.15) from IntegerSignal::RgbFraction16.
			/// </summary>
			using component_t = IntegerSignal::RgbFraction16::component_t;

			static constexpr component_t COMPONENT_MAX = IntegerSignal::RgbFraction16::COMPONENT_MAX;

			static constexpr color_t WHITE = color_t{ COMPONENT_MAX, COMPONENT_MAX, COMPONENT_MAX };
			static constexpr color_t BLACK = color_t{ 0, 0, 0 };

			inline constexpr color_t FromRGB(const uint8_t red, const uint8_t green, const uint8_t blue)
			{
				return color_t{ IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint8_t>(red, UINT8_MAX),
								IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint8_t>(green, UINT8_MAX),
								IntegerSignal::FixedPoint::ScalarFraction::UFraction16::GetScalar<uint8_t>(blue, UINT8_MAX) };
			}
		}

		namespace Output
		{
			/// <summary>
			/// LED strip native color model for output. Only 8-bit RGB or RGBW is supported for output.
			/// </summary>
			using component_t = uint8_t;
			static constexpr uint8_t RGB_CHANNEL_COUNT = 3;
			static constexpr uint8_t RGBW_CHANNEL_COUNT = 4;

			constexpr component_t Component(const Internal::component_t component)
			{
				return IntegerSignal::FixedPoint::ScalarFraction::Fraction(component, component_t(UINT8_MAX));
			}

			constexpr component_t White(const component_t red, const component_t green, const component_t blue)
			{
				return IntegerSignal::MinValue(IntegerSignal::MinValue(red, green), blue);
			}

			/// <summary>RGB byte order.</summary>
			struct Rgb
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGB_CHANNEL_COUNT;

				/*static constexpr color_t Pack(const Internal::color_t color)
				{
					return (color_t(color.red) << 16)
						| (uint16_t(color.green) << 8)
						| color.blue;
				}*/

				static constexpr color_t Pack(const Internal::color_t color)
				{
					return (color_t(Component(color.red)) << 16)
						| (uint16_t(Component(color.green)) << 8)
						| Component(color.blue);
				}
			};

			/// <summary>RBG byte order.</summary>
			struct Rbg
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGB_CHANNEL_COUNT;

				static constexpr color_t Pack(const Internal::color_t color)
				{
					return (color_t(Component(color.red)) << 16)
						| (uint16_t(Component(color.blue)) << 8)
						| Component(color.green);
				}
			};

			/// <summary>GRB byte order.</summary>
			struct Grb
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGB_CHANNEL_COUNT;

				static constexpr color_t Pack(const Internal::color_t color)
				{
					return (color_t(Component(color.green)) << 16)
						| (uint16_t(Component(color.red)) << 8)
						| Component(color.blue);
				}
			};

			/// <summary>GBR byte order.</summary>
			struct Gbr
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGB_CHANNEL_COUNT;

				static constexpr color_t Pack(const Internal::color_t color)
				{
					return (color_t(Component(color.green)) << 16)
						| (uint16_t(Component(color.blue)) << 8)
						| Component(color.red);
				}
			};

			/// <summary>BRG byte order.</summary>
			struct Brg
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGB_CHANNEL_COUNT;

				static constexpr color_t Pack(const Internal::color_t color)
				{
					return (color_t(Component(color.blue)) << 16)
						| (uint16_t(Component(color.red)) << 8)
						| Component(color.green);
				}
			};

			/// <summary>BGR byte order.</summary>
			struct Bgr
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGB_CHANNEL_COUNT;

				static constexpr color_t Pack(const Internal::color_t color)
				{
					return (color_t(Component(color.blue)) << 16)
						| (uint16_t(Component(color.green)) << 8)
						| Component(color.red);
				}
			};

			/// <summary>RGBW byte order with shared white removed from RGB.</summary>
			struct Rgbw
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGBW_CHANNEL_COUNT;

				static inline color_t Pack(const Internal::color_t color)
				{
					const component_t red = Component(color.red);
					const component_t green = Component(color.green);
					const component_t blue = Component(color.blue);
					const component_t white = White(red, green, blue);
					return (color_t(red - white) << 24)
						| (color_t(green - white) << 16)
						| (uint16_t(blue - white) << 8)
						| white;
				}
			};

			/// <summary>GRBW byte order with shared white removed from RGB.</summary>
			struct Grbw
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGBW_CHANNEL_COUNT;

				static inline color_t Pack(const Internal::color_t color)
				{
					const component_t red = Component(color.red);
					const component_t green = Component(color.green);
					const component_t blue = Component(color.blue);
					const component_t white = White(red, green, blue);
					return (color_t(green - white) << 24)
						| (color_t(red - white) << 16)
						| (uint16_t(blue - white) << 8)
						| white;
				}
			};

			/// <summary>WRGB byte order with shared white removed from RGB.</summary>
			struct Wrgb
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGBW_CHANNEL_COUNT;

				static inline color_t Pack(const Internal::color_t color)
				{
					const component_t red = Component(color.red);
					const component_t green = Component(color.green);
					const component_t blue = Component(color.blue);
					const component_t white = White(red, green, blue);
					return (color_t(white) << 24)
						| (color_t(red - white) << 16)
						| (uint16_t(green - white) << 8)
						| (blue - white);
				}
			};

			/// <summary>WGRB byte order with shared white removed from RGB.</summary>
			struct Wgrb
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = RGBW_CHANNEL_COUNT;

				static inline color_t Pack(const Internal::color_t color)
				{
					const component_t red = Component(color.red);
					const component_t green = Component(color.green);
					const component_t blue = Component(color.blue);
					const component_t white = White(red, green, blue);
					return (color_t(white) << 24)
						| (color_t(green - white) << 16)
						| (uint16_t(red - white) << 8)
						| (blue - white);
				}
			};

			using Ws2812 = Grb;
			using Neopixel = Grb;
			using Sk6812Rgb = Grb;
			using Sk6812Rgbw = Grbw;


		}
	}

	// Expose internal color model.
	using color_t = ColorModel::Internal::color_t;

	// Expose internal color model component type.
	using component_t = ColorModel::Internal::component_t;


	/// <summary>
	/// Convert one HDR color to packed native strip byte order.
	/// </summary>
	template<typename format_t>
	inline typename format_t::color_t ConvertColor(const color_t color)
	{
		return format_t::Pack(color);
	}

	/// <summary>
	/// Convert an HDR buffer to a native strip byte buffer in a single pass.
	/// </summary>
	template<typename format_t>
	inline void ConvertBuffer(const color_t* source, ColorModel::Output::component_t* destination, const uint16_t pixelCount)
	{
		for (uint16_t index = 0; index < pixelCount; index++)
		{
			const typename format_t::color_t nativeColor = format_t::Pack(source[index]);
			const uint16_t offset = index * format_t::CHANNEL_COUNT;

			for (uint8_t channel = 0; channel < format_t::CHANNEL_COUNT; channel++)
			{
				const uint8_t shift = (format_t::CHANNEL_COUNT - 1 - channel) << 3;
				destination[offset + channel] = ColorModel::Output::component_t(nativeColor >> shift);
			}
		}
	}
}



#endif

