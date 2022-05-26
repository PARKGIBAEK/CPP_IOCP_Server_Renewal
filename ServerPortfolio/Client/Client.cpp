#include "pch.h"
#include <iostream>
#include <thread>

using namespace std::chrono_literals;
int main()
{
	while (true)
	{
		std::cout << "Hello World!\n";
		std::this_thread::sleep_for(1s);
	}

	return 0;
}