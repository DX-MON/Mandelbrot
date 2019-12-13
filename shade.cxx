#include <array>
#include <math.h>
#include "mandelbrot.hxx"
#include "shade.hxx"

std::unique_ptr<rgb8_t []> image{nullptr};
std::array<floatRGB_t, 16> colours
{{
	rgb8_t{0x07, 0x00, 0x5D},
	rgb8_t{0x11, 0x19, 0x87},
	rgb8_t{0x1E, 0x4A, 0xAC},
	rgb8_t{0x43, 0x76, 0xCD},
	rgb8_t{0x86, 0xAF, 0xE1},
	rgb8_t{0xD0, 0xE8, 0xF7},
	rgb8_t{0xED, 0xE7, 0xBE},
	rgb8_t{0xF5, 0xC9, 0x5A},
	rgb8_t{0xFD, 0xA8, 0x01},
	rgb8_t{0xC8, 0x81, 0x01},
	rgb8_t{0x94, 0x54, 0x00},
	rgb8_t{0x64, 0x31, 0x01},
	rgb8_t{0x42, 0x12, 0x06},
	rgb8_t{0x0E, 0x03, 0x0E},
	rgb8_t{0x05, 0x00, 0x26},
	rgb8_t{0x05, 0x00, 0x47}
}};

std::unique_ptr<std::atomic<uint32_t> []> imageStatus{nullptr};
std::mutex imageMutex;
std::condition_variable imageSync;

inline floatRGB_t linearEase(const floatRGB_t &a, const floatRGB_t &b, const double amount) noexcept
	{ return a + (b * amount); }

rgb8_t colourFor(const double x) noexcept
{
	const auto normX = fmod(x, colours.size());
	const uint8_t i = uint8_t(normX);
	const auto frac = normX - floor(normX);

	const auto colour1 = colours[i];
	const auto colour2 = colours[(i + 1) % colours.size()] - colour1;
	return linearEase(colour1, colour2, frac).toRGB8();
}

inline rgb8_t shade(const double i) noexcept
{
	if (i >= maxIterations)
		return {};
	return colourFor(i / 6.0);
}

inline size_t xy(const area_t point, const area_t &size) noexcept
{
	size_t x = point.width();
	size_t y = point.height() * size.width();
	return y + x;
}

void shadeChunk(const area_t size, const area_t subchunk, const uint32_t subdiv,
	stream_t &stream, const uint32_t affinityOffset) noexcept
{
	area_t offset;
	if (!image)
		return;
	threadAffinity(affinityOffset);
	if (!stream.read(offset))
		abort();
	offset *= subchunk;

	printf("Shader launched for %u, %u\n", offset.width(), offset.height());
	for (uint32_t y{0}; y < subchunk.height(); ++y)
	{
		for (uint32_t x{0}; x < subchunk.width(); ++x)
		{
			rgb16_t colour;
			for (size_t i{0}; i < subdiv; ++i)
			{
				double point = 0.0;
				read(stream, point);
				if (i == 0)
					colour = shade(point);
				else
				{
					colour += shade(point);
					colour /= 2;
				}
			}
			image[xy(area_t{x, y} + offset, size)] = colour;
		}
		++imageStatus[y + offset.height()];
		if (imageStatus[y + offset.height()] == xTiles)
			imageSync.notify_all();
	}
	puts("Shader done");
}
