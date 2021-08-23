#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace Gino::Utils
{
	std::string WstrToStr(std::wstring str);
	std::wstring StrToWstr(std::string str);
	std::vector<uint8_t> ReadFile(const std::filesystem::path& filePath);
	
	struct ImageData
	{
		uint32_t texWidth, texHeight;
		uint32_t imageSize;
		unsigned char* pixels = nullptr;
		float* hdrFpPixels = nullptr;

		void Release();
		
		ImageData() = default;
		ImageData(unsigned char* pixels, uint32_t imageSize, uint32_t texWidth, uint32_t texHeight);
		ImageData(float* pixels, uint32_t imageSize, uint32_t texWidth, uint32_t texHeight);
		~ImageData();
	};

	ImageData ReadImageFile(const std::filesystem::path& filePath, bool hdr = false);


}
