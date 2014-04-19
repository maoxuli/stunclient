//
//  Config.h
//  stun
//
//  Created by Maoxu Li on 8/10/10.
//  Copyright (c) 2010 LIM Labs. All rights reserved.
//

#ifndef STUN_CONFIG_H
#define STUN_CONFIG_H

//
// Namespace 
//
#define STUN_NAMESPACE

#ifdef STUN_NAMESPACE
#   define STUN_BEGIN   namespace stun {
#   define STUN_END     }
#else
#   define STUN_BEGIN
#   define STUN_END
#endif

#endif
