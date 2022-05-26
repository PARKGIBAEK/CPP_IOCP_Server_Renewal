#include "Core.h"
#include "BufferWriter.h"

BufferWriter::BufferWriter()
{
	//...
}

BufferWriter::BufferWriter(BYTE* _buffer, uint32 _size, uint32 _writePos)
	:buffer(_buffer), size(_size), writeCursor(_writePos)
{
	//...
}

BufferWriter::~BufferWriter()
{
	//...
}

bool BufferWriter::Write(void* _src, uint32 _len)
{
	if (FreeSize() < _len)
		return false;
	::memcpy(&buffer[writeCursor], _src, _len);
	writeCursor += _len;
	return true;
}
