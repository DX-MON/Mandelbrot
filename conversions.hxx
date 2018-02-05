#include <stdio.h>
#include <type_traits>
#include <utility>
#include <string>
#include "memory.hxx"

inline bool isComma(const char x) noexcept
	{ return x == ','; }

template<typename A> using isSigned = std::is_signed<A>;
template<typename A> using makeUnsigned = typename std::make_unsigned<A>::type;

// This version of toInt_t intentionally doesn't support signed numbers.
template<typename int_t> struct toInt_t
{
private:
	using uint_t = makeUnsigned<int_t>;
	const char *const _value;
	const size_t _length;
	constexpr static const bool _isSigned = isSigned<int_t>::value;
	static_assert(!_isSigned, "This version of toInt_t<> doesn't support signed numbers.");

	static bool isNumber(const char x) noexcept { return x >= '0' && x <= '9'; }

	template<bool isFunc(const char)> bool checkValue() const noexcept
	{
		for (size_t i{0}; i < _length; ++i)
		{
			if (!isFunc(_value[i]))
				return false;
		}
		return true;
	}

public:
	toInt_t(const char *const value) noexcept : _value(value), _length(std::char_traits<char>::length(value)) { }
	size_t length() const noexcept { return _length; }
	bool isInt() const noexcept { return checkValue<isNumber>(); }

	operator int_t() const noexcept
	{
		uint_t value{0};
		for (size_t i{0}; i < _length; ++i)
		{
			value *= 10;
			value += _value[i] - '0';
		}
		return int_t(value);
	}
};

inline std::vector<std::string> parseNodelist(const std::string nodeList)
{
	std::vector<std::string> nodes;
	const uint32_t length = nodeList.length();
	uint32_t start{0}, i{0};

	const auto slice = [&](const uint32_t start, const uint32_t end) noexcept
	{
		if (start != end)
			nodes.emplace_back(nodeList.substr(start, end - start));
		else
			puts("Skipping empty node specification");
	};

	for (; i < length; ++i)
	{
		if (isComma(nodeList[i]))
		{
			slice(start, i);
			start = i + 1;
		}
	}
	slice(start, i);
	return nodes;
}
