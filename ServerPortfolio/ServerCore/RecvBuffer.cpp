#include "Core.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int32 _bufferSize) :bufferSize(_bufferSize)
{
	capacity = _bufferSize * BUFFER_COUNT;
	buffer.resize(capacity);
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clean()
{
	auto dataSize = DataSize();
	if (dataSize == 0)
	{
		readCursor = writeCursor = 0;
	}
	else if (FreeSize() < bufferSize)
	{
		::memcpy(&buffer[0], &buffer[readCursor], dataSize);
		readCursor = 0;
		writeCursor = dataSize;
	}

}

bool RecvBuffer::OnRead(int32 _bytesRead)
{

	if (_bytesRead > DataSize())// WSABUF에 버퍼의 길이를 입력해주기 때문에 여기 들어오면 문제가 있는 것
		return false;

	readCursor += _bytesRead;
	return true;
}

bool RecvBuffer::OnWrite(int32 _bytesWritten)
{

	if (_bytesWritten > FreeSize()) // WSABUF에 버퍼의 길이를 입력해주기 때문에 여기 들어오면 문제가 있는 것
		return false;

	writeCursor += _bytesWritten;
	return true;
}
