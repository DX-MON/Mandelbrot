#ifndef RING_BUFFER__HXX
#define RING_BUFFER__HXX

#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <functional>
#include <rSON.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
using std::operator ""us;
#pragma GCC diagnostic pop

constexpr inline uint32_t operator ""_kB(const unsigned long long int value) noexcept
	{ return value * 1024; }

template<typename T> struct ringBuffer_t final
{
private:
	constexpr static uint32_t maxEntries = 4_kB / sizeof(T);
	std::atomic<uint32_t> count;
	uint32_t readIndex, writeIndex;
	std::unique_ptr<T []> buffer;
	mutable std::mutex bufferMutex;
	mutable std::condition_variable bufferCond;

	void wait(const uint32_t threshold) const noexcept
	{
		std::unique_lock<std::mutex> lock(bufferMutex);
		bufferCond.wait(lock, [&]() noexcept { return count != threshold; });
	}

public:
	ringBuffer_t() : count{0}, readIndex{0}, writeIndex{0}, buffer{std::make_unique<T []>(maxEntries)},
		bufferMutex{}, bufferCond{} { }

	void read(T &value) noexcept
	{
		wait(0);
		value = buffer[readIndex++];
		readIndex %= maxEntries;
		--count;
		bufferCond.notify_all();
	}

	void write(const T &value) noexcept
	{
		wait(maxEntries);
		buffer[writeIndex++] = value;
		writeIndex %= maxEntries;
		++count;
		bufferCond.notify_all();
	}
};

struct ringStream_t final : rSON::stream_t
{
private:
	constexpr static uint32_t maxEntries = 4_kB;
	std::atomic<uint32_t> count;
	uint32_t readIndex, writeIndex;
	std::unique_ptr<char []> buffer;
	mutable std::mutex bufferMutex;
	mutable std::condition_variable bufferCond;

	void wait(std::function<bool()> cond) const noexcept
	{
		std::unique_lock<std::mutex> lock(bufferMutex);
		while (!cond())
			bufferCond.wait_for(lock, 50us, cond);
	}

public:
	ringStream_t() : count{0}, readIndex{0}, writeIndex{0}, buffer{std::make_unique<char []>(maxEntries)},
		bufferMutex{}, bufferCond{} { }

	bool read(void *const value, const size_t valueLen, size_t &actualLen) final override
	{
		wait([&]() noexcept { return count >= valueLen; });
		memcpy(value, &buffer[readIndex], valueLen);
		readIndex += valueLen;
		readIndex %= maxEntries;
		count -= valueLen;
		bufferCond.notify_all();
		actualLen = valueLen;
		return true;
	}

	bool write(const void *const value, const size_t valueLen) final override
	{
		wait([&]() noexcept { return count < (maxEntries - valueLen); });
		memcpy(&buffer[writeIndex], value, valueLen);
		writeIndex += valueLen;
		writeIndex %= maxEntries;
		count += valueLen;
		bufferCond.notify_all();
		return true;
	}
};

#endif /*RING_BUFFER__HXX*/
