//
//  packet.h
//  Comm
//
//  Created by gyf on 12-4-12.
//  Copyright (c) 2012年 G4Next. All rights reserved.
//

#ifndef _G4_NEXT_PACKET_H_
#define _G4_NEXT_PACKET_H_

#include <list>
#include <string>
#include <vector>

#define MAX_G4NEXT_STREAM_LENGTH            1024

class G4InStreamIF
{
public:
    virtual bool getbytes(unsigned char* buffer, unsigned short size) = 0;
    virtual bool skip(unsigned short offset) = 0;
};

class G4SocketStream : public G4InStreamIF
{
public:
    G4SocketStream():_size(0),_offset(0),_socket(0){};
    virtual ~G4SocketStream(){};
public:
    void reset();
    bool recvHeader();
    bool recvBody();
    bool recv();
    virtual bool getbytes(unsigned char* buffer, unsigned short size);
    virtual bool skip(unsigned short offset);
    void copyForTest(const unsigned char* buffer, unsigned short size);
    void setSocket(int socket){_socket = socket;};
    void print();
private:
    unsigned char _buffer[MAX_G4NEXT_STREAM_LENGTH];
    unsigned short _size;
    unsigned short _offset;
    int _socket;
};

class G4InStream
{
public:
    G4InStream(G4InStreamIF& inStream):_inStream(inStream){};
    virtual ~G4InStream(){};
public:
    bool get8(unsigned char& value);
    bool get16(unsigned short& value);
    bool get32(unsigned int& value);
    bool getbytes(unsigned char* buffer, unsigned short size);
    
private:
    G4InStreamIF& _inStream;
};

class G4OutStream
{
public:
    G4OutStream():_offset(0){};
    virtual ~G4OutStream(){};
public:
    void reset();
    bool put8(unsigned char value);
    bool put16(unsigned short value);
    bool put32(unsigned int value);
    bool putbytes(unsigned char* buffer, unsigned short size);
    bool putoffset();
    void print();
    const unsigned char* buffer(){return _buffer;};
    unsigned short size(){return _offset;};
private:
    unsigned char _buffer[MAX_G4NEXT_STREAM_LENGTH];
    unsigned short _offset;
};

#define G4_TLV_TYPE_CHAR            0x01
#define G4_TLV_TYPE_SHORT           0x02
#define G4_TLV_TYPE_INT             0x03
#define G4_TLV_TYPE_STRING          0x04
#define G4_TLV_TYPE_CHAR_ARRAY      0x11
#define G4_TLV_TYPE_SHORT_ARRAY     0x12
#define G4_TLV_TYPE_INT_ARRAY       0x13
#define G4_TLV_TYPE_STRING_ARRAY    0x14

class G4Object
{
public:
    virtual bool toStream(G4OutStream* stream) = 0;
    virtual bool fromStream(G4InStream* stream) = 0;
    virtual ~G4Object(){};
};

class G4CharObject : public G4Object
{
public:
    G4CharObject():_value(0){};
    G4CharObject(unsigned char value):_value(value){};
    virtual ~G4CharObject(){};
public:
    unsigned char getValue(){return _value;};
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    unsigned char _value;
};

class G4CharArrayObject : public G4Object
{
public:
    G4CharArrayObject():_value(NULL){};
    G4CharArrayObject(unsigned char* value, unsigned short count);
    virtual ~G4CharArrayObject();
public:
    unsigned char* getValue(){return _value;};
    unsigned short getCount(){return _count;};
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    unsigned char* _value;
    unsigned short _count;
};

class G4ShortObject : public G4Object
{
public:
    G4ShortObject():_value(0){};
    G4ShortObject(char value):_value(value){};
    virtual ~G4ShortObject(){};
public:
    unsigned short getValue(){return _value;};
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    unsigned short _value;
};

class G4ShortArrayObject : public G4Object
{
public:
    G4ShortArrayObject():_value(NULL){};
    G4ShortArrayObject(unsigned short* value, unsigned short count);
    virtual ~G4ShortArrayObject();
public:
    unsigned short* getValue(){return _value;};
    unsigned short getCount(){return _count;};
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    unsigned short* _value;
    unsigned short _count;
};

class G4IntObject : public G4Object
{
public:
    G4IntObject():_value(0){};
    G4IntObject(char value):_value(value){};
    virtual ~G4IntObject(){};
public:
    unsigned int getValue(){return _value;};
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    unsigned int _value;
};


