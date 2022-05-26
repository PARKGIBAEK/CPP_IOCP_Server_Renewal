module;
#include <functional>
#include <source_location>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
export module Helper;

import Types;

using BYTE = unsigned char;
using WCHAR = wchar_t;
static std::vector<BYTE>	ReadFile(const WCHAR* _path)
{
	std::vector<BYTE> ret;

	std::filesystem::path filepath(_path);

	const uint32 fileSize = std::filesystem::file_size(filepath);
	ret.resize(fileSize);

	std::basic_ifstream<BYTE> inputStream(filepath);
	inputStream.read(&ret[0], fileSize);

	return ret;
}
