#ifndef STRING__HXX
#define STRING__HXX

#include <string.h>

inline std::unique_ptr<char []> stringDup(const char *const str) noexcept
{
	auto ret = makeUnique<char []>(strlen(str) + 1);
	strcpy(ret.get(), str);
	return ret;
}

inline std::unique_ptr<const char []> strNewDup(const char *const str) noexcept
	{ return std::unique_ptr<const char []>(stringDup(str).release()); }

#endif /*STRING__HXX*/
