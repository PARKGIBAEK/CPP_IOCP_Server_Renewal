#pragma once

class SendBufferChunk;


class SendBuffer
{
	/* SendBufferChunk에서 버퍼로 사용할 메모리 구간의 정보를 가지고 있다.
	  메모리 구간 시작점(buffer)와 메모리 구간의 길이(writeSize)를 반환 받는 클래스*/
public:
	SendBuffer(std::shared_ptr<SendBufferChunk> _ownerChunk, 
		BYTE* _buffer, uint32 _allocSize);
	~SendBuffer();

	BYTE*	Buffer() const { return buffer; }
	uint32	AllocSize() const  { return allocSize; }
	uint32	WriteSize() const { return writeSize; }
	void	Close(uint32 _writeSize);

private:
	BYTE*								buffer;
	uint32								allocSize;
	uint32								writeSize = 0;
	std::shared_ptr<SendBufferChunk>	ownerChunk;
};


/*======================================
*			SendBufferChunk
======================================*/
class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	constexpr static unsigned int SEND_BUFFER_CHUNK_SIZE = 0x2000;
public:
	SendBufferChunk();
	~SendBufferChunk();

	void						ResetBuffer();
	// SendBufferChunk를 요청한 사이즈 만큼 내어주기
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
	// 다 쓴 SendBufferChunk를 전역 객체인 SendBufferManager->sendBufferChunkPool에 반환
	static void							PushGlobal(SendBufferChunk* _sendBuffer);

private:
	USE_LOCK;
	// pool for SendBufferChunk
	Vector<std::shared_ptr<SendBufferChunk>>	sendBufferChunkPool;
};