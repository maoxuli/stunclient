//
//  Discovery.cpp
//  ppio
//
//  Created by Maoxu Li on 2/11/12.
//  Copyright (c) 2012 PPEngine. All rights reserved.
//

#include "Discovery.h"
#include <stun/Buffer.h>
#include <iostream>

STUN_BEGIN

Discovery::Discovery(const std::string& host, unsigned short port, int timeout)
: _host(host)
, _port(port)
, _timeout(timeout)
, _socket(host, port)
{

}

Discovery::~Discovery()
{

}

void Discovery::setRemoteAddress(const std::string& host, unsigned short port)
{
    _socket.setRemoteAddress(host, port);
}

void Discovery::setRemoteAddress(const struct sockaddr_in& sin)
{
    _socket.setRemoteAddress(sin);
}

sockaddr_in Discovery::remoteAddress()
{
    return _socket.remoteAddress();
}

bool Discovery::isLocalAddress(const struct sockaddr_in& sin)
{
    std::vector<struct in_addr> addrs = network::getLocalAddress();
    for(int i = 0; i < addrs.size(); i++)
    {
        std::cout << "Local IP: " << inet_ntoa(addrs[i]) << "\n";
        if(addrs[i].s_addr == sin.sin_addr.s_addr)
        {
            return true;
        }
    }
    
    return false;
}

bool Discovery::isSameAddress(const struct sockaddr_in& a, const struct sockaddr_in& b)
{
    return (a.sin_addr.s_addr == b.sin_addr.s_addr) && (a.sin_port == b.sin_port);
}


// Send stun request mesage
void Discovery::sendMessage(Message* msg)
{
    assert(msg != NULL);
    //std::cout << ">> " << msg->toString() << "\n";
    _tid = msg->tid();
    network::Buffer buf;
    msg->toBuffer(&buf);
    _socket.write(buf.read(), buf.readable());
}

Message* Discovery::receiveMessage(int timeout)
{
    network::Buffer buf;
    buf.reserve(512);
    long len = _socket.read(buf.write(), buf.writable(), timeout);
    if(len > 0)
    {
        buf.write(len);
        Message* msg = MessageFactory::fromBuffer(&buf);
        //std::cout << "<< " << msg->toString() << "\n";
        if(msg != NULL && msg->tid() == _tid)
        {
            return msg;
        }
    }
    return NULL;
}

/*
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

 The flow makes use of three tests.  In test I, the client sends a
 STUN Binding Request to a server, without any flags set in the
 CHANGE-REQUEST attribute, and without the RESPONSE-ADDRESS attribute.
 This causes the server to send the response back to the address and
 port that the request came from.  In test II, the client sends a
 Binding Request with both the "change IP" and "change port" flags
 from the CHANGE-REQUEST attribute set.  In test III, the client sends
 a Binding Request with only the "change port" flag set.
 
 The client begins by initiating test I.  If this test yields no
 response, the client knows right away that it is not capable of UDP
 connectivity.  If the test produces a response, the client examines
 the MAPPED-ADDRESS attribute.  If this address and port are the same
 as the local IP address and port of the socket used to send the
 request, the client knows that it is not natted.  It executes test
 II.
 
 If a response is received, the client knows that it has open access
 to the Internet (or, at least, its behind a firewall that behaves
 like a full-cone NAT, but without the translation).  If no response
 is received, the client knows its behind a symmetric UDP firewall.
 
 In the event that the IP address and port of the socket did not match
 the MAPPED-ADDRESS attribute in the response to test I, the client
 knows that it is behind a NAT.  It performs test II.  If a response
 is received, the client knows that it is behind a full-cone NAT.  If
 no response is received, it performs test I again, but this time,
 does so to the address and port from the CHANGED-ADDRESS attribute
 from the response to test I.  If the IP address and port returned in
 the MAPPED-ADDRESS attribute are not the same as the ones from the
 first test I, the client knows its behind a symmetric NAT.  If the
 address and port are the same, the client is either behind a
 restricted or port restricted NAT.  To make a determination about
 which one it is behind, the client initiates test III.  If a response
 is received, its behind a restricted NAT, and if no response is
 received, its behind a port restricted NAT.
 */

