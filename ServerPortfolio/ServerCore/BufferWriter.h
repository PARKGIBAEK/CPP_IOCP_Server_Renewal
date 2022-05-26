#pragma once

class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* _buffer, uint32 _size, uint32 _writePos = 0);
	~BufferWriter();

	BYTE* Buffer() { return buffer; }
	uint32		Size() { return size; }
	uint32		WriteCursor() { return writeCursor; }
	uint32		FreeSize() { return size - writeCursor; }

	template<typename T>
	bool		Write(T* _src) {
		return Write(_src, sizeof(T));
	}
	bool		Write(void* _src, uint32 _len);

	template<typename T>
	T* Reserve(uint16 _count = 1);

	template<typename T>
	BufferWriter& operator<<(T&& _src);

private:
	BYTE* buffer = nullptr;
	uint32	size = 0;
	uint32	writeCursor = 0;

};

template<typename T>
inline T* BufferWriter::Reserve(uint16 _count)
{
	if (FreeSize() < (sizeof(T) * count))
		return nullptr;

	T* retAddress = reinterpret_cast<T*>(&buffer[writeCursor]);
	writeCursor += sizeof(T) * _count;
	
	return retAddress;
}

template<typename T>
inline BufferWriter& BufferWriter::operator<<(T&& _src)
{
	using DataType = std::remove_reference_t<T>;
	
}