class G4IntArrayObject : public G4Object
{
public:
    G4IntArrayObject():_value(NULL){};
    G4IntArrayObject(unsigned int* value, unsigned short count);
    virtual ~G4IntArrayObject();
public:
    unsigned int* getValue(){return _value;};
    unsigned short getCount(){return _count;};
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    unsigned int* _value;
    unsigned short _count;
};

class G4StringObject : public G4Object
{
public:
    G4StringObject():_value(NULL){};
    G4StringObject(const char* value);
    virtual ~G4StringObject();
public:
    char* getValue(){return _value;};
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    char* _value;
};

class G4StringArrayObject : public G4Object
{
public:
    G4StringArrayObject():_value(NULL){};
    G4StringArrayObject(char** value, unsigned char count);
    G4StringArrayObject(std::vector<std::string>& value);
    virtual ~G4StringArrayObject();
public:
    char** getValue(){return _value;};
    unsigned char getCount(){return _count;};
    void getValue(std::vector<std::string>& value);
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    void reset();
private:
    char** _value;
    unsigned char _count;
};

class G4TLV
{
public:
    G4TLV(unsigned short tag, unsigned char value);
    G4TLV(unsigned short tag, unsigned short value);
    G4TLV(unsigned short tag, unsigned int value);
    G4TLV(unsigned short tag, const char* value);
    G4TLV(unsigned short tag, unsigned char* value, unsigned short count);
    G4TLV(unsigned short tag, unsigned short* value, unsigned short count);
    G4TLV(unsigned short tag, unsigned int* value, unsigned short count);
    G4TLV(unsigned short tag, char** value, unsigned char count);
    G4TLV(unsigned short tag, std::vector<std::string>& value);
    
    G4TLV():_tag(0), _type(0),_object(NULL){};
    virtual ~G4TLV();
public:
    unsigned short getTag(){return _tag;};
    unsigned char getType(){return _type;};
    
    bool get8(unsigned char& value);
    bool get16(unsigned short& value);
    bool get32(unsigned int& value);
    bool gets(char*& value);
    bool gets(std::string &value);
    bool get8s(unsigned char*& value, unsigned short& count);
    bool get16s(unsigned short*& value, unsigned short& count);
    bool get32s(unsigned int*& value, unsigned short& count);
    bool getss(char**& value, unsigned char& count);
    bool getss(std::vector<std::string>& value);
    
    virtual bool toStream(G4OutStream* stream);
    virtual bool fromStream(G4InStream* stream);
private:
    unsigned short _tag;
    unsigned char _type;
    G4Object* _object;
};


class G4NextPacket
{
public:
    G4NextPacket():_packetId(0),_result(0){};
    G4NextPacket(unsigned short packetId, const char* destObject)
    :_packetId(packetId), _result(0), _destObject(destObject){};
    virtual ~G4NextPacket();
public:
    unsigned short _packetId;
    unsigned char _result;
    unsigned char _serverIndicator;
    std::string _destObject;
public:
    void put8(unsigned short tag, unsigned char value);
    void put16(unsigned short tag, unsigned short value);
    void put32(unsigned short tag, unsigned int value);
    void puts(unsigned short tag, const char* value);
    void put8s(unsigned short tag, unsigned char* value, unsigned short count);
    void put16s(unsigned short tag, unsigned short* value, unsigned short count);
    void put32s(unsigned short tag, unsigned int* value, unsigned short count);
    void putss(unsigned short tag, char** value, unsigned char count);
    void putss(unsigned short tag, std::vector<std::string>& value);
    
    bool get8(unsigned short tag, unsigned char& value);
    bool get16(unsigned short tag, unsigned short& value);
    bool get32(unsigned short tag, unsigned int& value);
    bool gets(unsigned short tag, char*& value);
    bool get8s(unsigned short tag, unsigned char*& value, unsigned short& count);
    bool get16s(unsigned short tag, unsigned short*& value, unsigned short& count);
    bool get32s(unsigned short tag, unsigned int*& value, unsigned short& count);
    bool getss(unsigned short tag, char**& value, unsigned char& count);
    bool getss(unsigned short tag, std::vector<std::string>& value);
    
    bool encode(G4OutStream* stream);
    bool decode(G4InStream* stream);
    bool decodeHeader(G4InStream* stream);
    bool decodeBody(G4InStream* stream);
    
    G4TLV* find(unsigned short tag);
private:
    void remove(unsigned short tag);
private:
    std::list<G4TLV*> _tlvs;
};

#endif
