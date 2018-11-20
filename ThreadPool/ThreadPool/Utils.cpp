#include "Utils.h"

std::wstring Utils::selectOpeningFile(HWND hWnd)
{
	wchar_t fileName[_MAX_PATH] = {};
	OPENFILENAME openFileName;
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = hWnd;
	openFileName.hInstance = nullptr;
	openFileName.lpstrFilter = L"Text files\0*.txt\0\0";
	openFileName.lpstrCustomFilter = nullptr;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = sizeof(fileName);
	openFileName.lpstrFileTitle = nullptr;
	openFileName.lpstrInitialDir = nullptr;
	openFileName.lpstrTitle = L"Select text file";
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFileName.lpstrDefExt = nullptr;
	::GetOpenFileName(&openFileName);
	return std::wstring(fileName);
}

std::wstring Utils::selectSavingFile(HWND hWnd)
{
	wchar_t fileName[_MAX_PATH] = {};
	OPENFILENAME openFileName;
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = hWnd;
	openFileName.hInstance = nullptr;
	openFileName.lpstrFilter = L"Text files\0*.txt\0\0";
	openFileName.lpstrCustomFilter = nullptr;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = sizeof(fileName);
	openFileName.lpstrFileTitle = nullptr;
	openFileName.lpstrInitialDir = nullptr;
	openFileName.lpstrTitle = L"Save file";
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFileName.lpstrDefExt = nullptr;
	::GetSaveFileName(&openFileName);
	return std::wstring(fileName);
}

std::vector<std::wstring> Utils::loadStringsFromFile(std::wstring fileName)
{
	std::vector<std::wstring> loadedStrings;

	if (0 == ::_waccess(fileName.c_str(), 04)) // Read - only access
	{
		std::wifstream stream(fileName, std::wios::binary);

		// set file input format as utf-8
		stream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		std::wstring line;

		if (stream.is_open())
		{		
			while (std::getline(stream, line))
			{
				loadedStrings.push_back(line);
			}							
			stream.close();
		}		
	}

	return loadedStrings;
}

void Utils::sortStrings(std::vector<std::wstring>* strings)
{
	std::sort((*strings).begin(), (*strings).end());
}

bool Utils::writeToFile(std::wstring path, std::vector<std::wstring> strings) 
{	
	bool result = false;

	std::wofstream stream(path, std::wios::binary);

	if (0 == ::_waccess(path.c_str(), 02)) // where 02 for write-only access
	{
		// set file input format as utf-8
		stream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		if (stream.is_open())
		{
			for (auto s : strings)
			{
				stream << s;
			}
			stream.close();
		}		
		result = true;		
	}

	return result;
}

void Utils::merge(std::vector<std::wstring> *a, int n, int m) {

	int i, j, k;

	std::vector<std::wstring> *x = new std::vector<std::wstring>(n);

	std::fill(x->begin(), x->begin() + x->size() - 1, L"");

	for (i = 0, j = m, k = 0; k < n; k++) {
		(*x)[k] = 
			j == n					? (*a)[i++]
			: i == m				? (*a)[j++]
			: (*a)[j] < (*a)[i]		? (*a)[j++]
			: (*a)[i++];
	}

	for (i = 0; i < n; i++) {
		(*a)[i] = (*x)[i];
	}

	delete x;

}

void Utils::margeSort(std::vector<std::wstring> *a, int n) {
	if (n < 2)
		return;
	int m = n / 2;
	margeSort(a, m);
	margeSort(a + m, n - m);
	merge(a, n, m);
}
