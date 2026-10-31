#ifndef NET_COMPAT_H
#define NET_COMPAT_H

// Thin layer over BSD sockets / Winsock so the client and server code is
// identical on Windows, Linux and macOS.

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>

    typedef SOCKET sock_t;
    typedef int sock_len_t;

    #define SOCK_INVALID INVALID_SOCKET
    #define SOCK_ERROR SOCKET_ERROR
    #define SOCK_SHUT_WR SD_SEND

    inline bool netInit() {
        WSADATA wsaData{};
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }
    inline void netCleanup() { WSACleanup(); }
    inline void sockClose(sock_t s) { closesocket(s); }
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <signal.h>
    #include <sys/socket.h>
    #include <unistd.h>

    typedef int sock_t;
    typedef size_t sock_len_t;

    #define SOCK_INVALID (-1)
    #define SOCK_ERROR (-1)
    #define SOCK_SHUT_WR SHUT_WR

    inline bool netInit() {
        // A peer disconnecting mid-send must not kill the process.
        signal(SIGPIPE, SIG_IGN);
        return true;
    }
    inline void netCleanup() {}
    inline void sockClose(sock_t s) { close(s); }
#endif

#endif
