#ifndef PNG_WRITER__HXX
#define PNG_WRITER__HXX

#include "mandelbrot.hxx"
#include "shade.hxx"

bool openPNG(const area_t size) noexcept;
void closePNG() noexcept;
void writePNGRow(const uint32_t row, const uint32_t width) noexcept;

#endif /*PNG_WRITER__HXX*/
