#include "pch.h"
#include "Utilities.h"

#include <Windows.h>

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
}