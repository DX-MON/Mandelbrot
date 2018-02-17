#include <png.h>
#include <zlib.h>
#include "shade.hxx"
#include "pngWriter.hxx"
#include "file.hxx"

file_t file;
png_structp png;
png_infop info;

bool openPNG(const area_t size) noexcept
{
	file = fopen("mandelbrot.png", "wb");
	if (!file.valid())
		return false;

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	info = png_create_info_struct(png);
	if (!png || !info)
	{
		if (png)
			png_destroy_write_struct(&png, nullptr);
		file.close();
		return false;
	}
	png_init_io(png, file);
	png_set_compression_level(png, Z_BEST_COMPRESSION);
	png_set_IHDR(png, info, size.width(), size.height(), 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	return true;
}

void closePNG() noexcept
{
	png_write_end(png, info);
	png_destroy_write_struct(&png, &info);
	file.close();
}

void writePNGRow(const uint32_t row, const uint32_t width) noexcept
{
	png_bytep const rowData = reinterpret_cast<png_bytep>(&image[row * width]);
	png_write_row(png, rowData);
}
