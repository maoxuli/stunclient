//
//  Message.h
//  stun
//
//  Created by Maoxu Li on 2/11/12.
//  Copyright (c) 2012 LIM Labs. All rights reserved.
//

#ifndef STUN_MESSAGE_H
#define STUN_MESSAGE_H

#include <stun/Config.h>
#include <stun/UUID.h>
#include <stun/Buffer.h>

STUN_BEGIN

/* 
 RFC 3489 (March 2003)
 11.1  Message Header
 All STUN messages consist of a 20 byte header:
 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      STUN Message Type        |         Message Length        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 Transaction ID
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 

 The message length is the count, in bytes, of the size of the
 message, not including the 20 byte header.
 */

#define MESSAGE_HEADER_LENGTH 20

/*
 The Message Types can take on the following values:
 0x0001  :  Binding Request
 0x0101  :  Binding Response
 0x0111  :  Binding Error Response
 0x0002  :  Shared Secret Request
 0x0102  :  Shared Secret Response
 0x0112  :  Shared Secret Error Response
 */
enum MESSAGE_TYPE
{
    MT_BINDING_REQUEST              = 0x0001,
    MT_BINDING_RESPONSE             = 0x0101,
    MT_BINDING_ERROR_RESPONSE       = 0x0111,
    MT_SHARED_SECRET_REQUEST        = 0x0002,
    MT_SHARED_SECRET_RESPONSE       = 0x0102,
    MT_SHARED_SECRET_ERROR_RESPONSE = 0x0112
};
    
/*
 RFC 3489 (March 2003)
 11.2  Message Attributes
 After the header are 0 or more attributes.  Each attribute is TLV
 encoded, with a 16 bit type, 16 bit length, and variable value:
 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |         Type                  |            Length             |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                             Value                             ....
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define ATTRIBUTE_HEADER_LENGTH 4
    
/*
 The following types are defined:
 0x0001: MAPPED-ADDRESS
 0x0002: RESPONSE-ADDRESS
 0x0003: CHANGE-REQUEST
 0x0004: SOURCE-ADDRESS
 0x0005: CHANGED-ADDRESS
 0x0006: USERNAME
 0x0007: PASSWORD
 0x0008: MESSAGE-INTEGRITY
 0x0009: ERROR-CODE
 0x000a: UNKNOWN-ATTRIBUTES
 0x000b: REFLECTED-FROM
 */
    
enum ATTRIBUTE_TYPE
{
    AT_MAPPED_ADDRESS       = 0x0001,
    AT_RESPONSE_ADDRESS     = 0x0002,
    AT_CHANGE_REQUEST       = 0x0003,
    AT_SOURCE_ADDRESS       = 0x0004,
    AT_CHANGED_ADDRESS      = 0x0005,
    AT_USER_NAME            = 0x0006,
    AT_PASS_WORD            = 0x0007,
    AT_MESSAGE_INTEGRITY    = 0x0008,
    AT_ERROR_CODE           = 0x0009,
    AT_UNKNOWN_ATTRIBUTES   = 0x000a,
    AT_REFLECTED_FROM       = 0x000b
};
    
class Attribute
{
public:
    Attribute(unsigned short type, size_t length = 0);
    virtual ~Attribute();
        
    unsigned short type() const;
    size_t length() const;
    
    // Pack into buffer
    size_t toBuffer(network::Buffer* buf) const;
    bool fromBuffer(network::Buffer* buf);
    
    // Attribute define the memory format
    // so knows how to parse type field
    static unsigned short checkType(network::Buffer* buf);
    
protected:
    Attribute();
    
    unsigned short _type;
    size_t _length; // Memory length of value
    
    // Pack and unpack value
    // Redefined by derived class
    virtual size_t valueToBuffer(network::Buffer* buf) const;
    virtual bool valueFromBuffer(network::Buffer* buf);
};
    
class Message
{
public:
    Message(MESSAGE_TYPE type);
    Message(MESSAGE_TYPE type, const network::UUID& tid);
    virtual ~Message();
    
    // Fields of message header
    MESSAGE_TYPE type() const;
    network::UUID tid() const;
    
    // Pack into buffer
    size_t toBuffer(network::Buffer* buf) const;
    bool fromBuffer(network::Buffer* buf);
    
    virtual std::string toString() const;
    
    static unsigned short checkType(network::Buffer* buf);
    
protected:
    Message();
    
    MESSAGE_TYPE _type;
    network::UUID _tid;
    
    std::vector<Attribute*> _attributes;
    
    size_t length() const; // Calculate length of all attributes
    void setAttribute(Attribute* attribute);
    Attribute* findAttribute(ATTRIBUTE_TYPE type) const;
    
};
    
class MessageFactory
{
public:
    static Message* fromBuffer(network::Buffer* buf);
};

class BindingRequest : public Message
{
public:
    BindingRequest();
    virtual ~BindingRequest();
    
    // Attributes
    void setResponseAddress(const sockaddr_in& sa);
    void setChangeRequest(bool port, bool ip = false);
    void setUserName(const std::string& name);
    void setMessageIntegrity();
};

class BindingResponse : public Message
{
public:
    BindingResponse();
    virtual ~BindingResponse();
    
    // Attributes
    sockaddr_in mappedAddress() const;
    sockaddr_in sourceAddress() const;
    sockaddr_in changedAddress() const;
    sockaddr_in reflectedFrom() const;
    bool messageIntegrity() const;
};

class BindingErrorResponse : public Message
{
public:
    BindingErrorResponse();
    virtual ~BindingErrorResponse();
    
    //Attributes
    int errorClass() const;
    int errorNumber() const;
    int errorCode() const;
    std::string errorReason();
    
    std::vector<ATTRIBUTE_TYPE> getUnknownAttributes() const;
};
    
class AttributeFactory
{
public:
    static Attribute* fromBuffer(network::Buffer* buf);
};
    
class AddressAttribute : public Attribute
{
public:
    AddressAttribute(ATTRIBUTE_TYPE type);
    AddressAttribute(ATTRIBUTE_TYPE type, const sockaddr_in& sa);
    virtual ~AddressAttribute();
        
    sockaddr_in address() const;

    // Customized packing and parsing of value
    virtual size_t valueToBuffer(network::Buffer* buf) const;
    virtual bool valueFromBuffer(network::Buffer* buf);
    
private:
    // Attribute value
    sockaddr_in _address;
};
    
class ChangeRequestAttribute : public Attribute
{
public:
    ChangeRequestAttribute(bool port, bool ip = false);
    virtual ~ChangeRequestAttribute();
    
    // Customized packing and parsing for value
    virtual size_t valueToBuffer(network::Buffer* buf) const;
    virtual bool valueFromBuffer(network::Buffer* buf);
    
private:
    bool _portChange;
    bool _ipChange;
};

STUN_END

#endif
