#include "Sorter.h"

//Входной поток читает файл в память, нарезает его на части и создает
//несколько заданий на сортировку(по числу сортирующих потоков),
//которые помещает в очередь заданий.Сортирующие потоки извлекают задания, 
//сортируют свои части файла, отдают все результаты
//выходному потоку.Выходной поток дожидается всех сортированных частей и 
//объединяет их методом сортирующего слияния.

void Sorter::loadAndSort(void * params)
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

	// =======================

	std::wstring sourceFile = Utils::selectOpeningFile(nullptr);
	std::vector<std::wstring> content(Utils::loadStringsFromFile(sourceFile));

	// =======================

	std::vector<std::vector<std::wstring>> packsToSort(parts);
	int totalStrings = (int) content.size();


	int * packsBounds = new int[parts + 1];
	packsBounds[0] = 0;
	for (size_t i = 1; i <= parts; i++)
	{
		packsBounds[i] = (totalStrings / parts) + packsBounds[i - 1];
	}
	packsBounds[parts] = totalStrings - 1;	

	for (size_t i = 0; i < parts; i++)
	{
		packsToSort[i] = std::vector<std::wstring>(content.begin() + packsBounds[i], content.begin() + packsBounds[i + 1]);
	}

	for (size_t i = 0; i < parts; i++)
	{ 
		Utils::sortStrings(&(packsToSort[i]));
	}
	
	content.clear(); 
	for (size_t i = 0; i < parts; i++)
	{
		for (size_t j = 0; j < packsToSort[i].size(); j++)
		{
			content.push_back(packsToSort[i][j]);
		}
	}

	// =======================

	std::wstring * mergedArray = Utils::vectorToArray(content);
	Utils::mergeSort(mergedArray, (int) content.size());
	content = Utils::arrayToVector(mergedArray, (int) content.size());
	std::wstring destination = Utils::selectSavingFile(nullptr);
	bool result = Utils::writeToFile(destination, content);

	// =======================

	threadpool->closeSafely();
	delete threadpool;
	delete[] mergedArray;
	delete[] packsBounds;
}

void Sorter::sort(void * params)
{

}

void Sorter::mergeAndOutput(void * params)
{

}

