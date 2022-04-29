module;
#include <streambuf>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include <Winsock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#pragma warning(disable : 4267)
export module socket_stream;

namespace hexpress {

	struct winsock {
		typedef BYTE byte_type;
		typedef std::pair<byte_type, byte_type> version_type;

		static version_type default_version()
		{
			return version_type(static_cast<byte_type>(2),
				static_cast<byte_type>(2));
		}

		static int init(const version_type& ver = default_version())
		{
			WSADATA wsd = WSADATA();
			return ::WSAStartup(MAKEWORD(ver.first, ver.second),
				&wsd) == 0 ? 0 : -1;
		}

		static int term()
		{
			return ::WSACleanup() == 0 ? 0 : -1;
		}
	};

	/*
	 * It's ok to call WSAStartup multiple times as long as an equal
	 * number of WSACleanup calls are made. They are also thread-safe.
	 *
	 * Using thread-safe reference counting so that WSAStartup and
	 * WSACleanup only get called once can yield performance benefits,
	 * but implementing this requires using either the C++11 std::atomic
	 * class or the Interlocked Windows functions on a 32-bit aligned
	 * integer.
	 *
	 * Using std::atomic breaks compatibility with C++03. Using alignas
	 * to fulfill the alignment requirement for the Interlocked operations
	 * also breaks compatibility with C++03. Non-standard alignment
	 * specifiers exist but they vary across compilers.
	 *
	 * Windows has a function called _align_malloc, but it is not
	 * guaranteed to zero out the memory it allocates and not all
	 * compilers on Windows have their header files updated to provide
	 * this function.
	 *
	 * The Boost libraries provide a type_with_alignment class but
	 * I do not want this library to have any dependencies.
	 */

	class winsock_auto_init {
	public:
		winsock_auto_init()
		{
			winsock::init();
		}

		~winsock_auto_init()
		{
			winsock::term();
		}
	};

#ifndef SWOOPE_MANUAL_WINSOCK_INIT
	static const winsock_auto_init winsock_auto_init_instance;
#endif

}

namespace hexpress {

	struct native_socket_traits {

		typedef SOCKET socket_type;

		static socket_type invalid()
		{
			return INVALID_SOCKET;
		}

		static socket_type open(const std::string& host,
			const std::string& service)
		{
			using std::swap;
			addrinfo* ai, hints = addrinfo();
			socket_type result((invalid()));

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			if (::getaddrinfo(host.c_str(), service.c_str(),
				&hints, &ai) != 0)
				return result;
			socket_type socket((::socket(ai->ai_family,
				ai->ai_socktype,
				ai->ai_protocol)));
			if (socket != result && ::connect(socket, ai->ai_addr,
				static_cast<int>(ai->ai_addrlen)) == 0)
				swap(result, socket);
			::freeaddrinfo(ai);
			return result;
		}

