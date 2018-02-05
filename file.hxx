#ifndef FILE__HXX
#define FILE__HXX

#include <stdio.h>

struct file_t final
{
private:
	FILE *file;

public:
	constexpr file_t() noexcept : file(nullptr) { }
	~file_t() noexcept { close(); }
	void close() noexcept { if (file) fclose(file); file = nullptr; }
	bool valid() const noexcept { return bool(file); }
	operator FILE *() const noexcept { return file; }

	void operator =(FILE *const f) noexcept
	{
		close();
		file = f;
	}
};

#endif /*FILE__HXX*/
