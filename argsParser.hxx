#ifndef ARGS_PARSER__HXX
#define ARGS_PARSER__HXX

#include <stdint.h>
#include <memory>

struct arg_t final
{
	const char *value;
	const uint32_t numMinParams;
	const uint32_t numMaxParams;
	const uint8_t flags;
};

struct parsedArg_t final
{
	using strPtr_t = std::unique_ptr<const char []>;

	strPtr_t value;
	uint32_t paramsFound;
	std::unique_ptr<strPtr_t []> params;
	uint8_t flags;
};

using constParsedArg_t = const parsedArg_t *;
using parsedArgs_t = std::unique_ptr<constParsedArg_t []>;

#define ARG_REPEATABLE	1
#define ARG_INCOMPLETE	2

void registerArgs(const arg_t *allowedArgs) noexcept;
parsedArgs_t parseArguments(const uint32_t argc, const char *const *const argv) noexcept;
constParsedArg_t findArg(constParsedArg_t *const args, const char *const value, const constParsedArg_t defaultVal);
inline constParsedArg_t findArg(const parsedArgs_t &args, const char *const value, const constParsedArg_t defaultVal)
	{ return findArg(args.get(), value, defaultVal); }
const arg_t *findArgInArgs(const char *const value);
inline const arg_t *findArgInArgs(const parsedArg_t::strPtr_t &value)
	{ return findArgInArgs(value.get()); }
inline const arg_t *findArgInArgs(const parsedArg_t *const arg)
	{ return findArgInArgs(arg->value); }

inline uint32_t countParsedArgs(constParsedArg_t *const args)
{
	uint32_t n(0);
	while (args[n] != nullptr)
		++n;
	return n;
}
inline uint32_t countParsedArgs(const parsedArgs_t &args)
	{ return countParsedArgs(args.get()); }

extern bool checkAlreadyFound(const parsedArgs_t &parsedArgs, const parsedArg_t &toCheck) noexcept;
extern uint32_t checkParams(const uint32_t argc, const char *const *const argv, const uint32_t argPos, const arg_t &argument, const arg_t *const args) noexcept;

#endif /*ARGS_PARSER__HXX*/