		static socket_type open(const std::string& service,
			int backlog)
		{
			using std::swap;
			addrinfo* ai, hints = addrinfo();
			socket_type result((invalid()));
			BOOL optval = TRUE;

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags |= AI_PASSIVE;
			if (::getaddrinfo(0, service.c_str(), &hints, &ai) != 0)
				return result;
			socket_type socket((::socket(ai->ai_family,
				ai->ai_socktype,
				ai->ai_protocol)));
			if (socket == result) return result;
			if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR,
				(const char*)&optval, sizeof(optval)) != 0 ||
				::bind(socket, ai->ai_addr,
					ai->ai_addrlen) != 0 ||
				::listen(socket, backlog) != 0) {
				::closesocket(socket);
				socket = result;
			}
			::freeaddrinfo(ai);
			swap(result, socket);
			return result;
		}

		static socket_type accept(socket_type sock)
		{
			return ::accept(sock, 0, 0);
		}
	private:
		static std::string sockaddr_storage_to_string(
			SOCKADDR_STORAGE* ss)
		{
			std::string result;
			socklen_t sslen(sizeof(*ss));
			char addrbuf[64];

			if (::getnameinfo((const SOCKADDR*)ss,
				sslen,
				addrbuf,
				sizeof(addrbuf),
				0,
				0,
				NI_NUMERICHOST) == 0) {
				result = addrbuf;
			}
			return result;
		}
	public:
		static std::string local_address(socket_type sock)
		{
			SOCKADDR_STORAGE ss;
			int sl = sizeof(ss);

			if (::getsockname(sock, (SOCKADDR*)&ss, &sl) != 0)
				return "";
			return sockaddr_storage_to_string(&ss);
		}

		static std::string remote_address(socket_type sock)
		{
			SOCKADDR_STORAGE ss;
			int sl = sizeof(ss);

			if (::getpeername(sock, (SOCKADDR*)&ss, &sl) != 0)
				return "";
			return sockaddr_storage_to_string(&ss);
		}

		static std::streamsize read(socket_type socket,
			void* buf,
			std::streamsize n)
		{
			return ::recv(socket, static_cast<char*>(buf),
				static_cast<int>(n), 0);
		}

		static std::streamsize write(socket_type socket,
			const void* buf,
			std::streamsize n)
		{
			return ::send(socket, static_cast<const char*>(buf),
				static_cast<int>(n), 0);
		}

		static int shutdown(socket_type socket, std::ios_base::
			openmode how)
		{
			int result(-1);

			if (how == (std::ios_base::in | std::ios_base::out))
				result = ::shutdown(socket,
					SD_BOTH) == 0 ? 0 : -1;
			else if (how == std::ios_base::out)
				result = ::shutdown(socket,
					SD_SEND) == 0 ? 0 : -1;
			else if (how == std::ios_base::in)
				result = ::shutdown(socket,
					SD_RECEIVE) == 0 ? 0 : -1;
			return result;
		}

		static int close(socket_type socket)
		{
			return (::closesocket(socket) == 0) ? 0 : -1;
		}

	};

}

namespace hexpress {

	template <class SocketTraits>
	class basic_socketbuf_base {
	public:
		/* Socket handle */
		typename SocketTraits::socket_type socket;

		/* Buffer used for unbuffered I/O */
		char buf[1];

		/* Start of buffer */
		char* base;

		std::streamsize
			gasize, /* get area size */
			pasize; /* put area size */

		std::ios_base::openmode mode;

		bool is_open, auto_delete_base;

		basic_socketbuf_base();
#if __cplusplus >= 201103L
		basic_socketbuf_base(const basic_socketbuf_base&) = delete;
		void swap(basic_socketbuf_base& rhs);
#endif
		virtual ~basic_socketbuf_base();
		void release_base();
		void reset_base(char* p, bool auto_delete);

#if __cplusplus < 201103L
	private:
		basic_socketbuf_base(const basic_socketbuf_base&);
#endif
	};

#if __cplusplus >= 201103L
	template <class SocketTraits>
	inline void
		swap(basic_socketbuf_base<SocketTraits>& a, basic_socketbuf_base<
			SocketTraits>& b)
	{
		a.swap(b);
	}
#endif

