// waterside.cpp: 定义应用程序的入口点。
//

#include "waterside.h"
#include "tbb/scalable_allocator.h"
#include "Logger.h"

using namespace std;

int main()
{
	auto& logger = waterside::TLogger<>::instance();
	logger.init("test");

	LOG_ERROR("xxxxxx {}", 1);

	cout << "Hello CMake." << endl;
	tbb::scalable_allocator<int> all;
	auto p = all.allocate(1);
	all.deallocate(p, 1);

	logger.join();
	system("pause");
	return 0;
}
