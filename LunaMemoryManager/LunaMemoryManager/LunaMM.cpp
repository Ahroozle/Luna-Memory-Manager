
#include "LunaMM.h"

#pragma region DynamicArray

// THIS IS MEANT AS A SIMPLIFICATION CLASS FOR STORING ALL THESE FUCKING INDICES EFFICIENTLY

#pragma endregion


// This exception gets thrown when trying to bite off more memory than there is left.


Luna::Luna()
{
	pool.first = reinterpret_cast<MemPool::MemBlock*>(pool.mem);
	pool.first->alive = 0;
	pool.first->length = LUNA_POOLSIZE;
}

Luna::~Luna()
{

}

unsigned char* Luna::Request(unsigned int numBytes)
{
	// Start from the beginning of the heap
	unsigned char* currHeader = pool.mem;
	MemPool::MemBlock* castedCurr = reinterpret_cast<MemPool::MemBlock*>(currHeader);
	unsigned int currLengthWithoutHeader = castedCurr->length - 4;

	unsigned char* poolEnd = currHeader + LUNA_POOLSIZE;

	// Hop through every location where a header is expected to be.
	while (currHeader != poolEnd)
	{
		// If this header isn't being used...
		if (!castedCurr->alive)
		{

			// If it's exactly the size I want...
			if (currLengthWithoutHeader == numBytes)
			{
				// ... Then we're done!
				castedCurr->alive = 1;
				return currHeader + 4;
			}
			else if (currLengthWithoutHeader > numBytes) // Otherwise, if it's bigger than what I want...
			{
				castedCurr->alive = 1;

				int bytesLeft = currLengthWithoutHeader - numBytes;

				// If I can make another header in the space left over...
				if (bytesLeft >= 4)
				{
					// ... Then I will!
					//
					// NOTE: I know this has the potential to result in blocks which are only a header with no feasible amount of
					// room afterward, as well as some memory that isn't big enough to make a header being left on the end of some
					// given chunks, destined to not be used by that object. I feel that leaving these flaws in is better compared
					// the alternative of adding several more checks which could potentially impede performance.
					MemPool::MemBlock* newHeader = reinterpret_cast<MemPool::MemBlock*>(currHeader + 4 + numBytes);
					newHeader->alive = 0;
					newHeader->length = bytesLeft;

					castedCurr->length -= bytesLeft;
				}

				return currHeader + 4;
			}
		}

		currHeader += castedCurr->length;
		castedCurr = reinterpret_cast<MemPool::MemBlock*>(currHeader);
		currLengthWithoutHeader = castedCurr->length - 4;
	}

	throw pool_overflow_exception("Not enough memory left in the pool!\n");

	return nullptr;
}

void Luna::Return(unsigned char* memblock)
{
	if (memblock == nullptr)
	{
		throw invalid_pool_return_exception("The pointer which was given to be returned to the managed pool was null!\n");
		return;
	}
	else if (memblock < pool.mem || memblock > pool.mem + LUNA_POOLSIZE)
	{
		throw invalid_pool_return_exception("The pointer which was given to be returned to the managed pool points to outside the pool's bounds!\n");
		return;
	}

	// Get the very start of the heap, for use later.
	unsigned char* prevHeader = pool.mem;
	MemPool::MemBlock* castedPrev = reinterpret_cast<MemPool::MemBlock*>(prevHeader);

	// Get the header expected to precede this block of memory.
	unsigned char* currHeader = memblock - 4;
	MemPool::MemBlock* castedCurr = reinterpret_cast<MemPool::MemBlock*>(currHeader);

	// Get the one expected to be next in line.
	unsigned char* nextHeader = currHeader + castedCurr->length;
	MemPool::MemBlock* castedNext = reinterpret_cast<MemPool::MemBlock*>(nextHeader);

	unsigned char* poolEnd = pool.mem + LUNA_POOLSIZE;

	castedCurr->alive = 0;

	// If the next chunk isn't being used...
	if (nextHeader != poolEnd && !castedNext->alive)
	{
		// Combine it with the current one.
		castedCurr->length += castedNext->length;
	}

	// If there even IS any space before the current header
	if (prevHeader != currHeader)
	{
		// Locate the previous header
		while (prevHeader + castedPrev->length != currHeader)
		{
			if (prevHeader == poolEnd)
			{
				//Do some error stuff to explain that the returned memory was not found in the pool

				return;
			}

			prevHeader += castedPrev->length;
			castedPrev = reinterpret_cast<MemPool::MemBlock*>(prevHeader);
		}

		// If the previous header isn't being used...
		if (!castedPrev->alive)
		{
			// Combine the current header with it.
			castedPrev->length += castedCurr->length;
		}
	}

	return;
}


// COPY/PASTED OLD STUFF FROM MY OLD DYNAMIC MEMORY MANAGER BEYOND THIS HORIZON

#include <iostream>
#include <sstream>

#include <Windows.h>

#define PrintByteHex(c) std::cout << (unsigned int)((c & 0xF0) >> 4) << (unsigned int)(c & 0x0F) /*<< ' '*/

void Luna::PrintPool()
{
	unsigned char* currByte = pool.mem;
	unsigned char* lastByte = currByte + LUNA_POOLSIZE;

	std::cout << std::hex;

	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO sbi;

	SetConsoleCursorPosition(hConsole, { 0, 0 });


	GetConsoleScreenBufferInfo(hConsole, &sbi);


	WORD liveHeaderPalette = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
	WORD liveBodyPalette = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_RED;
	WORD deadHeaderPalette = BACKGROUND_BLUE;
	WORD deadBodyPalette = BACKGROUND_RED;

	unsigned int memBlockSize = sizeof(MemPool::MemBlock);
	unsigned int currentLength;
	bool currentAlive;
	while (currByte != lastByte)
	{
		currentLength = ((MemPool::MemBlock*)currByte)->length;
		currentAlive = ((MemPool::MemBlock*)currByte)->alive;

		if (currentAlive)
			SetConsoleTextAttribute(hConsole, liveHeaderPalette);
		else
			SetConsoleTextAttribute(hConsole, deadHeaderPalette);


		for (unsigned int i = 0; i < memBlockSize; ++i)
		{
			PrintByteHex((*currByte));
			++currByte;
		}

		if (currentAlive)
			SetConsoleTextAttribute(hConsole, liveBodyPalette);
		else
			SetConsoleTextAttribute(hConsole, deadBodyPalette);

		for (unsigned int i = memBlockSize; i < currentLength; ++i)
		{
			PrintByteHex((*currByte));
			++currByte;
		}
	}

	std::cout << std::dec << '\n';

	SetConsoleTextAttribute(hConsole, sbi.wAttributes);
}
