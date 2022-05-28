#pragma once

class RecvBuffer
{
	constexpr static unsigned int BUFFER_COUNT = 10;
public:
	RecvBuffer(int32 _bufferSize);
	~RecvBuffer();

	void		Clean();
	bool		OnRead(int32 _bytesRead);
	bool		OnWrite(int32 _bytesWritten);

	BYTE*		ReadCursor() { return &buffer[readCursor]; }
	BYTE*		WriteCursor() { return &buffer[writeCursor]; }
	int32		DataSize() { return writeCursor - readCursor; }
	int32		FreeSize() { return capacity - writeCursor; }

private:
	int32			capacity = 0; // ���� ���� ũ��� bufferSize * BUFFER_COUNT
	int32			bufferSize = 0; // ���
	int32			readCursor = 0;
	int32			writeCursor = 0;
	Vector<BYTE>	buffer;
};

