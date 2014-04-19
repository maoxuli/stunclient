//
//  main.cpp
//
//  Created by Maoxu Li on 2/11/12.
//  Copyright (c) 2012 LIM Labs. All rights reserved.
//

#include <stun/Discovery.h>
#include <stun/Network.h>
#include <iostream>

int main(int argc, const char * argv[])
{
    std::cout <<
    "/* \n"
    "STUN client, NAT Discovery, Version 0.1.1 \n"
    "Compliant with IETF RFC 3489 (STUN) spec. \n"
    "Copyright 2012 LIM Labs. \n"
    "*/ \n"
    "\n"
    "Discovery() \n"
    "{ \n";

    network::startup();
    stun::Discovery disc("stunserver.org", 3478);
    disc.discover();
    network::cleanup();

    std::cout  << "} \n";
    return 0;
}
