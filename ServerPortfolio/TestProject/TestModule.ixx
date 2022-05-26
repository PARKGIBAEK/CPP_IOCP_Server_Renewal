module;
#include <iostream>
#include <filesystem>
#include <source_location>
#include <functional>
export module TestModule;


export void Assert(std::function<bool()>&& _expression, const char* _message)
{
	if (_expression())
	{
		const std::source_location sl = std::source_location::current();
		std::cout << "Error :\n>> File location : " << sl.file_name() << "\n"
			<< ">> Line : " << sl.line() << "\n"
			<< ">> Column : " << sl.column() << "\n"
			<< ">> Description : " << _message << std::endl;
		std::terminate();
	}
}

export class Test
{
public:
	int a = 0;

	std::vector<int> v;
};