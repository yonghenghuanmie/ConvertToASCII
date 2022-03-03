#include <cstdlib>
#include <mutex>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>
#include "ConvertToASCII.h"

int main(int argc,char *argv[])
{
	auto arguments=[argc,argv](int i)
	{
		if(argc>i)
			return std::atoi(argv[i]);
		return 1;
	};
	//文件名、数量、起点、位数
	ConvertToASCII c({argv[1],arguments(2),arguments(3),arguments(4)});
	try
	{
		c.MutilThreadConvert(ConvertToASCII::TEXT);
	}
	catch(const std::runtime_error &e)
	{
		std::cout<<e.what()<<std::endl;
	}
	return 0;
}