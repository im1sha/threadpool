#include "Sorter.h"

void Sorter::loadAndSort()
{
	const int DEFAULT_PARTS = 5;

	printf_s("\nParts to sort\t");
	int parts;
	scanf_s("%i", &parts);
	if (parts <= 0)
	{
		parts = DEFAULT_PARTS;
	}

	std::wstring source = Utils::selectOpeningFile(nullptr);
	std::vector<std::wstring> *strings = new std::vector<std::wstring>(Utils::loadStringsFromFile(source));
	//Utils::sortStrings(strings);
	int len = strings->size();
	std::wstring ** arr = Utils::vectorToArray(*strings);
	Utils::mergeSort(arr, (int)strings->size());
	delete strings;
	std::vector<std::wstring> strings2 = Utils::arrayToVector(arr, len);

	std::wstring destination = Utils::selectSavingFile(nullptr);
	bool result = Utils::writeToFile(destination, strings2);

	free(arr);
}

void Sorter::sort(void * params)
{

}

void Sorter::mergeAndOutput(void * params)
{

}
