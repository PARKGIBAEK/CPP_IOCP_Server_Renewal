#include "Core.h"
#include "BufferReader.h"

BufferReader::BufferReader()
{
}

BufferReader::BufferReader(BYTE* _buffer, uint32 _size, uint32 _readPos)
	:buffer(_buffer), size(_size), readCursor(_readPos)
{
}

BufferReader::~BufferReader()
{
}

bool BufferReader::Peek(void* _dest, uint32 _len)
{
	if (FreeSize() < _len)
		return false;
	::memcpy(_dest, &buffer[readCursor], _len);
	//readCursor += _len;
	return false;
}

bool BufferReader::Read(void* _dest, uint32 _len)
{
	if (false == Peek(_dest, _len))
		return false;
	
	readCursor += _len;
	return false;
}