//
//                     +--------+
//                     |  Test  |
//                     |   I    |
//                     +--------+
//                          |
//                          |
//                          V
//                         /\              /\
//                      N /  \ Y          /  \ Y             +--------+
//       UDP     <-------/Resp\--------->/ IP \------------->|  Test  |
//       Blocked         \ ?  /          \Same/              |   II   |
//                        \  /            \? /               +--------+
//                         \/              \/                    |
//                                          | N                  |
//                                          |                    V
//                                          V                    /\
//                                      +--------+  Sym.      N /  \
//                                      |  Test  |  UDP    <---/Resp\
//                                      |   II   |  Firewall   \ ?  /
//                                      +--------+              \  /
//                                          |                    \/
//                                          V                     |Y
//               /\                         /\                    |
//Symmetric  N  /  \       +--------+   N  /  \                   V
//   NAT  <--- / IP \<-----|  Test  |<--- /Resp\               Open
//             \Same/      |   I    |     \ ?  /               Internet
//              \? /       +--------+      \  /
//               \/                         \/
//               |                           |Y
//               |                           |
//               |                           V
//               |                           Full
//               |                           Cone
//               V              /\
//           +--------+        /  \ Y
//           |  Test  |------>/Resp\---->Restricted
//           |   III  |       \ ?  /
//           +--------+        \  /
//                              \/
//                               |N
//                               |       Port
//                               +------>Restricted
//

void Discovery::discover()
{
    // TEST I
    // Send binding request with no change address request attribute
    setRemoteAddress(_host, _port);
    std::cout << "TEST I to " << network::addressToString(remoteAddress()) << "\n";
    BindingResponse* t1_response = binding();
    if(t1_response == NULL) // TEST I -> No Response
    {
        std::cout << "TEST I -> No Response.\n";
        std::cout << "You are behind a firewall that blocks UDP.\n";
    }
    else // TEST I -> Yes Response
    {
        sockaddr_in sin = t1_response->mappedAddress();
        std::cout << "Mapped address: " << network::addressToString(sin) << "\n";
        if(isLocalAddress(sin)) // IP same Yes
        {
            std::cout << "TEST I -> Mapped IP is same as local one.\n";
            // TEST II
            std::cout << "TEST II to " << network::addressToString(remoteAddress()) << "\n";
            BindingResponse* t2_response = binding(true, true);
            if(t2_response == NULL) // TEST II -> No Response
            {
                std::cout << "TEST II -> No Response.\n";
                std::cout << "You are behind a symmetric UDP Firewall.\n";
            }
            else // TEST II -> Yes Response
            {
                std::cout << "TEST II -> Yes Response.\n";
                std::cout << "You have a open Internet access.\n";
            }
        }
        else // IP same No
        {
            std::cout << "TEST I -> Mapped IP is different from local one.\n";
            // TEST II
            std::cout << "TEST II to " << network::addressToString(remoteAddress()) << "\n";
            BindingResponse* t2_response = binding(true, true);
            if(t2_response != NULL) // TEST II -> Yes Response
            {
                std::cout << "TEST II -> Yes Response.\n";
                std::cout << "You are behind a full cone NAT.\n";
            }
            else // TEST II -> No Response
            {
                std::cout << "TEST II -> No Response.\n";
                // TEST I again
                // To TEST I mapped address
                _socket.setRemoteAddress(t1_response->changedAddress());
                std::cout << "TEST I again to " << network::addressToString(remoteAddress()) << "\n";
                BindingResponse* t12_response = binding();
                if(t12_response == NULL) // TEST I(2) -> No Response
                {
                    assert(false);
                    std::cout << "ERROR!\n";
                    std::cout << "TEST I again -> No Response.\n";
                }
                else // TEST I(2) -> Yes Response
                {
                    sockaddr_in sin = t12_response->mappedAddress();
                    std::cout << "Mapped address: " << network::addressToString(sin) << "\n";
                    if(!isSameAddress(sin, t1_response->mappedAddress()))
                    {
                        std::cout << "TEST I again -> Mapped port is different from first test.\n";
                        std::cout << "You are behind a symmetric NAT.\n";
                    }
                    else
                    {
                        std::cout << "TEST I again -> Mapped port is same as first test.\n";
                        // TEST III
                        std::cout << "TEST III to " << network::addressToString(remoteAddress()) << "\n";
                        BindingResponse* t3_response = binding(true);
                        if(t3_response != NULL) // TEST III -> Yes Response
                        {
                            std::cout << "TEST III -> Yes Response.\n";
                            std::cout << "You are behind a restricted cone NAT.\n";
                        }
                        else // TEST III -> No Response
                        {
                            std::cout << "TEST III -> No Response.\n";
                            std::cout << "You are behind a restricted port cone NAT.\n";
                        }
                    }
                }
            }
        }
    }
}

BindingResponse* Discovery::binding(bool portChange, bool ipChange)
{
    BindingRequest request;
    if(ipChange || portChange)
    {
        request.setChangeRequest(portChange, ipChange);
    }
    sendMessage(&request);
    
    // Receive and resend every 200 ms until timeout
    Message* response = NULL;
    for(int i = 0; i < _timeout/200; i++)
    {
        response = receiveMessage(200);
        if(response == NULL)
        {
            sendMessage(&request);
        }
        else
        {
            break;
        }
    }
    return dynamic_cast<BindingResponse*>(response);
}

STUN_END
