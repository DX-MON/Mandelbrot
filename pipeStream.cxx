#include "pipeStream.hxx"
#include <unistd.h>
#include <errno.h>
#include <system_error>

pipeStream_t::pipeStream_t() : stream_t(), readFD(-1), writeFD(-1), lastRead(0)
{
	int fds[2];
	if (pipe(fds))
		throw std::system_error(errno, std::system_category());
	readFD = fds[0];
	writeFD = fds[1];
}

pipeStream_t::~pipeStream_t() noexcept
{
	close(writeFD);
	close(readFD);
}

bool pipeStream_t::read(void *const value, const size_t valueLen, size_t &actualLen)
{
	const ssize_t result = ::read(readFD, value, valueLen);
	if (result > 0)
	{
		actualLen = size_t(result);
		lastRead = static_cast<char *const>(value)[actualLen - 1];
	}
	else if (result < 0)
		throw std::system_error(errno, std::system_category());
	return actualLen == valueLen;
}

bool pipeStream_t::write(const void *const valuePtr, const size_t valueLen)
{
	const ssize_t result = ::write(writeFD, valuePtr, valueLen);
	if (result < 0)
		throw std::system_error(errno, std::system_category());
	return size_t(result) == valueLen;
}

void pipeStream_t::readSync() noexcept { lastRead = 0; }
void pipeStream_t::writeSync() noexcept { write("\n", 1); }

bool pipeStream_t::atEOF() const noexcept
	{ return lastRead == '\n'; }
