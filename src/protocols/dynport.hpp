#ifndef UDP_FORWARDER_PROT_PLAIN_HPP_
#define UDP_FORWARDER_PROT_PLAIN_HPP_ 1

#include <protocols/base.hpp>
#include <rlib/sys/sio.hpp>
#include <rlib/string.hpp>

namespace Protocols {
	class MiscInboundListener : public BaseListener {
	public:
		virtual loadConfig(string config) override {
			auto ar = rlib::string(config).split('@'); // Also works for ipv6.
			if (ar.size() != 3)
				throw std::invalid_argument("Wrong parameter string for protocol 'misc'. Example:    plain@fe00:1e10:ce95:1@1080-3080");
			listenAddr = ar[1];
			// listenPort = ar[2].as<uint16_t>();

			// listen these ports.

		}
		virtual listenForever(BaseHandler* nextHop) override {

			// if message arrives:
			// send to handler. 

		}

	private:
		string listenAddr;
		uint16_t listenPort;
		string psk;
	};

	class MiscOutboundListener : public BaseListener {

	};

	class MiscOutboundHandler : public BaseHandler {

	};
}

#endif


