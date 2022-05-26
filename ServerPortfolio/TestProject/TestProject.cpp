#include <iostream>
#include <vector>
#include <unordered_map>
//import TestModule;
std::unordered_map<const char*, int> v;

class TestClass
{
public:

	void What()
	{
		std::cout << typeid(this).name() << std::endl;
	}
};


int main()
{
	TestClass* t0 = new TestClass();

}