#ifndef SHADE__HXX
#define SHADE__HXX

#include <stdint.h>
#include <type_traits>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "mandelbrot.hxx"

struct floatRGB_t;

template<typename T> struct rgb_t final
{
protected:
	T _r, _g, _b;
	constexpr static T max = std::numeric_limits<T>::max();

public:
	using type = T;
	constexpr rgb_t() noexcept : _r(0), _g(0), _b(0) { }
	constexpr rgb_t(const T r, const T g, const T b) noexcept : _r(r), _g(g), _b(b) { }
	template<typename U, typename = typename std::enable_if<!std::is_same<T, U>::value>::type>
		constexpr rgb_t(const rgb_t<U> &pixel) noexcept : _r(pixel.r()), _g(pixel.g()), _b(pixel.b()) { }

	rgb_t<T> operator +(const rgb_t<T> &pixel) const noexcept
		{ return {T(_r + pixel._r), T(_g + pixel._g), T(_b + pixel._b)}; }
	rgb_t<T> operator -(const rgb_t<T> &pixel) const noexcept
		{ return {T(_r - pixel._r), T(_g - pixel._g), T(_b - pixel._b)}; }

	template<typename U> void operator +=(const rgb_t<U> &pixel) noexcept
	{
		_r += pixel.r();
		_g += pixel.g();
		_b += pixel.b();
	}

	rgb_t<T> operator *(const double scalar) const noexcept
		{ return {T(_r * scalar), T(_g * scalar), T(_b * scalar)}; }

	void operator /=(const uint32_t scalar) noexcept
	{
		_r /= scalar;
		_g /= scalar;
		_b /= scalar;
	}

	constexpr T r() const noexcept { return _r; }
	constexpr T g() const noexcept { return _g; }
	constexpr T b() const noexcept { return _b; }

	constexpr double floatR() const noexcept { return _r * 2.2 / max; }
	constexpr double floatG() const noexcept { return _g * 2.2 / max; }
	constexpr double floatB() const noexcept { return _b * 2.2 / max; }
};

using rgb8_t = rgb_t<uint8_t>;
using rgb16_t = rgb_t<uint16_t>;

struct floatRGB_t final
{
protected:
	double _r, _g, _b;

	constexpr static uint8_t clamp(const double value) noexcept
		{ return std::max<double>(0, std::min<double>(255, value)); }

	constexpr static uint8_t scaleClamp(const double value) noexcept
		{ return clamp(value * 255); }

public:
	constexpr floatRGB_t() noexcept : _r(0), _g(0), _b(0) { }
	constexpr floatRGB_t(const double r, const double g, const double b) noexcept : _r(r), _g(g), _b(b) { }
	constexpr floatRGB_t(const rgb8_t &pixel) noexcept : _r(pixel.floatR()), _g(pixel.floatG()), _b(pixel.floatB()) { }

	floatRGB_t operator +(const floatRGB_t &pixel) const noexcept
		{ return {_r + pixel._r, _g + pixel._g, _b + pixel._b}; }
	floatRGB_t operator -(const floatRGB_t &pixel) const noexcept
		{ return {_r - pixel._r, _g - pixel._g, _b - pixel._b}; }

	floatRGB_t operator *(const double scalar) const noexcept
		{ return {_r * scalar, _g * scalar, _b * scalar}; }
	floatRGB_t operator /(const double scalar) const noexcept
		{ return {_r / scalar, _g / scalar, _b / scalar}; }

	double r() const noexcept { return _r; }
	double g() const noexcept { return _g; }
	double b() const noexcept { return _b; }

	rgb8_t toRGB8() const noexcept
	{
		const auto colour = *this / 2.2;
		return {scaleClamp(colour.r()), scaleClamp(colour.g()), scaleClamp(colour.b())};
	}
};

extern std::unique_ptr<rgb8_t []> image;
extern std::unique_ptr<std::atomic<uint32_t> []> imageStatus;
extern std::mutex imageMutex;
extern std::condition_variable imageSync;
extern uint32_t xTiles;

void shadeChunk(const area_t size, const area_t subchunk, const uint32_t subdiv,
	stream_t &stream, const uint32_t affinityOffset) noexcept;

#endif /*SHADE__HXX*/
