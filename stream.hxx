#ifndef STREAM__HXX
#define STREAM__HXX

#include <stddef.h>

struct stream_t
{
public:
	stream_t() = default;
	stream_t(const stream_t &) = delete;
	stream_t(stream_t &&) = default;
	virtual ~stream_t() = default;
	stream_t &operator =(const stream_t &) = delete;
	stream_t &operator =(stream_t &&) = default;

	template<typename T> bool read(T &value)
		{ return read(&value, sizeof(T)); }
	template<typename T, size_t N> bool read(std::array<T, N> &value)
		{ return read(value.data(), N); }

	template<typename T> bool write(const T &value)
		{ return write(&value, sizeof(T)); }
	template<typename T, size_t N> bool write(const std::array<T, N> &value)
		{ return write(value.data(), N); }

	bool read(void *const value, const size_t valueLen)
	{
		size_t actualLen = 0;
		if (!read(value, valueLen, actualLen))
			return false;
		return valueLen == actualLen;
	}

	virtual bool read(void *const, const size_t, size_t &) = 0;
	virtual bool write(const void *const, const size_t) = 0;
};

#endif /*STREAM__HXX*/
