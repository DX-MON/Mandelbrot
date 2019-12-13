#ifndef MEM_BUFFER__HXX
#define MEM_BUFFER__HXX

#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

template<typename T> struct memBuffer_t final
{
private:
	uint32_t readIndex;
	std::atomic<uint32_t> writeIndex;
	std::unique_ptr<T []> buffer;
	mutable std::mutex bufferMutex;
	mutable std::condition_variable bufferCond;

public:
	static uint32_t length;

	memBuffer_t() : readIndex{0}, writeIndex{0},
		buffer{std::make_unique<T []>(length)}, bufferMutex{}, bufferCond{} { }

	T readNext() noexcept
	{
		if (readIndex == writeIndex)
		{
			std::unique_lock<std::mutex> lock(bufferMutex);
			bufferCond.wait(lock, [&]() noexcept { return readIndex != writeIndex; });
		}
		return buffer[readIndex++];
	}

	void write(const T &value) noexcept
	{
		buffer[writeIndex++] = value;
		bufferCond.notify_all();
	}
};

template<typename T> uint32_t memBuffer_t<T>::length = 0;

#endif /*MEM_BUFFER__HXX*/
