#include "../res/InputMemoryBitStream.h"
#include "../res/OutputMemoryBitStream.h"
#include <iostream>

int main()
{
	std::string str = "hello world";

	OutputMemoryBitStream out; 
	out.WriteString(str, 7);

	InputMemoryBitStream in(out.GetBufferPtr(), out.GetBitLength());

	std::string result = in.ReadString(7);

	std::cout << "Initially: " << str << ", Result: " << result << std::endl;

	system("pause");
	return 0;
}