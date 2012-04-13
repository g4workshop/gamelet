//
//  packet.cpp
//  Comm
//
//  Created by gyf on 12-4-12.
//  Copyright (c) 2012年 G4Next. All rights reserved.
//

#import "packet.h"
#import <sys/socket.h>
#import <string.h>

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
void G4SocketStream::reset()
{
    _size = 0;
    _offset = 0;
}

bool G4SocketStream::recvHeader()
{
    if(::recv(_socket, _buffer, 2, 0) != 2)
        return false;
    _size = _buffer[1] << 8 | _buffer[0];
    _offset = 2;
    return (_size < MAX_G4NEXT_STREAM_LENGTH - 2);
}

bool G4SocketStream::recvBody()
{
    return (::recv(_socket, &_buffer[_offset], _size, 0) == _size);    
}

bool G4SocketStream::recv()
{
    return recvHeader() && recvBody();
}

bool G4SocketStream::getbytes(unsigned char* buffer, unsigned short size)
{
    if(_offset + size > _size + 2)
        return false;
    ::memcpy(buffer, &_buffer[_offset], size);
    _offset += size;
    return true;
}

void G4SocketStream::copyForTest(const unsigned char* buffer, unsigned short size)
{
    ::memcpy(_buffer, buffer, size);
    _size = size;
    _offset = 0;
}

bool G4SocketStream::skip(unsigned short offset)
{
    if(_offset + offset > _size + 2)
        return false;
    _offset += offset;
    return true;
}

void G4SocketStream::print()
{
    printf("SOCKET:");
    for(unsigned short i = 0; i < _size + 2; i++)
    {
        printf("%02X ", _buffer[i]);
    }
    printf("\n");
}
////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
bool G4InStream::get8(unsigned char& value)
{
    return _inStream.getbytes(&value, 1);
}

bool G4InStream::get16(unsigned short& value)
{
    unsigned char buffer[2];
    if(!_inStream.getbytes(buffer, 2))
        return false;
    value = (buffer[0] | buffer[1] << 8);
    return true;
}

bool G4InStream::get32(unsigned int& value)
{
    unsigned char buffer[4];
    if(!_inStream.getbytes(buffer, 4))
        return false;
    value = (buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24);
    return true;
}

