//
//  Network.cpp
//
//  Created by Maoxu Li on 8/10/10.
//  Copyright (c) 2010 LIM Labs. All rights reserved.
//

#include "Network.h"
#include <cassert>
#include <iostream>
#include <sstream>

NETWORK_BEGIN

bool startup()
{
#ifdef _WIN32
	WORD wVersionRequested = MAKEWORD(2, 2); //MAKEWORD(lowbyte, highbyte)
    WSADATA wsaData;
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
	{
        printf("WSAStartup failed with error: %d\n", err);
        return false;
    }
    
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
    
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return false;
    }
#endif
    
    return true;
}

void cleanup()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

std::string addressToString(const struct sockaddr_in& sin)
{
    std::ostringstream oss;
    oss << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port);
    return oss.str();
}

struct in_addr resolveHostName(const std::string& name)
{
    int retry = 5;
    struct addrinfo* info = 0;
    struct addrinfo hints = { 0 };
    hints.ai_family = PF_INET;

    int rs = 0;
    do
    {
        rs = ::getaddrinfo(name.c_str(), 0, &hints, &info);
    }
    while(info == 0 && rs == EAI_AGAIN && --retry >= 0);
    
    if(rs != 0)
    {
        assert(false);
    }
    
    sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(info->ai_addr);
    in_addr ip = sin->sin_addr;
    freeaddrinfo(info);
    
    return ip;
}

std::vector<struct in_addr> getLocalAddress()
{
    std::vector<struct in_addr> result;
    
#if defined(__linux) || defined(__APPLE__) || defined(__FreeBSD__)
    struct ifaddrs* ifap;
    if(::getifaddrs(&ifap) == SOCKET_ERROR)
    {
        assert(false);
        throw;
    }
    struct ifaddrs* curr = ifap;
    while(curr != 0)
    {
        if(curr->ifa_addr && !(curr->ifa_flags & IFF_LOOPBACK))  // Exclude loopback interface
        {
            if(curr->ifa_addr->sa_family == AF_INET)
            {
                sockaddr_in* sin = (sockaddr_in*)curr->ifa_addr;
                if(sin->sin_addr.s_addr != 0)
                {
                    result.push_back(sin->sin_addr);
                }
            }
        }
        curr = curr->ifa_next;
    }
    ::freeifaddrs(ifap);
#endif
    
    return result;
}

UdpSocket::UdpSocket()
{
    memset(&_sin, 0, sizeof(_sin));
	_sin.sin_family = AF_INET;

    _socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(_socket != INVALID_SOCKET);
}

UdpSocket::UdpSocket(const std::string& host, unsigned short port)
{
    memset(&_sin, 0, sizeof(_sin));
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
	_sin.sin_addr.s_addr = resolveHostName(host).s_addr;
 
    _socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(_socket != INVALID_SOCKET);
}

UdpSocket::~UdpSocket()
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    closesocket(_socket);
    WSASetLastError(error);
#else
    int error = errno;
    close(_socket);
    errno = error;
#endif
}

sockaddr_in UdpSocket::localAddress()
{
    sockaddr_in sin;
    socklen_t len = sizeof(sockaddr_in);
    if(::getsockname(_socket, (struct sockaddr*)&sin, &len) == SOCKET_ERROR)
    {
        assert(false);
        std::cerr << "UdpSocket::localAddress() failed!\n";
    }
    return sin;
}

sockaddr_in UdpSocket::remoteAddress()
{
    return _sin;
}

void UdpSocket::setRemoteAddress(const std::string& host, unsigned short port)
{
    _sin.sin_port = htons(port);
    _sin.sin_addr.s_addr = resolveHostName(host).s_addr;
}

void UdpSocket::setRemoteAddress(const struct sockaddr_in& sin)
{
    //_sin.sin_family = sin.sin_family;
    _sin.sin_port = sin.sin_port;
    _sin.sin_addr.s_addr = sin.sin_addr.s_addr;
}

ssize_t UdpSocket::write(const unsigned char* buf, size_t size)
{
	return ::sendto(_socket, buf, size, 0, (struct sockaddr*)&_sin, sizeof(_sin));
}

ssize_t UdpSocket::read(unsigned char* buf, size_t size)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    socklen_t len = sizeof(sin);
	return ::recvfrom(_socket, buf, size, 0, (struct sockaddr*)&sin, &len);
}

ssize_t UdpSocket::read(unsigned char* buf, size_t size, int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
	
	fd_set fds;
    FD_ZERO(&fds);
    FD_SET(_socket, &fds);
    
    int rc = ::select(sizeof(fds)*8, &fds, NULL, NULL, &tv);
    if(rc > 0 && FD_ISSET(_socket, &fds))
    {
	    struct sockaddr_in sin;
	    memset(&sin, 0, sizeof(sin));
	    socklen_t len = sizeof(sin);
		return ::recvfrom(_socket, buf, size, 0, (struct sockaddr*)&sin, &len);
	}
	
	return -1;
}

NETWORK_END
