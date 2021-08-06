#include "pch.h"
#include "Utilities.h"

#include <Windows.h>
#include <fstream>

namespace Gino::Utils
{
	std::string WstrToStr(std::wstring wstr)
	{
		if (wstr.empty()) return std::string();
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
		return strTo;
	}
	std::wstring StrToWstr(std::string str)
	{
		if (str.empty()) return std::wstring();
		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring wstrTo(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], sizeNeeded);
		return wstrTo;
	}

	std::vector<uint8_t> ReadFile(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			assert(false);

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<uint8_t> buffer(fileSize);

		file.seekg(0); 
		file.read((char*)buffer.data(), fileSize);    

		file.close();
		return buffer;
	}

}