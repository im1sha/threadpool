#include "Sorter.h"

//������� ����� ������ ���� � ������, �������� ��� �� ����� � �������
//��������� ������� �� ����������(�� ����� ����������� �������),
//������� �������� � ������� �������.����������� ������ ��������� �������, 
//��������� ���� ����� �����, ������ ��� ����������
//��������� ������.�������� ����� ���������� ���� ������������� ������ � 
//���������� �� ������� ������������ �������.

void Sorter::loadAndSort(void ** params)
{
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

	std::vector<std::vector<std::wstring>> packsToSort(parts);
	int totalStrings = (int) content.size();

	// determine packs bounds
	int * packsBounds = new int[parts + 1];
	packsBounds[0] = 0;
	for (size_t i = 1; i < parts; i++)
	{
		packsBounds[i] = (totalStrings / parts) + packsBounds[i - 1];
	}
	packsBounds[parts] = totalStrings - 1;	

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
	void ** args = new void*[5];
	int * requiredParts = new int(parts);

	// threadPool sorts packs 
	for (size_t i = 0; i < parts; i++)
	{
		args[0] = &packsToSort[i];
		args[1] = totalCompleted;
		args[2] = accessSection;
		args[3] = readyEvent;
		args[4] = requiredParts;
		UnitOfWork sortPack(Sorter::sort, args);
		threadPool->enqueue(sortPack);
	}
	::WaitForSingleObject(readyEvent, INFINITE);

	// release memory
	delete[] args;
	delete totalCompleted;
	::DeleteCriticalSection(accessSection);
	::CloseHandle(*readyEvent);
	delete accessSection;
	delete readyEvent;
	delete requiredParts;

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
	std::wstring destination = Utils::selectSaveFile(nullptr);
	bool result = Utils::writeToFile(destination, content);

	// free memory 
	delete[] mergedArray;
	delete[] packsBounds;
}

void Sorter::sort(void ** params)
{
	std::vector<std::wstring> * v = (std::vector<std::wstring> *) params[0];
	Utils::sortStrings(v);
	int * totalCompleted = (int *) params[1];
	CRITICAL_SECTION * accessSection = (CRITICAL_SECTION *) params[2];
	HANDLE * readyEvent = (HANDLE *) params[3];
	int * requiredParts = (int *) params[4];
	::EnterCriticalSection(accessSection);
	(*totalCompleted)++;	
	if (*totalCompleted == *requiredParts)
	{
		::SetEvent(readyEvent);
	}
	::LeaveCriticalSection(accessSection);
}

void Sorter::mergeAndOutput(void ** params)
{

}

