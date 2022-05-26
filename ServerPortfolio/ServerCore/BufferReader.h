#pragma once

class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* _buffer, uint32 _size, uint32 _readPos = 0);
	~BufferReader();

	BYTE* Buffer() { return buffer; }
	uint32 Size() { return size; }
	uint32 ReadSize() { return readCursor; }
	uint32 FreeSize() { return size - readCursor; }

	template<typename T>
	bool Peek(T& _dest) { return Peek(_dest, sizeof(T)); }
	bool Peek(void* _dest, uint32 _len);
	bool Read(void* _dest, uint32 _len);

	
	template <typename T>
	BufferReader& operator>>(OUT T& _dest);

private:
	BYTE* buffer = nullptr;
	uint32 size = 0;
	uint32 readCursor = 0;
};

template<typename T>
inline BufferReader& BufferReader::operator>>(OUT T& _dest)
{
	/*
		_dest에 readCursor 위치의 포인터가 참조하고 있는 바이너리 데이터 대입

		예시)
		BufferReader br(buffer, len);

		PacketHeader header;
		br >> header;

		uint64 id;
		uint32 hp;
		uint16 attack;
		// 이 함수의 반환 타입이 const가 아닌 이유는 아래와 같이 '>>' 를 연속으로 사용하기 위함
		br >> id >> hp >> attack;
	*/
	if (sizeof(T) > FreeSize())
	{
		// TODO : 버퍼 읽기 범위 초과 에러 처리
		return nullptr;
	}
	_dest = *reinterpret_cast<T*>(&buffer[readCursor]);
	readCursor += sizeof(T); // T크기 만큼 읽은 만큼 커서 당겨야 함
	return *this;
}