bool G4InStream::getbytes(unsigned char* buffer, unsigned short size)
{
    if(size == 0) return true;
    return _inStream.getbytes(buffer, size);
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////

bool G4OutStream::put8(unsigned char value)
{
    return putbytes(&value, 1);
}

bool G4OutStream::put16(unsigned short value)
{
    unsigned char buffer[2];
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
    return putbytes(buffer, 2);
}

bool G4OutStream::put32(unsigned int value)
{
    unsigned char buffer[4];
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
    return putbytes(buffer, 4);
}

bool G4OutStream::putbytes(unsigned char* buffer, unsigned short size)
{
    if(_offset + size > MAX_G4NEXT_STREAM_LENGTH)
        return false;
    ::memcpy(&_buffer[_offset], buffer, size);
    _offset += size;
    return true;
}
     
bool G4OutStream::putoffset()
{
    unsigned short offset = _offset;
    _offset = 0;
    bool ret = put16(offset - 2);
    _offset = offset;
    return ret;
}

void G4OutStream::print()
{
    printf("OUT:");
    for(unsigned short i = 0; i < _offset; i++)
    {
        printf("%02X ", _buffer[i]);
    }
    printf("\n");
}

void G4OutStream::reset()
{
    _offset = 0;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
bool G4CharObject::toStream(G4OutStream* stream)
{
    return stream->put8((unsigned char)_value);
}

bool G4CharObject::fromStream(G4InStream* stream)
{
    return stream->get8(_value);
}


////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
G4CharArrayObject::G4CharArrayObject(unsigned char* value, unsigned short count)
{
    _count = count;
    _value = new unsigned char[_count];
    ::memcpy(_value, value, _count);
}

G4CharArrayObject::~G4CharArrayObject()
{
    if(_value)
        delete[] _value;
}

bool G4CharArrayObject::toStream(G4OutStream* stream)
{
    if(!stream->put16(_count))
        return false;
    if(0 == _count)
        return true;
    return stream->putbytes(_value, _count);
}

bool G4CharArrayObject::fromStream(G4InStream* stream)
{
    if(!stream->get16(_count))
        return false;
    if(_value)
        delete[] _value;
    _value = NULL;
    if(0 == _count)
        return true;
    _value = new unsigned char[_count];
    return stream->getbytes(_value, _count);
}


////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
bool G4ShortObject::toStream(G4OutStream* stream)
{
    return stream->put16(_value);
}

bool G4ShortObject::fromStream(G4InStream* stream)
{
    return stream->get16(_value);
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////

G4ShortArrayObject::G4ShortArrayObject(unsigned short* value, unsigned short count)
{
    _count = count;
    _value = new unsigned short[_count];
    ::memcpy(_value, value, count * sizeof(short));
}

G4ShortArrayObject::~G4ShortArrayObject()
{
    if(_value)
        delete[] _value;
}

bool G4ShortArrayObject::toStream(G4OutStream* stream)
{
    if(!stream->put16(_count))
        return false;
    for(unsigned short i = 0; i < _count; i++)
    {
        if(!stream->put16(_value[i]))
            return false;
    }
    return true;
}

bool G4ShortArrayObject::fromStream(G4InStream* stream)
{
    if(!stream->get16(_count))
        return false;
    if(_value)
        delete[] _value;
    _value = NULL;
    if(_count == 0)
        return true;
    _value = new unsigned short[_count];
    for(unsigned short i = 0; i < _count; i++)
    {
        if(!stream->get16(_value[i]))
            return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////

bool G4IntObject::toStream(G4OutStream* stream)
{
    return stream->put32(_value);
}

bool G4IntObject::fromStream(G4InStream* stream)
{
    return stream->get32(_value);
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
G4IntArrayObject::G4IntArrayObject(unsigned int* value, unsigned short count)
{
    _count = count;
    _value = new unsigned int[_count];
    ::memcpy(_value, value, count * sizeof(int));
}

G4IntArrayObject::~G4IntArrayObject()
{
    if(_value)
        delete[] _value;
}

bool G4IntArrayObject::toStream(G4OutStream* stream)
{
    if(!stream->put16(_count))
        return false;
    for(unsigned short i = 0; i < _count; i++)
    {
        if(!stream->put32(_value[i]))
            return false;
    }
    return true;
}

bool G4IntArrayObject::fromStream(G4InStream* stream)
{
    if(!stream->get16(_count))
        return false;
    if(_value)
        delete[] _value;
    _value = NULL;
    if(_count == 0)
        return true;
    _value = new unsigned int[_count];
    for(unsigned short i = 0; i < _count; i++)
    {
        if(!stream->get32(_value[i]))
            return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////

G4StringObject::G4StringObject(const char* value)
{
    if(!value)
        _value = NULL;
    else
    {
        _value = new char[strlen(value) + 1];
        strcpy(_value, value);
    }
}

G4StringObject::~G4StringObject()
{
    if(_value)
        delete[] _value;
}

bool G4StringObject::toStream(G4OutStream* stream)
{
    unsigned char length = 0;
    if(_value)
        length = (unsigned char)strlen(_value);
    if(!stream->put8(length))
        return false;
    return stream->putbytes((unsigned char*)_value, length);
}

bool G4StringObject::fromStream(G4InStream* stream)
{
    unsigned char length = 0;
    if(!stream->get8(length))
        return false;
    if(length == 0)
        return true;
    _value = new char[length + 1];
    _value[length] = '\0';
    return stream->getbytes((unsigned char*)_value, length);
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////

G4StringArrayObject::G4StringArrayObject(char** value, unsigned char count)
{
    _count = count;
    _value = new char*[_count];
    for(unsigned char i = 0; i < _count; i++)
    {
        _value[i] = new char[::strlen(value[i]) + 1];
        strcpy(_value[i], value[i]);
    }
}

G4StringArrayObject::G4StringArrayObject(std::vector<std::string>& value)
{
    _count = value.size();
    _value = new char*[_count];
    for(unsigned char i = 0; i < _count; i++)
    {
        _value[i] = new char[value[i].size() + 1];
        strcpy(_value[i], value[i].c_str());
    }
}

void G4StringArrayObject::getValue(std::vector<std::string>& value)
{
    value.resize(_count);
    for(unsigned char i = 0; i < _count; i++)
        value[i] = _value[i];
}

G4StringArrayObject::~G4StringArrayObject()
{
    reset();
}

void G4StringArrayObject::reset()
{
    if(_value)
    {
        for(unsigned char i = 0; i < _count; i++)
            delete[] _value[i];
        delete[] _value;
    }
    _count = 0;
    _value = NULL;
}

bool G4StringArrayObject::toStream(G4OutStream* stream)
{
    if(!stream->put8(_count))
        return false;
    for(unsigned char i = 0; i < _count; i++)
    {
        unsigned char len = strlen(_value[i]);
        if(!stream->put8(len))
            return false;
        if(len == 0)
            continue;
        if(!stream->putbytes((unsigned char*)_value[i], len))
            return false;
    }
    return true;
}

bool G4StringArrayObject::fromStream(G4InStream* stream)
{
    reset();
    if(!stream->get8(_count))
        return false;
    if(_count == 0)
        return true;
    _value = new char*[_count];
    for(unsigned char i = 0; i < _count; i++)
    {
        unsigned char len = 0;
        if(!stream->get8(len))
            return false;
        if(len == 0)
            continue;
        _value[i] = new char[len + 1];
        _value[i][len] = '\0';
        if(!stream->getbytes((unsigned char*)_value[i], len))
            return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////

G4TLV::G4TLV(unsigned short tag, unsigned char value)
:_tag(tag), _type(G4_TLV_TYPE_CHAR)
{
    _object = new G4CharObject(value);
}

G4TLV::G4TLV(unsigned short tag, unsigned short value)
:_tag(tag), _type(G4_TLV_TYPE_SHORT)
{
    _object = new G4ShortObject(value);
}

G4TLV::G4TLV(unsigned short tag, unsigned int value)
:_tag(tag), _type(G4_TLV_TYPE_INT)
{
    _object = new G4IntObject(value);
}

G4TLV::G4TLV(unsigned short tag, const char* value)
:_tag(tag), _type(G4_TLV_TYPE_STRING)
{
    _object = new G4StringObject(value);
}

G4TLV::G4TLV(unsigned short tag, unsigned char* value, unsigned short count)
:_tag(tag), _type(G4_TLV_TYPE_CHAR_ARRAY)
{
    _object = new G4CharArrayObject(value, count);
}

G4TLV::G4TLV(unsigned short tag, unsigned short* value, unsigned short count)
:_tag(tag), _type(G4_TLV_TYPE_SHORT_ARRAY)
{
    _object = new G4ShortArrayObject(value, count);
}

G4TLV::G4TLV(unsigned short tag, unsigned int* value, unsigned short count)
:_tag(tag), _type(G4_TLV_TYPE_INT_ARRAY)
{
    _object = new G4IntArrayObject(value, count);
}

G4TLV::G4TLV(unsigned short tag, char** value, unsigned char count)
:_tag(tag), _type(G4_TLV_TYPE_STRING_ARRAY)
{
    _object = new G4StringArrayObject(value, count);
}

G4TLV::G4TLV(unsigned short tag, std::vector<std::string>& value)
:_tag(tag), _type(G4_TLV_TYPE_STRING_ARRAY)
{
    _object = new G4StringArrayObject(value);
}

G4TLV::~G4TLV()
{
    if(_object)
        delete _object;
}

bool G4TLV::get8(unsigned char& value)
{
    if(_type != G4_TLV_TYPE_CHAR)
        return false;
    value = ((G4CharObject*)_object)->getValue();
    return true;
}

bool G4TLV::get16(unsigned short& value)
{
    if(_type != G4_TLV_TYPE_SHORT)
        return false;
    value = ((G4ShortObject*)_object)->getValue();
    return true;
}

bool G4TLV::get32(unsigned int& value)
{
    if(_type != G4_TLV_TYPE_INT)
        return false;
    value = ((G4IntObject*)_object)->getValue();
    return true;
}

bool G4TLV::gets(char*& value)
{
    if(_type != G4_TLV_TYPE_STRING)
        return false;
    value = ((G4StringObject*)_object)->getValue();
    return true;
}

bool G4TLV::gets(std::string &value)
{
    if(_type != G4_TLV_TYPE_STRING)
        return false;
    char *tmp = ((G4StringObject*)_object)->getValue();;
    if (tmp)
        value = tmp;
    return true;
}

bool G4TLV::get8s(unsigned char*& value, unsigned short& count)
{
    if(_type != G4_TLV_TYPE_CHAR_ARRAY)
        return false;
    value = ((G4CharArrayObject*)_object)->getValue();
    count = ((G4CharArrayObject*)_object)->getCount();
    return true;
}

bool G4TLV::get16s(unsigned short*& value, unsigned short& count)
{
    if(_type != G4_TLV_TYPE_SHORT_ARRAY)
        return false;
    value = ((G4ShortArrayObject*)_object)->getValue();
    count = ((G4ShortArrayObject*)_object)->getCount();
    return true;
}

bool G4TLV::get32s(unsigned int*& value, unsigned short& count)
{
    if(_type != G4_TLV_TYPE_INT_ARRAY)
        return false;
    value = ((G4IntArrayObject*)_object)->getValue();
    count = ((G4IntArrayObject*)_object)->getCount();
    return true;
}

bool G4TLV::getss(char**& value, unsigned char& count)
{
    if(_type != G4_TLV_TYPE_STRING_ARRAY)
        return false;
    value = ((G4StringArrayObject*)_object)->getValue();
    count = ((G4StringArrayObject*)_object)->getCount();
    return true;
}

bool G4TLV::getss(std::vector<std::string>& value)
{
    if(_type != G4_TLV_TYPE_STRING_ARRAY)
        return false;
    ((G4StringArrayObject*)_object)->getValue(value);
    return true;
}

bool G4TLV::toStream(G4OutStream* stream)
{
    if(!stream->put16(_tag))
        return false;
    if(!stream->put8(_type))
        return false;
    return _object->toStream(stream);
}

bool G4TLV::fromStream(G4InStream* stream)
{
    if(!stream->get16(_tag))
        return false;
    if(!stream->get8(_type))
        return false;
    switch (_type) {
        case G4_TLV_TYPE_CHAR:
            _object = new G4CharObject();
            break;
        case G4_TLV_TYPE_SHORT:
            _object = new G4ShortObject();
            break;
        case G4_TLV_TYPE_INT:
            _object = new G4IntObject();
            break;
        case G4_TLV_TYPE_STRING:
            _object = new G4StringObject();
            break;
        case G4_TLV_TYPE_CHAR_ARRAY:
            _object = new G4CharArrayObject();
            break;
        case G4_TLV_TYPE_SHORT_ARRAY:
            _object = new G4ShortArrayObject();
            break;
        case G4_TLV_TYPE_INT_ARRAY:
            _object = new G4IntArrayObject();
            break;
        case G4_TLV_TYPE_STRING_ARRAY:
            _object = new G4StringArrayObject();
            break;
        default:
            return false;
    }
    return _object->fromStream(stream);
}


////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
G4NextPacket::~G4NextPacket()
{
    for(std::list<G4TLV*>::iterator it = _tlvs.begin(); it != _tlvs.end(); it++)
        delete (*it);
    _tlvs.clear();
}

void G4NextPacket::put8(unsigned short tag, unsigned char value)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value);
    _tlvs.push_back(tlv);
}

void G4NextPacket::put16(unsigned short tag, unsigned short value)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value);
    _tlvs.push_back(tlv);
}

void G4NextPacket::put32(unsigned short tag, unsigned int value)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value);
    _tlvs.push_back(tlv);
}

void G4NextPacket::puts(unsigned short tag, const char* value)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value);
    _tlvs.push_back(tlv);
}

void G4NextPacket::put8s(unsigned short tag, unsigned char* value, unsigned short count)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value, count);
    _tlvs.push_back(tlv);
}

void G4NextPacket::put16s(unsigned short tag, unsigned short* value, unsigned short count)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value, count);
    _tlvs.push_back(tlv);
}

