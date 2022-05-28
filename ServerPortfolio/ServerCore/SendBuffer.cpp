#include "Core.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(std::shared_ptr<SendBufferChunk> _ownerChunk, BYTE* _buffer, uint32 _allocSize)
	:ownerChunk(_ownerChunk), buffer(_buffer), allocSize(_allocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 _writeSize)
{
	ASSERT_CRASH(allocSize < _writeSize, "_writeSize is larger than allocSize");
	writeSize = _writeSize;
	ownerChunk->Close(_writeSize);
}

/*======================================
 			SendBufferChunk
======================================*/
SendBufferChunk::SendBufferChunk()
{
	//...
}

SendBufferChunk::~SendBufferChunk()
{
	//...
}

void SendBufferChunk::ResetBuffer()
{
	isOpened = false;
	usedSize = 0;
}

std::shared_ptr<SendBuffer> SendBufferChunk::Open(uint32 _allocSize)
{
	ASSERT_CRASH(_allocSize > SEND_BUFFER_CHUNK_SIZE, 
		"_allocSize is larger than SEND_BUFFER_CHUNK_SIZE");
	ASSERT_CRASH(isOpened == true, "SendBufferChunk is already opened");

	if (_allocSize > FreeSize())
		return  nullptr;

	isOpened = true;

	return ObjectPool<SendBuffer>::MakeShared(
		shared_from_this(), Buffer(), _allocSize);
}

void SendBufferChunk::Close(uint32 _writeSize)
{
	ASSERT_CRASH(isOpened == false, 
		"isOpened should be true before calling Close()");
	isOpened = false;
	usedSize += _writeSize;
}


/*======================================
*			SendBufferManager
======================================*/

std::shared_ptr<SendBuffer> SendBufferManager::Open(uint32 _size)
{
	if (tls_SendBufferChunk == nullptr)
	{
		tls_SendBufferChunk = Pop(); // WRITE_LOCK 
		tls_SendBufferChunk->ResetBuffer();
	}
	ASSERT_CRASH(tls_SendBufferChunk->IsOpen() == true,
		"SendBufferChunk should be close before using");

	if (tls_SendBufferChunk->FreeSize() < _size)
	{// 여유 공간이 요청 사이즈보다 작으면 새로운 SendBufferChunk 꺼내기
		tls_SendBufferChunk = Pop();
		tls_SendBufferChunk->ResetBuffer();
	}

	return tls_SendBufferChunk->Open(_size);
}

std::shared_ptr<SendBufferChunk> SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (sendBufferChunkPool.empty() == false)
		{
			std::shared_ptr<SendBufferChunk> sendBufferChunk
				= sendBufferChunkPool.back();
			sendBufferChunkPool.pop_back();
			return sendBufferChunk;
		}
	}

	return std::shared_ptr<SendBufferChunk>(
		XNew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(std::shared_ptr<SendBufferChunk> _sendBuffer)
{
	WRITE_LOCK;
	sendBufferChunkPool.emplace_back(_sendBuffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* _sendBuffer)
{
	G_SendBufferManager->Push(
		std::shared_ptr<SendBufferChunk>(_sendBuffer, PushGlobal));
}