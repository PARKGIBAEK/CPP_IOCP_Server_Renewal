#pragma once

class SendBufferChunk;

class SendBuffer
{
public:
	SendBuffer(std::shared_ptr<SendBufferChunk> _ownerChunk, 
		BYTE* _buffer, uint32 _allocSize);
	~SendBuffer();

	BYTE*	Buffer() const { return buffer; }
	uint32	AllocSize() const  { return allocSize; }
	uint32	WriteSize() const { return writeSize; }
	void	Close(uint32 _writeSize);

private:
	BYTE* buffer;
	uint32								allocSize;
	uint32								writeSize = 0;
	std::shared_ptr<SendBufferChunk>	ownerChunk;
};


/*======================================
*			SendBufferChunk
======================================*/
class SendBufferChunk :public std::enable_shared_from_this<SendBufferChunk>
{
	constexpr static unsigned int SEND_BUFFER_CHUNK_SIZE = 0x2000;
public:
	SendBufferChunk();
	~SendBufferChunk();

	void						Reset();
	std::shared_ptr<SendBuffer> Open(uint32 _allocSize);
	void						Close(uint32 _writeSize);

	bool						IsOpen() { return isOpened; }
	BYTE*						Buffer() { return &buffer[usedSize]; }
	uint32						FreeSize() { return static_cast<uint32>(buffer.size()) - usedSize; }
private:
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE> buffer = {};
	bool isOpened = false;
	uint32 usedSize = 0;
};


/*======================================
*			SendBufferManager
======================================*/
class SendBufferManager
{
public:
	std::shared_ptr<SendBuffer>			Open(uint32 _size);

private:
	std::shared_ptr<SendBufferChunk>	Pop();
	void								Push(std::shared_ptr < SendBufferChunk> _sendBuffer);
	static void							PushGlobal(SendBufferChunk* _sendBuffer);

private:
	USE_LOCK;
	Vector<std::shared_ptr<SendBufferChunk>>	sendBufferChunks;
};