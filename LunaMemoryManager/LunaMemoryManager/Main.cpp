#include <iostream>

#include "LunaMM.h"


#define ARRSIZE 50

int main(void)
{
	Luna memoryManager;

	memoryManager.PrintPool();
	system("pause");

	long double* testAllocs[ARRSIZE];
	for (int i = 0; i < ARRSIZE; ++i)
	{
		testAllocs[i] = LunaOp::Create<long double>(memoryManager);

		memoryManager.PrintPool();
		system("pause");
	}

	for (int i = 0; i < ARRSIZE; ++i)
	{
		LunaOp::Destroy(testAllocs[i], memoryManager);

		memoryManager.PrintPool();
		system("pause");
	}

	return 0;
}
