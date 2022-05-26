#include "Core.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(uint32 _bufferSize) :bufferSize(_bufferSize)
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

bool RecvBuffer::OnRead(uint32 _bytesRead)
{

	if (_bytesRead > DataSize())// WSABUF�� ������ ���̸� �Է����ֱ� ������ ���� ������ ������ �ִ� ��
		return false;

	readCursor += _bytesRead;
	return true;
}

bool RecvBuffer::OnWrite(uint32 _bytesWritten)
{

	if (_bytesWritten > FreeSize()) // WSABUF�� ������ ���̸� �Է����ֱ� ������ ���� ������ ������ �ִ� ��
		return false;

	writeCursor += _bytesWritten;
	return true;
}
