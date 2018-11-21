#include "Sorter.h"

//Входной поток читает файл в память, нарезает его на части и создает
//несколько заданий на сортировку(по числу сортирующих потоков),
//которые помещает в очередь заданий.Сортирующие потоки извлекают задания, 
//сортируют свои части файла, отдают все результаты
//выходному потоку.Выходной поток дожидается всех сортированных частей и 
//объединяет их методом сортирующего слияния.

void Sorter::loadAndSort(void ** params)
{
	printf("loadAndSort started\n");
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

	// load txt file and its content
	std::wstring sourceFile = Utils::selectOpenFile(nullptr);
	std::vector<std::wstring> content(Utils::loadStringsFromFile(sourceFile));
	if (content.size() == 0)
	{
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
		packsToSort[i] =  std::vector<std::wstring>(content.begin() + packsBounds[i], content.begin() + packsBounds[i + 1]);
	}
	
	// synchronizing items initializing
	CRITICAL_SECTION * accessSection = new CRITICAL_SECTION();
	::InitializeCriticalSectionAndSpinCount(accessSection, 4000);
	HANDLE * readyEvent = new HANDLE();
	*readyEvent = ::CreateEvent(nullptr, true, false, nullptr);
	int * totalCompleted = new int(0);

	// threadpool's arguments initializing
	void ** args = new void*[5];
	int * requiredParts = new int(parts);
	args[1] = totalCompleted;
	args[2] = accessSection;
	args[3] = readyEvent;
	args[4] = requiredParts;

	// threadPool sorts packs 
	for (size_t i = 0; i < parts; i++)
	{
		args[0] = &(packsToSort[i]);		
		UnitOfWork sortPack(Sorter::sort, args);
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
			stringsToMerge[shift + j] =  packsToSort[i][j];
		}
		shift += packsToSort[i].size();
	}

	// threadPool merges strings and outputs them to file
	void ** mergeArgs = new void*[2];
	mergeArgs[0] = stringsToMerge;
	mergeArgs[1] = new int (totalStrings);
	UnitOfWork mergeUnit(Sorter::mergeAndOutput, mergeArgs);
	threadPool->enqueue(mergeUnit);

	// release memory

	delete[] packsToSort;
	delete[] args;
	delete totalCompleted;
	::DeleteCriticalSection(accessSection);
	::CloseHandle(*readyEvent);
	delete accessSection;
	delete readyEvent;
	delete requiredParts;	
	delete[] mergeArgs;
	delete[] packsBounds;
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
	if (*totalCompleted == *requiredParts)
	{
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

