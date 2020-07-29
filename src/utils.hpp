#ifndef UDP_FORWARDER_DYN_UTILs_HPP_
#define UDP_FORWARDER_DYN_UTILs_HPP_ 1

#include <rlib/sys/os.hpp>
#include <rlib/sys/sio.hpp>
#include <thread>
#include <unordered_map>
#include "common.hpp"

#if RLIB_OS_ID == OS_LINUX
#include <sys/epoll.h>
#elif RLIB_OS_ID == OS_WINDOWS
#include <wepoll.h>
#endif

struct SockAddr {
    union {
        sockaddr_storage addr_storage;
        sockaddr addr;
        sockaddr_in in4;
        sockaddr_in6 in6;
    };
    int len;
};

struct ConnectionMapping {
    std::unordered_map<string, fd_t> client2server;
    std::unordered_multimap<fd_t, string> server2client;
    static string makeClientId(const SockAddr &osStrust) const {
        // ClientId is a binary string.
        string result(sizeof(osStruct), '\0');
        std::memcpy(result.data(), &osStruct, sizeof(osStrust));
        return result;
    }
    static void parseClientId(const string &clientId, SockAddr &output) const {
        static_assert(sizeof(output) == sizeof(SockAddr), "error: programming error detected.");
        if (clientId.size() != sizeof(output))
            throw std::invalid_argument("parseClientId, invalid input binary string length.");
        std::memcpy(&output, clientId.data(), sizeof(output));
    }
};

inline void epoll_add_fd(fd_t epollFd, fd_t fd) {
    epoll_event event {
        .events = EPOLLIN,
        .data = {
                .fd = fd,
        }
    };
    auto ret1 = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
    if(ret1 == -1)
        throw std::runtime_error("epoll_ctl failed.");
}
inline void epoll_del_fd(fd_t epollFd, fd_t fd) {
    epoll_event event {
        .events = EPOLLIN,
        .data = {
                .fd = fd,
        }
    };
    auto ret1 = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &event); // Can be nullptr since linux 2.6.9
    if(ret1 == -1)
        throw std::runtime_error("epoll_ctl failed.");
}

/*
#if RLIB_OS_ID == OS_WINDOWS
#error This code "mkpipe" is POSIX only. You may disable this error while developing on windows. 
// Even you make pipe working on windows, this code may still crash. because wepoll doesn't support pipe on windows!
#else
#include <unistd.h>
inline auto mkpipe() {
    int pipefd[2];
    if(0 != pipe(pipefd))
        throw std::runtime_error("mkpipe failed.");
    return std::make_pair(pipefd[0], pipefd[1]);
}
#endif
*/

inline auto mk_tcp_pipe() {
    fd_t connfd_cli_side, connfd_srv_side;
    auto listenfd = rlib::quick_listen("::1", TCP_TMP_PORT_NUMBER);
    auto serverThread = std::thread([&] {
        connfd_srv_side = rlib::quick_accept(listenfd);
    });
    connfd_cli_side = rlib::quick_connect("::1", TCP_TMP_PORT_NUMBER);
    serverThread.join();
    return std::make_pair(connfd_cli_side, connfd_srv_side);
}


#endif

