#ifndef SOCKET__HXX
#define SOCKET__HXX

#include <memory>
#include <stream.hxx>

struct sockaddr;
struct sockaddr_storage;

using socklen_t = unsigned int;

struct socket_t final
{
private:
	int32_t socket;

	bool bind(const void *const addr, const size_t len) const noexcept;
	bool connect(const void *const addr, const size_t len) const noexcept;

public:
	constexpr socket_t() noexcept : socket(-1) { }
	constexpr socket_t(const int32_t s) noexcept : socket(s) { }
	socket_t(const int family, const int type, const int protocol = 0) noexcept;
	socket_t(const socket_t &) = delete;
	socket_t(socket_t &&s) noexcept : socket_t() { swap(s); }
	~socket_t() noexcept;

	socket_t &operator =(const socket_t &) = delete;
	void operator =(socket_t &&s) noexcept { swap(s); }
	operator int32_t() const noexcept { return socket; }
	bool operator ==(const int32_t s) const noexcept { return socket == s; }
	bool operator !=(const int32_t s) const noexcept { return socket != s; }
	void swap(socket_t &s) noexcept { std::swap(socket, s.socket); }
	bool valid() const noexcept { return socket != -1; }

	template<typename T> bool bind(const T &addr) const noexcept
		{ return bind(static_cast<const void *>(&addr), sizeof(T)); }
	bool bind(const sockaddr_storage &addr) const noexcept;
	template<typename T> bool connect(const T &addr) const noexcept
		{ return connect(static_cast<const void *>(&addr), sizeof(T)); }
	bool connect(const sockaddr_storage &addr) const noexcept;
	bool listen(const int32_t queueLength) const noexcept;
	socket_t accept(sockaddr *peerAddr = nullptr, socklen_t *peerAddrLen = nullptr) const noexcept;
	ssize_t write(const void *const bufferPtr, const size_t len) const noexcept;
	ssize_t read(void *const bufferPtr, const size_t len) const noexcept;
	char peek() const noexcept;
};

inline void swap(socket_t &a, socket_t &b) noexcept
	{ return a.swap(b); }

enum class socketType_t : uint8_t
{
	unknown,
	ipv4,
	ipv6,
	dontCare
};

struct socketStream_t : public stream_t
{
private:
	constexpr static const uint32_t bufferLen = 1024;
	socketType_t family;
	socket_t sock;
	std::unique_ptr<char []> buffer;
	uint32_t pos;
	char lastRead;

	void makeBuffer() noexcept;

protected:
	socketStream_t(const socketType_t _family, socket_t _sock, std::unique_ptr<char []> _buffer) noexcept :
		family(_family), sock(std::move(_sock)), buffer(std::move(_buffer)), pos(0), lastRead(0) { }
	socketStream_t() noexcept;

public:
	socketStream_t(const socketType_t type);
	socketStream_t(const socketStream_t &) = delete;
	socketStream_t(socketStream_t &&) = default;
	~socketStream_t() noexcept override = default;
	socketStream_t &operator =(const socketStream_t &) = delete;
	socketStream_t &operator =(socketStream_t &&) = default;

	bool valid() const noexcept;
	// Either call the listen() API OR the connect() - NEVER both for a socketStream_t instance.
	bool connect(const char *const where, uint16_t port) noexcept;
	bool listen(const char *const where, uint16_t port) noexcept;
	socketStream_t accept() const noexcept;

	bool read(void *const value, const size_t valueLen, size_t &actualLen) final override;
	bool write(const void *const value, const size_t valueLen) final override;
};

#endif /*SOCKET__HXX*/
