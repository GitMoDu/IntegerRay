#ifndef _INTEGER_RAY_ADAPTERS_INERTIA_FORMAT_h
#define _INTEGER_RAY_ADAPTERS_INERTIA_FORMAT_h

#include <IntegerRay.h>
#include <InertiaDrivers.h>

namespace IntegerRay
{
	namespace Adapters
	{
		namespace Inertia
		{
			namespace OutputFormat
			{
				using Rgb = ::Inertia::Drivers::Light::LedStrip::Sk6812::OutputFormat::Rgb;
				using Rgbw = ::Inertia::Drivers::Light::LedStrip::Sk6812::OutputFormat::Rgbw;
			}

			namespace IntegerRayFormat
			{
				using Rgb = IntegerRay::ColorModel::Output::Grb;
				using Rgbw = IntegerRay::ColorModel::Output::Grbw;
			}
		}
	}
}

#endif
