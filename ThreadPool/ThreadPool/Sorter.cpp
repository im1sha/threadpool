#include "Sorter.h"

void Sorter::loadAndSort(void ** params)
{
	printf("+ loadAndSort started\n");
	ThreadPool * threadPool = (ThreadPool *) params[0];
	if (threadPool == nullptr)
	{
		return;
	}

	// total tasks to enqueue
	int parts = threadPool->getMaxThreads(); 
	if (parts > 1)
	{
		parts--; // 1 enqueued task is current  
	}
	else
	{
		return;
	}

	// load txt file and its content
	std::wstring sourceFile = Utils::selectOpenFile(nullptr);
	std::vector<std::wstring> content(Utils::loadStringsFromFile(sourceFile));
	if (content.size() == 0)
	{
		printf("no content");
		return;
	}

	std::vector<std::wstring> * packsToSort = new std::vector<std::wstring>[parts];
	int totalStrings = (int) content.size();

	// determine packs bounds
	int * packsBounds = new int[parts + 1];
	packsBounds[0] = 0;
	for (size_t i = 1; i < parts; i++)
	{
		packsBounds[i] = (totalStrings / parts) + packsBounds[i - 1];
	}
	packsBounds[parts] = totalStrings;	

	// place file content into packs
	for (size_t i = 0; i < parts; i++)
	{
		packsToSort[i] = std::vector<std::wstring>(content.begin() + packsBounds[i], content.begin() + packsBounds[i + 1]);
	}
	
	// synchronizing items initializing
	CRITICAL_SECTION * accessSection = new CRITICAL_SECTION();
	::InitializeCriticalSectionAndSpinCount(accessSection, 4000);
	HANDLE * readyEvent = new HANDLE();
	*readyEvent = ::CreateEvent(nullptr, true, false, nullptr);
	int * totalCompleted = new int(0);

	// threadpool's arguments initializing
	int * requiredParts = new int(parts);
	size_t argsTotal = 5;

	void *** args = new void**[parts];
	for (size_t i = 0; i < parts; i++)
	{
		args[i] = new void*[argsTotal] {
			nullptr, totalCompleted, accessSection, readyEvent, requiredParts
		};
	}

	// threadPool sorts packs 
	for (size_t i = 0; i < parts; i++)
	{
		args[i][0] = &(packsToSort[i]);		
		UnitOfWork sortPack(Sorter::sort, args[i]);
		threadPool->enqueue(sortPack);
	}
	::WaitForSingleObject(*readyEvent, 60000);

	// preparing strings for merge
	std::wstring * stringsToMerge = new std::wstring [totalStrings];
	size_t shift = 0;
	for (size_t i = 0; i < parts; i++)
	{
		for (size_t j = 0; j < packsToSort[i].size(); j++)
		{
			stringsToMerge[shift + j] = packsToSort[i][j];
		}
		shift += packsToSort[i].size();
	}

	// threadPool merges strings and outputs them to file
	size_t mergeArgsTotal = 2;
	void ** mergeArgs = new void*[mergeArgsTotal] {
		stringsToMerge, new int(totalStrings) 
	};
	UnitOfWork mergeUnit(Sorter::mergeAndOutput, mergeArgs);
	threadPool->enqueue(mergeUnit);

	// release memory

	delete[] packsToSort;
	for (int i = 0; i < parts; i++)
	{
		delete[] args[i];
	}
	delete[] args;
	delete totalCompleted;
	::DeleteCriticalSection(accessSection);
	::CloseHandle(*readyEvent);
	delete accessSection;
	delete readyEvent;
	delete requiredParts;	
	delete[] mergeArgs;
	delete[] packsBounds;
	printf("+ loadAndSort finished\n");
}

void Sorter::sort(void ** params)
{
	std::vector<std::wstring> * strings = (std::vector<std::wstring> *) params[0];	
	int * totalCompleted = (int *) params[1];
	CRITICAL_SECTION * accessSection = (CRITICAL_SECTION *) params[2];
	HANDLE * readyEvent = (HANDLE *) params[3];
	int * requiredParts = (int *) params[4];
	
	Utils::sortStrings(strings);

	::EnterCriticalSection(accessSection);	
	(*totalCompleted)++;	
	printf("totalCompleted==%i\n", *totalCompleted);
	if (*totalCompleted == *requiredParts)
	{
		printf("::SetEvent(*readyEvent)\n");
		::SetEvent(*readyEvent);
	}
	::LeaveCriticalSection(accessSection);
}

void Sorter::mergeAndOutput(void ** params)
{
	std::wstring * mergedArray = (std::wstring *) params[0];
	int * totalStrings = (int *)params[1];
	Utils::mergeSort(mergedArray, *totalStrings);
	std::vector<std::wstring> content = Utils::arrayToVector(mergedArray, *totalStrings);

	std::wstring destination = Utils::selectSaveFile(nullptr);
	bool result = Utils::writeToFile(destination, content);

	printf("+ output is ready\n");
	delete[] mergedArray;
	delete totalStrings;
}

