#include "stdafx.h"
#include "FolderIterator.h"
#include "Implement\LogUserImpl.h"
#include "Client\DataHelper.h"
#include "Implement\DataObject.h"

#include <regex>
#include <string>
#include <vector>
#include <io.h>
#include <iomanip>
#include <ctime>
#include <direct.h>
#include <iostream>

#pragma warning(disable:4996)

using namespace Yap;
using namespace std;

FolderIterator::FolderIterator() :
	ProcessorImpl(L"FolderIterator")
{
	AddInput(L"Input", YAP_ANY_DIMENSION, DataTypeAll);
	AddOutput(L"Output", YAP_ANY_DIMENSION, DataTypeAll);

	AddProperty<const wchar_t * const>(L"Path", L"", L"文件夹目录");
	AddProperty<bool>(L"SearchSubDir", false, L"搜索当前Path目录下的子目录文件夹");
	AddProperty<const wchar_t * const>(L"Regular", L"", L"文件件搜索正则表达式");
}

FolderIterator::~FolderIterator()
{
}

FolderIterator::FolderIterator(const FolderIterator& rhs) : 
	ProcessorImpl(rhs)
{
}

bool FolderIterator::Input(const wchar_t * name, IData * data)
{
	assert(data == nullptr || (Inputs()->Find(name) != nullptr));

	auto path = GetProperty<const wchar_t * const>(L"Path");
	bool recursive = GetProperty<bool>(L"SearchSubDir");
	/* regular expression */
	const wchar_t * c_reg = GetProperty<const wchar_t* const>(L"RegularExpression");

	auto folders = GetFolders(path, recursive, c_reg);

	if (folders.empty())
	{
		LOG_ERROR(L"Property [Path] is not aproperate a directories", L"PythonRecon");
		return false;
	}

	for (auto iter : folders)
	{
		VariableSpace variables;
		variables.Add(L"string", L"FolderPath", L"Full path of one sub-folder in the current folder");
		variables.Set(L"FolderPath", iter.c_str());

		auto output = DataObject<int>::CreateVariableObject(variables.Variables(), _module.get());

		Feed(L"Output", output.get());
	}
	return true;
}

bool FolderIterator::InputObsolete(const wchar_t * name, IData * data)
{
	assert(data == nullptr || (Inputs()->Find(name) != nullptr));

	string path = ToMbs(GetProperty<const wchar_t * const>(L"Path"));
	bool is_subfolder = GetProperty<bool>(L"SearchSubDir");
	/* regular */
	const wchar_t * c_reg = GetProperty<const wchar_t* const>(L"Regular");

	vector<string> folders;
	//GetFolders(path, folders, is_subfolder,c_reg);

	if (folders.empty())
	{
		LOG_ERROR(L"Property [Path] is not aproperate a directories", L"PythonRecon");
		return false;
	}

	unsigned int count = 0;
	TODO("how 'SetProperty' & 'GetProperty' work in Multi-thread or multi-processor?");
	SetProperty<bool>(L"FolderIteratorFinished", false);
	for (auto iter:folders)
	{
		/*
		//@note  This dimensions created is not real dimension of data, it just for
		//  sending message with dimensions; And the data sent is a folder name only.
		// DimensionUser1 --> marks folder #number.
		// DimensionUser2 --> size of current folder name string.
		*/
		Dimensions dimension;
		dimension(DimensionUser1, count, folders.size())(DimensionUser2, 0U, unsigned(iter.length()+1));
		++count;
		std::cout << count << "(/" << folders.size() << ") Iterator" << std::endl;
		std::cout << iter.c_str() << std::endl;
		char * tempt = new  char[iter.length() + 1];
		strcpy(tempt, iter.c_str());
		auto out_data = CreateData<char>(nullptr, tempt, dimension);
		Feed(L"Output", out_data.get());
	}
	SetProperty<bool>(L"FolderIteratorFinished", true);
	return true;
}

string FolderIterator::ToMbs(const wchar_t * wcs)
{
	assert(wcslen(wcs) < 500 && "The string cannot contain more than 500 characters.");

	static char buffer[1024];
	size_t size;
	wcstombs_s(&size, buffer, (size_t)1024, wcs, (size_t)1024);
	return std::string(buffer);
}

vector<wstring> FolderIterator::GetFolders(const wchar_t * path_i, 
	bool recursive,
	const wchar_t * regex_) 
{
	wstring path{ path_i };
	vector<wstring> folders;

	assert(path.length() != 0);
	if (path.length() >= _MAX_FNAME || int(path.find(L' ')) != -1)
	{
		return folders;
	}

	return folders;
// 
// 	if (path.rfind(L'\\') == (path.length() - 1))
// 		path.pop_back();
// 
// 	long long   hFile = 0;
// 	_finddata_t file_info;// file information struct.
// 	string temp;
// 
// 	if ((hFile = _findfirst(((temp.assign(ToMbs(path.c_str))).append("/*")).c_str(), &file_info)) != -1)
// 	{
// 		do 
// 		{
// 			if ((file_info.attrib & _A_SUBDIR)) 
// 			{
// 				if (strcmp(file_info.name, ".") != 0 && strcmp(file_info.name, "..") != 0)
// 				{	
// 					if (wcslen(regex_)!=0)
// 					{
// 						std::regex re(ToMbs(regex_));
// 						if (regex_match(file_info.name, re))
// 						{
// 							folders.push_back(((temp.assign(path)).append("//")).append(file_info.name));
// 						}
// 					}
// 					else
// 					{
// 						folders.push_back(((temp.assign(path)).append("//")).append(file_info.name));
// 					}
// 					if (recursive)
// 					{
// 						GetFolders(temp.assign(path).append("//").append(file_info.name), folders, recursive,regex_);
// 					}
// 				}
// 			}
// 		} while (_findnext(hFile, &file_info) == 0);
// 
// 		_findclose(hFile);
// 	}
}
