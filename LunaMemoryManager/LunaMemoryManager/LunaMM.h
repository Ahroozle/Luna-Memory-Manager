#pragma once

#include <exception>

class pool_overflow_exception : public std::exception
{
public:
	pool_overflow_exception(char* str) throw() : exception(str, 1) {}
};

class invalid_pool_return_exception : public std::exception
{
public:
	invalid_pool_return_exception(char* str) throw() : exception(str, 1) {}
};

// LUNA stack-based memory manager
// Coded by Ahroo

// Stack size is 1MB (1048576 bytes)
// This stakes out roughly half the stack for use as memory space.
#define LUNA_POOLSIZE 500/*500000*/

class Luna final
{
private:

	Luna(Luna&) = delete;
	Luna(Luna&&) = delete;
	Luna& operator=(Luna&) = delete;
	Luna& operator=(Luna&&) = delete;

	struct MemPool
	{
		struct MemBlock
		{
			// Bit field; the alive is a single bit in the uint while the length is the rest.
			// Length includes the memory block itself.
			// the bit pattern is as follows:
			// ALLL LLLL   LLLL LLLL   LLLL LLLL   LLLL LLLL
			unsigned int length : 31, alive : 1;
		};

		unsigned char mem[LUNA_POOLSIZE];
		MemBlock* first;
	};

	MemPool pool;

public:

	Luna();
	~Luna();

	// Gives you a chunk of the stack-based array which is the exact size you want.
	// NOTE: WILL RETURN NULLPTR IF OUT OF MEMORY
	unsigned char* Request(unsigned int numBytes);

	// Returns a given chunk to the stack-based array, allowing it to be used for other purposes.
	void Return(unsigned char* memblock);

	// Prints the pool to cout, to be visualized. The output is color-coded to show headers and bodies inside the pool.
	void PrintPool();
};

namespace LunaOp
{
	template<typename T>
	T* Create(Luna& _lunaRef)
	{
		unsigned char* memchunk = nullptr;

		try
		{
			memchunk = _lunaRef.Request(sizeof(T));
		}
		catch (const pool_overflow_exception& e)
		{
			std::cout << e.what() << "Bad day today, eh? \n";
			system("pause");
		}

		return reinterpret_cast<T*>(memchunk);
	}

	template<typename T>
	void Destroy(T* _toDestroy, Luna& _lunaRef)
	{
		unsigned char* memchunk = reinterpret_cast<unsigned char*>(_toDestroy);

		try
		{
			_lunaRef.Return(memchunk);
		}
		catch (const invalid_pool_return_exception& e)
		{
			std::cout << e.what() << "Bad day today, eh? \n";
			system("pause");
		}

		return;
	}
}