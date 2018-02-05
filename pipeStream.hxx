#ifndef PIPE_STREAM__HXX
#define PIPE_STREAM__HXX

#include <rSON.h>

struct pipeStream_t final : public rSON::stream_t
{
private:
	int readFD, writeFD;
	char lastRead;

public:
	pipeStream_t();
	~pipeStream_t() noexcept;

	bool read(void *const value, const size_t valueLen, size_t &actualLen) final override;
	bool write(const void *const value, const size_t valueLen) final override;
	bool atEOF() const noexcept final override;
	bool valid() const noexcept { return readFD != -1 && writeFD != -1; }
	void readSync() noexcept final override;
	void writeSync() noexcept final override;
};

#endif /*PIPE_STREAM__HXX*/
