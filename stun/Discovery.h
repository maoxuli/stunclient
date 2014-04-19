//
//  Discovery.h
//  stun
//
//  Created by Maoxu Li on 2/11/12.
//  Copyright (c) 2012 LIM Labs. All rights reserved.
//

#ifndef STUN_DISCOVERY_H
#define STUN_DISCOVERY_H

#include <stun/Config.h>
#include <stun/Message.h>
#include <stun/Network.h>

STUN_BEGIN

/* 
 RFC 3489 (March 2003)
 10.1  Discovery Process
 
 In this scenario, a user is running a multimedia application which
 needs to determine which of the following scenarios applies to it:
 
 o  On the open Internet
 o  Firewall that blocks UDP
 o  Firewall that allows UDP out, and responses have to come back to
 the source of the request (like a symmetric NAT, but no
 translation.  We call this a symmetric UDP Firewall)
 o  Full-cone NAT
 o  Symmetric NAT
 o  Restricted cone or restricted port cone NAT
 */

class Discovery
{
public:
    // Initiate with STUN server
    Discovery(const std::string& host, unsigned short port, int timeout = 2000); // ms
    ~Discovery();
    
    void discover();
    
private:    
    void setRemoteAddress(const std::string& host, unsigned short port);
    void setRemoteAddress(const struct sockaddr_in& sin);
    sockaddr_in remoteAddress();
    bool isLocalAddress(const struct sockaddr_in& sin);
    bool isSameAddress(const struct sockaddr_in& a, const struct sockaddr_in& b);
    
    void sendMessage(Message* msg);
    Message* receiveMessage(int timeout);

    BindingResponse* binding(bool portChange = false, bool ipChange = false);

private: 
    std::string _host;
    unsigned short _port;
    int _timeout;
    
    // UDP socket
    network::UdpSocket _socket;
    
    // Transaction ID of last request, discard unmatched response
    network::UUID _tid;
};

STUN_END

#endif 
