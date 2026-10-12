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

		namespace Grading
		{
			/// <summary>
			/// Transforms an internal high-precision color before native output conversion.
			/// </summary>
			struct IColorGrader
			{
				virtual ~IColorGrader() = default;

				virtual Internal::color_t Grade(const Internal::color_t color, const uint16_t pixelIndex) const = 0;
			};
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

			/// <summary>
			/// Intensity, blue, green, red byte order with shared white removed from RGB.
			/// APA102 standard BGR byte order utilizing dynamic Global Current scaling.
			/// </summary>
			struct Ibgr
			{
				using color_t = uint32_t;
				static constexpr uint8_t CHANNEL_COUNT = 4;

				static inline color_t Pack(const Internal::color_t color)
				{
					// Find peak component value to maximize the global current envelope range
					const uint16_t max_val = IntegerSignal::MaxValue(IntegerSignal::MaxValue(color.red, color.green), color.blue);
					using Scalar = IntegerSignal::FixedPoint::ScalarFraction::UFraction16;

					if (max_val == 0)
					{
						// Return base frame markers with entirely dark channels
						return color_t(0xE0) << 24;
					}

					// 1. Map the maximum channel value into the 5-bit global range (1 to 31)
					uint8_t g_step = Scalar::Fraction(max_val, uint8_t(31));
					g_step = IntegerSignal::MaxValue(g_step, uint8_t(1));

					// 2. Compute the exact scale ceiling for the chosen global register setting
					// This acts as the scalar denominator for our normalization step
					const uint16_t scalar_denominator = Scalar::Fraction(Scalar::GetScalar<uint8_t>(g_step, uint8_t(31)), Internal::COMPONENT_MAX);

					// 3. Upscale components into the maximum resolution boundaries of the 8-bit space
					const uint8_t r_8 = Scalar::Fraction(
						Scalar::GetScalar<uint16_t>(color.red, scalar_denominator), uint8_t(UINT8_MAX));
					const uint8_t g_8 = Scalar::Fraction(
						Scalar::GetScalar<uint16_t>(color.green, scalar_denominator), uint8_t(UINT8_MAX));
					const uint8_t b_8 = Scalar::Fraction(
						Scalar::GetScalar<uint16_t>(color.blue, scalar_denominator), uint8_t(UINT8_MAX));

					// 4. Pack directly into a 32-bit word matching ConvertBuffer's extraction order
					// channel 0 (shift 24) = Global (with 11100000 marker configuration)
					// channel 1 (shift 16) = Blue
					// channel 2 (shift 8)  = Green
					// channel 3 (shift 0)  = Red
					return (color_t(0xE0 | g_step) << 24)
						| (color_t(b_8) << 16)
						| (uint16_t(g_8) << 8)
						| r_8;
				}
			};
		}
	}

	// Expose internal color model.
	using color_t = ColorModel::Internal::color_t;

	// Expose internal color model component type.
	using component_t = ColorModel::Internal::component_t;

	static constexpr component_t COMPONENT_MAX = ColorModel::Internal::COMPONENT_MAX;

	static constexpr color_t COLOR_WHITE = ColorModel::Internal::WHITE;

	static constexpr color_t COLOR_BLACK = ColorModel::Internal::BLACK;


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

