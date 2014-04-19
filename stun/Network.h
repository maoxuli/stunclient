//
//  Network.h
//
//  Created by Maoxu Li on 8/10/10.
//  Copyright (c) 2010 LIM Labs. All rights reserved.
//

#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>

#if defined(_WIN32)
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "ws2_32.lib")
typedef int ssize_t;
typedef int socklen_t;
#else
#	include <ifaddrs.h>
#   include <unistd.h>
#   include <fcntl.h>
#	include <sys/ioctl.h>
#	include <net/if.h>
#   include <sys/socket.h>
#   include <sys/poll.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <errno.h>
#endif

#ifndef _WIN32
#   define SOCKET 			int
#   define SOCKET_ERROR		-1
#   define INVALID_SOCKET	-1
#endif

#define NETWORK_NAMESPACE

#ifdef NETWORK_NAMESPACE
#   define NETWORK_BEGIN namespace network {
#   define NETWORK_END   }
#else
#   define NETWORK_BEGIN 
#   define NETWORK_END
#endif

NETWORK_BEGIN

bool startup();
void cleanup();

std::string addressToString(const struct sockaddr_in& sin);
struct in_addr resolveHostName(const std::string& name);
std::vector<struct in_addr> getLocalAddress();

class UdpSocket
{        
public:
    UdpSocket();
	UdpSocket(const std::string& host, unsigned short port); // Remote address
	virtual ~UdpSocket();
    
    sockaddr_in localAddress();
    sockaddr_in remoteAddress();
    
    void setRemoteAddress(const std::string& host, unsigned short port);
    void setRemoteAddress(const struct sockaddr_in& sin);
    
	ssize_t write(const unsigned char* buf, size_t size);
	ssize_t read(unsigned char* buf, size_t size);
	ssize_t read(unsigned char* buf, size_t size, int timeout);

private:
	SOCKET _socket;
	sockaddr_in _sin;
};

NETWORK_END

#endif