	template <class SocketTraits>
	class basic_socketbuf :
		public std::streambuf,
		private basic_socketbuf_base<SocketTraits> {
	public:
		typedef basic_socketbuf_base<SocketTraits>
			__socketbuf_base_type;
		typedef std::streambuf __streambuf_type;

		typedef SocketTraits socket_traits_type;
		typedef typename socket_traits_type::socket_type socket_type;

		typedef char char_type;
		typedef std::char_traits<char_type> traits_type;
		typedef typename traits_type::int_type int_type;
		typedef typename traits_type::pos_type pos_type;
		typedef typename traits_type::off_type off_type;

		basic_socketbuf();
#if __cplusplus >= 201103L
		basic_socketbuf(basic_socketbuf&& rhs);
#endif
		virtual ~basic_socketbuf();

#if __cplusplus >= 201103L
		basic_socketbuf& operator=(
			const basic_socketbuf& rhs) = delete;
		basic_socketbuf& operator=(basic_socketbuf&& rhs);
		void swap(basic_socketbuf& rhs);
#endif
		bool is_open() const;
		/*
		 * Associate existing socket descriptor s to this streambuf. Returns this
		 * on success.
		 */
		basic_socketbuf* open(socket_type s,
			std::ios_base::openmode mode);
		/*
		 * Create an underlying TCP/IP socket and connect it to the specified host on
		 * the specified port or service. Returns this on success.
		 */
		basic_socketbuf* open(const std::string& host,
			const std::string& service,
			std::ios_base::openmode mode);
		/*
		 * Create an underlying TCP/IP socket, bind it to the specified port or
		 * service, then make it listen for connections with the specified backlog.
		 * Returns this on success.
		 */
		basic_socketbuf* open(const std::string& service, int backlog);
		/*
		 * Accepts a pending connection from this socket and stores the resulting
		 * connected socket into socketbuf_result. The string representation of
		 * the connected socket's address will be stored in address_result. Returns
		 * this on success.
		 */
		basic_socketbuf* accept(basic_socketbuf& d_socketbuf);
		/*
		 * Returns a string representing the address to which the socket is
		 * bound.
		 */
		std::string local_address() const;
		/*
		 * Returns a string representing the address of the peer that is connected
		 * to the socket.
		 */
		std::string remote_address() const;
		/*
		 * Shutdowns down the socket for input, output, or both. Possible values
		 * for how are: std::ios_base::in, std::ios_base::out, or
		 * std::ios_base::in | std::ios_base::out. Returns this on success.
		 */
		basic_socketbuf* shutdown(std::ios_base::openmode how);
		/* Closes the socket. Returns this on success. */
		basic_socketbuf* close();
		/* Returns the underlying socket descriptor. */
		socket_type socket() const;
	protected:
		basic_socketbuf* setbuf(char_type* s, std::streamsize n);
		int sync();
		std::streamsize xsgetn(char_type* s, std::streamsize n);
		int_type underflow();
		std::streamsize xsputn(const char_type* s, std::streamsize n);
		int_type overflow(int_type c = traits_type::eof());
	private:
#if __cplusplus < 201103L
		basic_socketbuf& operator=(const basic_socketbuf& rhs);
#endif
		basic_socketbuf(const basic_socketbuf& rhs);
		void init_io();
		std::streamsize read(char_type* s, std::streamsize n);
		std::streamsize write(const char_type* s, std::streamsize n);
	};

#if __cplusplus >= 201103L
	template <class SocketTraits>
	inline void
		swap(basic_socketbuf<SocketTraits>& a,
			basic_socketbuf<SocketTraits>& b)
	{
		a.swap(b);
	}
#endif

}

namespace hexpress {

