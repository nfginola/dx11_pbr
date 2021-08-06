#pragma once

#include <string>
#include <vector>

namespace Gino::Utils
{
	std::string WstrToStr(std::wstring str);
	std::wstring StrToWstr(std::string str);
	std::vector<uint8_t> ReadFile(const std::string& filePath);


}