void G4NextPacket::put32s(unsigned short tag, unsigned int* value, unsigned short count)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value, count);
    _tlvs.push_back(tlv);
}

void G4NextPacket::putss(unsigned short tag, char** value, unsigned char count)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value, count);
    _tlvs.push_back(tlv);
}

void G4NextPacket::putss(unsigned short tag, std::vector<std::string>& value)
{
    remove(tag);
    G4TLV* tlv = new G4TLV(tag, value);
    _tlvs.push_back(tlv);
}

bool G4NextPacket::get8(unsigned short tag, unsigned char& value)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->get8(value);
}

bool G4NextPacket::get16(unsigned short tag, unsigned short& value)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->get16(value);
}

bool G4NextPacket::get32(unsigned short tag, unsigned int& value)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->get32(value);
}

bool G4NextPacket::gets(unsigned short tag, char*& value)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->gets(value);
}

bool G4NextPacket::get8s(unsigned short tag, unsigned char*& value, unsigned short& count)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->get8s(value, count);
}

bool G4NextPacket::get16s(unsigned short tag, unsigned short*& value, unsigned short& count)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->get16s(value, count);
}

bool G4NextPacket::get32s(unsigned short tag, unsigned int*& value, unsigned short& count)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->get32s(value, count);
}

bool G4NextPacket::getss(unsigned short tag, char**& value, unsigned char& count)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->getss(value, count);
}

