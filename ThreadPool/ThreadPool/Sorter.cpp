#include "Sorter.h"

//Входной поток читает файл в память, нарезает его на части и создает
//несколько заданий на сортировку(по числу сортирующих потоков),
//которые помещает в очередь заданий.Сортирующие потоки извлекают задания, 
//сортируют свои части файла, отдают все результаты
//выходному потоку.Выходной поток дожидается всех сортированных частей и 
//объединяет их методом сортирующего слияния.

void Sorter::loadAndSort()
{
	const int TIMEOUT = 1;
	const int TOTAL_THREADS = 5;

	printf_s("\nParts to sort\t");
	int parts;
	scanf_s("%i", &parts);
	if (parts <= 0)
	{
		parts = TOTAL_THREADS;
	}

	ThreadPool * threadpool = new ThreadPool(parts, TIMEOUT);

	std::wstring source = Utils::selectOpeningFile(nullptr);
	std::vector<std::wstring> * sourceStrings = new std::vector<std::wstring>(Utils::loadStringsFromFile(source));
	std::vector<std::vector<std::wstring>*> * packsToSort = new std::vector<std::vector<std::wstring>*>(parts);

	int totalStrings = (int) sourceStrings->size();

	int * packsBounds = new int[parts + 1];
	packsBounds[0] = 0;
	for (size_t i = 1; i < parts + 1; i++)
	{
		packsBounds[i] = (totalStrings / parts) + packsBounds[i - 1];
	}
	packsBounds[parts] = totalStrings - 1;
	
	for (size_t i = 0; i < parts; i++)
	{
		(*packsToSort)[i] = new std::vector<std::wstring>(
			sourceStrings->begin() +
			packsBounds[i],
			sourceStrings->begin() + 
			packsBounds[i + 1]);
	}

	for (size_t i = 0; i < parts; i++)
	{ 
		Utils::sortStrings((*packsToSort)[i]);
	}

	std::vector<std::wstring> * mergedStrings = new std::vector<std::wstring> (/*totalStrings*/);
	for (size_t i = 0; i < parts; i++)
	{
		??mergedStrings->insert((mergedStrings)->end(), (&packsToSort[i])->begin(), (&packsToSort[i])->end());
	}

	std::wstring ** readyToSortItems = Utils::vectorToArray(*mergedStrings);
	Utils::mergeSort(readyToSortItems, totalStrings);
	std::vector<std::wstring> readyStrings = Utils::arrayToVector(readyToSortItems, totalStrings);

	std::wstring destination = Utils::selectSavingFile(nullptr);
	bool result = Utils::writeToFile(destination, readyStrings);


	// free variables
	for (size_t i = 0; i < totalStrings; i++)
	{
		free(readyToSortItems[i]);
	}
	free(readyToSortItems);
	delete sourceStrings;
	delete[] packsBounds;
	for (size_t i = 0; i < parts; i++)
	{
		for (size_t j = 0; j < ((*packsToSort)[i])->size(); j++)
		{
			delete (*packsToSort)[i];
		}
	}
	delete mergedStrings;
	delete packsToSort;
	threadpool->closeSafely();
	delete threadpool;
}

void Sorter::sort(void * params)
{

}

void Sorter::mergeAndOutput(void * params)
{

}

