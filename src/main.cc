#include <rlib/stdio.hpp>
#include <rlib/opt.hpp>
#include <rlib/sys/os.hpp>
#include <thread>
#include "common.hpp"
#include "forwarder.hpp"

rlib::logger rlog(std::cerr);
using namespace rlib::literals;
using namespace std::chrono_literals;

#if RLIB_OS_ID == OS_WINDOWS
    #define windows_main main
    #ifdef ERROR
    #undef ERROR
    #endif
#else
    #define real_main main
#endif

int real_main(int argc, char **argv) {
    rlib::opt_parser args(argc, argv);
    if(args.getBoolArg("--help", "-h")) {
        rlog.info("Usage: {} -i $InboundConfig -o $OutboundConfig [--filter $filterConfig ...] [--log=error/info/verbose/debug]"_rs.format(args.getSelf()));
        rlog.info("  InboundConfig and OutboundConfig are in this format: ");
        rlog.info("  '$method@$params', available methods: ");
        rlog.info("  'plain@$addr@$port', 'misc@$addr@$portRange@$psk'");
        rlog.info("There could be multiple --filter, but they MUST be in correct order. ");
        rlog.info("available filters: 'aes@$password' , 'xor@$password', 'reverse@$otherFilter'");
        return 0;
    }
    auto inboundConfig = args.getValueArg("-i");
    auto outboundConfig = args.getValueArg("-o");
    auto log_level = args.getValueArg("--log", false, "info");

    if(log_level == "error")
        rlog.set_log_level(rlib::log_level_t::ERROR);
    else if(log_level == "info")
        rlog.set_log_level(rlib::log_level_t::INFO);
    else if(log_level == "verbose")
        rlog.set_log_level(rlib::log_level_t::VERBOSE);
    else if(log_level == "debug")
        rlog.set_log_level(rlib::log_level_t::DEBUG);
    else
        throw std::runtime_error("Unknown log level: " + log_level);

    std::list<rlib::string> filterConfigs;
    while (true) {
        if (auto filterConfig = args.getValueArg("--filter", false, ""); !filterConfig.empty()) {
            filterConfigs.push_back(filterConfig);
        }
        else
            break;
    }

    Forwarder(inboundConfig, outboundConfig, filterConfigs).runForever();

    return 0;
}

// This wrapper (maybe) makes debug easier. 
int windows_main(int argc, char** argv) {
    // fucking windows behaves strangely on exception. 
    // Let's catch all exceptions and print. 
    try {
        return real_main(argc, argv);
    }
    catch (std::exception& e) {
        rlog.fatal(e.what());
        rlog.warning("Sleep 5s before exit...");
        std::this_thread::sleep_for(5s);
        return 2;
    }
}