bool G4NextPacket::getss(unsigned short tag, std::vector<std::string>& value)
{
    G4TLV* tlv = find(tag);
    if(!tlv)
        return false;
    return tlv->getss(value);
}

bool G4NextPacket::encode(G4OutStream* stream)
{
    if(!stream->put16(0))         //长度
        return false;
    
    if(!stream->put8(_serverIndicator))
        return false;
    
    unsigned char len = _destObject.length();
    if(!stream->put8(len))
        return false;
    if(!stream->putbytes((unsigned char*)_destObject.c_str(), len))
        return false;
    
    if(!stream->put16(_packetId))
        return false;
    if(!stream->put8(_result))
        return false;
    unsigned char count = _tlvs.size();
    if(!stream->put8(count))
        return false;
    for(std::list<G4TLV*>::iterator it = _tlvs.begin(); it != _tlvs.end(); it++)
    {
        G4TLV* tlv = (*it);
        if(!tlv->toStream(stream))
            return false;
    }
    return stream->putoffset();
}

bool G4NextPacket::decodeHeader(G4InStream* stream)
{
//    unsigned short offset;
//    if(!stream->get16(offset))
//        return false;
    if(!stream->get8(_serverIndicator))
        return false;
    unsigned char len;
    if(!stream->get8(len))
        return false;
    char value[256];
    memset(value, 0, 256);
    if(!stream->getbytes((unsigned char*)value, len))
        return false;
    _destObject = value;   
    return true;
}

bool G4NextPacket::decodeBody(G4InStream* stream)
{
    if(!stream->get16(_packetId))
        return false;
    if(!stream->get8(_result))
        return false;
    unsigned char count;
    if(!stream->get8(count))
        return false;
    for(unsigned char i = 0; i < count; i++)
    {
        G4TLV* tlv = new G4TLV();
        _tlvs.push_back(tlv);
        if(!tlv->fromStream(stream))
            return false;
    }
    return true;
}

bool G4NextPacket::decode(G4InStream* stream)
{
    return decodeHeader(stream) && decodeBody(stream);
}

void G4NextPacket::remove(unsigned short tag)
{
    for(std::list<G4TLV*>::iterator it = _tlvs.begin(); it != _tlvs.end(); it++)
    {
        if((*it)->getTag() == tag)
        {
            delete (*it);
            _tlvs.erase(it);
            break;
        }
    }
}

G4TLV* G4NextPacket::find(unsigned short tag)
{
    for(std::list<G4TLV*>::iterator it = _tlvs.begin(); it != _tlvs.end(); it++)
    {
        if((*it)->getTag() == tag)
            return (*it);
    }
    return NULL;
}















