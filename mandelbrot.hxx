#ifndef MANDELBROT__HXX
#define MANDELBROT__HXX

#include <stdint.h>
#include <utility>
#include <vector>
#include "stream.hxx"
#include "fixedVector.hxx"

struct area_t final
{
private:
	uint32_t _width, _height;

public:
	constexpr area_t() noexcept : _width(0), _height(0) { }
	constexpr area_t(const uint32_t x, const uint32_t y) noexcept : _width(x), _height(y) { }

	uint32_t width() const noexcept { return _width; }
	void width(const uint32_t width) noexcept { _width = width; }
	uint32_t height() const noexcept { return _height; }
	void height(const uint32_t height) noexcept { _height = height; }

	area_t operator +(const area_t point) const noexcept
		{ return {_width + point._width, _height + point._height}; }
	area_t operator -(const area_t point) const noexcept
		{ return {_width - point._width, _height - point._height}; }
	area_t operator *(const area_t point) const noexcept
		{ return {_width * point._width, _height * point._height}; }
	area_t operator /(const area_t point) const noexcept
		{ return {_width / point._width, _height / point._height}; }

	void operator *=(const area_t point) noexcept
		{ _width *= point._width; _height *= point._height; }

	void swap(area_t &point) noexcept
	{
		std::swap(_width, point._width);
		std::swap(_height, point._height);
	}
};

struct point2_t final
{
private:
	double _x, _y;

public:
	constexpr point2_t() noexcept : _x(0), _y(0) { }
	constexpr point2_t(const double x, const double y) noexcept : _x(x), _y(y) { }

	double x() const noexcept { return _x; }
	void x(const double x) noexcept { _x = x; }
	double y() const noexcept { return _y; }
	void y(const double y) noexcept { _y = y; }

	point2_t operator +(const point2_t point) const noexcept
		{ return {_x + point._x, _y + point._y}; }
	point2_t operator -(const point2_t point) const noexcept
		{ return {_x - point._x, _y - point._y}; }
	point2_t operator *(const point2_t point) const noexcept
		{ return {_x * point._x, _y * point._y}; }
	point2_t operator /(const point2_t point) const noexcept
		{ return {_x / point._x, _y / point._y}; }
	point2_t operator -() const noexcept
		{ return {-_x, -_y}; }

	point2_t operator *(const area_t point) const noexcept
		{ return {_x * point.width(), _y * point.height()}; }

	point2_t operator /(const uint32_t scalar) const noexcept
		{ return {_x / scalar, _y / scalar}; }

	double mul() const noexcept { return _x * _y; }
	double sum() const noexcept { return _x + _y; }
	double diff() const noexcept { return _x - _y; }

	void swap(point2_t &point) noexcept
	{
		std::swap(_x, point._x);
		std::swap(_y, point._y);
	}
};

inline point2_t operator /(const area_t a, const point2_t b) noexcept
	{ return {a.width() / b.x(), a.height() / b.y()}; }

constexpr static const int16_t maxIterations = 1000;
extern uint32_t width, height;
extern uint32_t xTiles, yTiles;
extern std::vector<uint32_t> availableProcessors;

void computeChunk(const area_t size, const area_t offset, const point2_t scale,
	const point2_t center, const uint32_t subdiv, stream_t &stream) noexcept;

inline void threadAffinity(const uint32_t affinityOffset) noexcept
{
	cpu_set_t affinity = {};
	CPU_ZERO(&affinity);
	CPU_SET(availableProcessors[affinityOffset % availableProcessors.size()], &affinity);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &affinity);
}

template<typename T> bool read(stream_t &stream, T &value)
	{ return stream.read(&value, sizeof(T)); }
template<typename T> bool read(stream_t &stream, fixedVector_t<T> &value)
	{ return stream.read(value.data(), sizeof(T) * value.count()); }
template<typename T> bool write(stream_t &stream, const T &value)
	{ return stream.write(&value, sizeof(T)); }
template<typename T> bool write(stream_t &stream, const fixedVector_t<T> &value)
	{ return stream.write(value.data(), sizeof(T) * value.count()); }

#endif /*MANDELBROT__HXX*/
