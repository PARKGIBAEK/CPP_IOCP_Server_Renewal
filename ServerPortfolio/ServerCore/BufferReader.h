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
		_dest�� readCursor ��ġ�� �����Ͱ� �����ϰ� �ִ� ���̳ʸ� ������ ����

		����)
		BufferReader br(buffer, len);

		PacketHeader header;
		br >> header;

		uint64 id;
		uint32 hp;
		uint16 attack;
		// �� �Լ��� ��ȯ Ÿ���� const�� �ƴ� ������ �Ʒ��� ���� '>>' �� �������� ����ϱ� ����
		br >> id >> hp >> attack;
	*/
	if (sizeof(T) > FreeSize())
	{
		// TODO : ���� �б� ���� �ʰ� ���� ó��
		return nullptr;
	}
	_dest = *reinterpret_cast<T*>(&buffer[readCursor]);
	readCursor += sizeof(T); // Tũ�� ��ŭ ���� ��ŭ Ŀ�� ��ܾ� ��
	return *this;
}