	template <class SocketTraits>
	class basic_socketstream :
		public std::iostream {
	public:
		typedef basic_socketbuf<SocketTraits> __socketbuf_type;
		typedef std::iostream __iostream_type;

		typedef typename __socketbuf_type::socket_type socket_type;
		typedef typename __socketbuf_type::socket_traits_type
			socket_traits_type;

		typedef char char_type;
		typedef std::char_traits<char> traits_type;
		typedef typename traits_type::int_type int_type;
		typedef typename traits_type::pos_type pos_type;
		typedef typename traits_type::off_type off_type;

		basic_socketstream() :
			__iostream_type(&buf),
			buf()
		{
		}

		explicit basic_socketstream(socket_type s,
			std::ios_base::openmode mode =
			std::ios_base::in | std::ios_base::out) :
			__iostream_type(&buf),
			buf()
		{
			open(s, mode);
		}

		explicit basic_socketstream(
			const std::string& host,
			const std::string& service,
			std::ios_base::openmode mode =
			std::ios_base::in | std::ios_base::out) :
			__iostream_type(&buf),
			buf()
		{
			open(host, service, mode);
		}

#if __cplusplus >= 201103L
		basic_socketstream(const basic_socketstream&) = delete;

		basic_socketstream(basic_socketstream&& rhs) :
			__iostream_type(std::move(rhs)),
			buf(std::move(rhs.buf))
		{
			this->set_rdbuf(&buf);
		}
#endif

		virtual ~basic_socketstream() {}

#if __cplusplus >= 201103L
		basic_socketstream& operator=(
			const basic_socketstream&) = delete;

		basic_socketstream& operator=(basic_socketstream&& rhs)
		{
			this->__iostream_type::operator=(std::move(rhs));
			buf = std::move(rhs.buf);
			return *this;
		}

		void swap(basic_socketstream& rhs)
		{
			this->__iostream_type::swap(rhs);
			buf.swap(rhs.buf);
		}
#endif

		__socketbuf_type* rdbuf() const
		{
			return const_cast<__socketbuf_type*>(&buf);
		}

		bool is_open() const
		{
			return rdbuf()->is_open();
		}

		void open(socket_type s, std::ios_base::openmode mode =
			std::ios_base::in | std::ios_base::out)
		{
			if (rdbuf()->open(s, mode) == 0)
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		void open(const std::string& host, const std::string& service,
			std::ios_base::openmode mode =
			std::ios_base::in | std::ios_base::out)
		{
			if (rdbuf()->open(host, service, mode) == 0)
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		void open(const std::string& service, int backlog)
		{
			if (rdbuf()->open(service, backlog) == 0)
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		void accept(basic_socketstream& d_socketstream)
		{
			rdbuf()->accept(*(d_socketstream.rdbuf()));
		}

		std::string local_address() const
		{
			return rdbuf()->local_address();
		}

		std::string remote_address() const
		{
			return rdbuf()->remote_address();
		}

		void shutdown(std::ios_base::openmode how)
		{
			if (rdbuf()->shutdown(how) == 0)
				this->setstate(std::ios_base::failbit);
		}

		void close()
		{
			if (rdbuf()->close() == 0)
				this->setstate(std::ios_base::failbit);
		}

	private:
		__socketbuf_type buf;
	};

#if __cplusplus >= 201103L
	template <class SocketTraits>
	inline void swap(basic_socketstream<SocketTraits>& a,
		basic_socketstream<SocketTraits>& b)
	{
		a.swap(b);
	}
#endif

}

namespace hexpress {

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
		basic_socketbuf() :
		__streambuf_type(),
		__socketbuf_base_type()
	{
	}

#if __cplusplus >= 201103L
	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
		basic_socketbuf(basic_socketbuf&& rhs) :
		__streambuf_type(),
		__socketbuf_base_type()
	{
		swap(rhs);
	}
#endif

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
		~basic_socketbuf()
	{
		try {
			close();
		} catch (...) {}
	}

#if __cplusplus >= 201103L
	template <class SocketTraits>
	basic_socketbuf<SocketTraits>&
		basic_socketbuf<SocketTraits>::
		operator=(basic_socketbuf&& rhs)
	{
		close();
		this->reset_base(0, false);
		swap(rhs);
		return *this;
	}

	template <class SocketTraits>
	void
		basic_socketbuf<SocketTraits>::
		swap(basic_socketbuf& rhs)
	{
		__streambuf_type::swap(rhs);
		__socketbuf_base_type::swap(rhs);
	}
#endif

	template <class SocketTraits>
	bool
		basic_socketbuf<SocketTraits>::
		is_open() const
	{
		return this->__socketbuf_base_type::is_open;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
		basic_socketbuf<SocketTraits>::
		open(socket_type socket, std::ios_base::openmode m)
	{
		if (is_open() != false) return 0;
		if (socket == this->__socketbuf_base_type::socket) return 0;
		this->__socketbuf_base_type::socket = socket;
		this->mode = m;
		this->__socketbuf_base_type::is_open = true;
		return this;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
		basic_socketbuf<SocketTraits>::
		open(const std::string& host, const std::string& service,
			std::ios_base::openmode m)
	{
		if (is_open() != false) return 0;
		return open(socket_traits_type::open(host, service), m);
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
		basic_socketbuf<SocketTraits>::
		open(const std::string& service, int backlog)
	{
		if (is_open() != false) return 0;
		return open(socket_traits_type::open(service, backlog),
			std::ios_base::in | std::ios_base::out);
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
		basic_socketbuf<SocketTraits>::
		accept(basic_socketbuf& d_socketbuf)
	{
		socket_type invalid_socket(socket_traits_type::invalid()),
			server_socket(socket()),
			client_socket;
		if (d_socketbuf.is_open() != false)
			d_socketbuf.close();
		client_socket = socket_traits_type::accept(server_socket);
		if (client_socket == invalid_socket) return 0;
		if (d_socketbuf.open(client_socket, std::ios_base::in |
			std::ios_base::out) == 0)
			return 0;
		return this;
	}

	template <class SocketTraits>
	std::string
		basic_socketbuf<SocketTraits>::
		local_address() const
	{
		return socket_traits_type::local_address(socket());
	}

	template <class SocketTraits>
	std::string
		basic_socketbuf<SocketTraits>::
		remote_address() const
	{
		return socket_traits_type::remote_address(socket());
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
		basic_socketbuf<SocketTraits>::shutdown(std::ios_base::openmode m)
	{
		basic_socketbuf* result((this));

		if (is_open() == false) return 0;
		if (sync() == -1) result = 0;
		if (socket_traits_type::shutdown(this->__socketbuf_base_type::
			socket, m) != 0)
			result = 0;
		return result;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
		basic_socketbuf<SocketTraits>::close()
	{
		using std::swap;
		basic_socketbuf* result((this));
		socket_type invalid((socket_traits_type::invalid()));

		if (is_open() == false) return 0;
		if (sync() == -1) result = 0;
		if (socket_traits_type::close(this->__socketbuf_base_type::
			socket) != 0)
			result = 0;
		swap(this->__socketbuf_base_type::socket, invalid);
		this->setg(0, 0, 0);
		this->setp(0, 0);
		this->__socketbuf_base_type::is_open = false;
		return result;
	}

	template <class SocketTraits>
	typename basic_socketbuf<SocketTraits>::socket_type
		basic_socketbuf<SocketTraits>::
		socket() const
	{
		return this->__socketbuf_base_type::socket;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
		basic_socketbuf<SocketTraits>::
		setbuf(char_type* s, std::streamsize n)
	{
		if (s != 0) {
			if (n < 1) return 0;
			this->reset_base(s, false);
		} else {
			if (n <= 1) {
				this->reset_base(&this->buf[0], false);
				n = 1;
			} else {
				this->reset_base(new char_type[static_cast<
					std::size_t>(n)], true);
			}
		}

		std::ldiv_t d((std::ldiv(static_cast<long int>(n),
			static_cast<long int>(2))));
		this->gasize = d.quot + d.rem;
		this->pasize = d.quot;
		return this;
	}

	template <class SocketTraits>
	int
		basic_socketbuf<SocketTraits>::
		sync()
	{
		int_type eof((traits_type::eof()));
		int result(0);

		if (this->pptr() != 0)
			result = (overflow(eof) != eof) ? 0 : -1;
		return result;
	}

	template <class SocketTraits>
	std::streamsize
		basic_socketbuf<SocketTraits>::
		xsgetn(char_type* s, std::streamsize n)
	{

		std::streamsize result(0), avail;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::in) == 0) return result;
		if (this->gptr() == 0) init_io();
		avail = this->egptr() - this->gptr();
		if (avail >= n) {
			std::copy(this->gptr(), this->gptr() + n, s);
			this->gbump(static_cast<std::size_t>(n));
			result = n;
		} else {
			s = std::copy(this->gptr(), this->gptr() + avail, s);
			this->gbump(static_cast<std::size_t>(avail));
			result = avail + read(s, n - avail);
		}
		return result;
	}

	template <class SocketTraits>
	typename basic_socketbuf<SocketTraits>::int_type
		basic_socketbuf<SocketTraits>::
		underflow()
	{
		int_type result((traits_type::eof()));
		std::streamsize got;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::in) == 0) return result;
		if (this->gptr() == 0) init_io();
		got = read(this->eback(), this->gasize);
		if (got > 0) {
			this->setg(this->eback(), this->eback(),
				this->eback() + got);
			result = *this->gptr();
		}
		return result;
	}

	template <class SocketTraits>
	std::streamsize
		basic_socketbuf<SocketTraits>::
		xsputn(const char_type* s, std::streamsize n)
	{
		int_type eof((traits_type::eof()));
		std::streamsize result(0), pending, put;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::out) == 0) return result;
		if (this->pptr() == 0) init_io();
		pending = this->pptr() - this->pbase();
		if (pending + n <= this->pasize) {
			std::copy(s, s + n, this->pptr());
			this->pbump(static_cast<std::size_t>(n));
			result += n;
		} else {
			if (overflow(eof) != 0) return result;
			std::ldiv_t d((std::div(static_cast<long int>(n),
				static_cast<long int>(this->pasize))));
			if (d.quot > 0) {
				d.quot *= static_cast<long int>(this->pasize);
				put = write(s, d.quot);
				if (put < d.quot) return put;
				s += put;
				result += put;
			}
			std::copy(s, s + d.rem, this->pbase());
			this->pbump(static_cast<std::size_t>(d.rem));
			result += d.rem;
		}
		return result;
	}

	template <class SocketTraits>
	typename basic_socketbuf<SocketTraits>::int_type
		basic_socketbuf<SocketTraits>::
		overflow(int_type c)
	{
		int_type result((traits_type::eof()));
		std::streamsize put, pending;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::out) == 0) return result;
		if (this->pptr() == 0) init_io();
		if (this->pptr() < this->epptr() && c != result)
			return this->sputc(traits_type::to_char_type(c));
		if (this->pbase() == this->epptr()) {
			if (c == result) {
				result = traits_type::not_eof(c);
			} else {
				char_type tmp((traits_type::to_char_type(c)));
				if (write(&tmp, 1) == 1)
					result = traits_type::not_eof(c);
			}
		} else {
			pending = this->pptr() - this->pbase();
			if (pending == 0) {
				put = 0;
			} else {
				put = write(this->pbase(), pending);
				this->pbump(static_cast<std::size_t>(
					-pending));
			}
			if (put == pending) {
				if (c == result)
					result = traits_type::not_eof(c);
				else
					result = this->sputc(traits_type::
						to_char_type(c));
			}
		}
		return result;
	}

	template <class SocketTraits>
	void
		basic_socketbuf<SocketTraits>::
		init_io()
	{
		char_type* gbase, * pbase;

		if (this->base == 0)
			this->setbuf(0, 1ull << 20);
		gbase = this->base;
		pbase = gbase + this->gasize;
		if ((this->mode & std::ios_base::in) != 0)
			this->setg(gbase, gbase, gbase);
		if ((this->mode & std::ios_base::out) != 0)
			this->setp(pbase, pbase + this->pasize);
	}

	template <class SocketTraits>
	std::streamsize
		basic_socketbuf<SocketTraits>::
		read(char_type* s, std::streamsize n)
	{
		std::streamsize got, result(0);

		got = socket_traits_type::read(this->__socketbuf_base_type::
			socket, s, n);
		if (got > 0) {
			char_type* p = s;
			for (char_type *q = s, *e = s + got; p != e && q != e; ) {
				if (q + 1 != e && *q == '\r' && *(q + 1) == '\n') {
					*p = '\n';
					q++;
				} else *p = *q;
				p++, q++;
			}
			result = p - s;
		}

		return result;
	}

	template <class SocketTraits>
	std::streamsize
		basic_socketbuf<SocketTraits>::
		write(const char_type* s, std::streamsize n)
	{
		std::streamsize put, result(0);
		static std::string buffer;
		buffer.clear();
		for (char c : std::string_view{ s, static_cast<size_t>(n) }) {
			if (c == '\n') buffer.push_back('\r');
			buffer.push_back(c);
		}

		//fprintf(stderr, "n = %d buffer.size = %d\n", (int)n, (int)buffer.size());
		s = buffer.data(), n = buffer.size();

		while (result < n) {
			put = socket_traits_type::write(
				this->__socketbuf_base_type::
				socket, s, n - result);
			if (put < 0) break;
			s += put;
			result += put;
		}
		return result;
	}

	template <class SocketTraits>
	basic_socketbuf_base<SocketTraits>::
		basic_socketbuf_base() :
		socket(SocketTraits::invalid()),
		buf(),
		base(0),
		gasize(0),
		pasize(0),
		mode(),
		is_open(false),
		auto_delete_base(false)
	{
	}

	template <class SocketTraits>
	basic_socketbuf_base<SocketTraits>::
		~basic_socketbuf_base()
	{
		reset_base(0, false);
	}

#if __cplusplus >= 201103L
	template <class SocketTraits>
	void
		basic_socketbuf_base<SocketTraits>::
		swap(basic_socketbuf_base& rhs)
	{
		using std::swap;
		swap(socket, rhs.socket);
		swap(buf, rhs.buf);
		swap(base, rhs.base);
		swap(gasize, rhs.gasize);
		swap(pasize, rhs.pasize);
		swap(mode, rhs.mode);
		swap(is_open, rhs.is_open);
		swap(auto_delete_base, rhs.auto_delete_base);
	}
#endif

	template <class SocketTraits>
	void
		basic_socketbuf_base<SocketTraits>::
		release_base()
	{
		base = 0;
		auto_delete_base = false;
	}

	template <class SocketTraits>
	void
		basic_socketbuf_base<SocketTraits>::
		reset_base(char* p, bool auto_delete)
	{
		if (base != 0 && auto_delete_base == true) {
			delete[] base;
		}
		base = p;
		auto_delete_base = auto_delete;
	}

}

export namespace hexpress {

	typedef basic_socketbuf<native_socket_traits> socketbuf;
	typedef basic_socketstream<native_socket_traits> socketstream;

}
