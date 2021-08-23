#include "pch.h"
#include "Utilities.h"

#include <Windows.h>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

	std::vector<uint8_t> ReadFile(const std::filesystem::path& filePath)
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

	ImageData ReadImageFile(const std::filesystem::path& filePath, bool hdr)
	{
		int texWidth, texHeight, texChannels;
		texWidth = texHeight = texChannels = 0;

		if (!hdr)
		{
			stbi_uc* pixels = stbi_load(filePath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			if (!pixels)
			{
				assert(false);		// failed to load image
			}

			size_t imageSize = texWidth * texHeight * sizeof(uint32_t);
			return ImageData(pixels, static_cast<uint32_t>(imageSize), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		}
		else
		{
			float* pixels = stbi_loadf(filePath.string().c_str(), &texWidth, &texHeight, &texChannels, 4);
			if (!pixels)
			{
				assert(false);		// failed to load image
			}

			size_t imageSize = texWidth * texHeight * sizeof(uint32_t) * 4;		// each texel is 3 floats
			return ImageData(pixels, static_cast<uint32_t>(imageSize), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		}


		// We ignore giving amount of channels as output since we are forcing RGBA (4 channel, 32 bit always)
		// Pixels will be auto cleaned up on ImageData destructor 
	}

	void ImageData::Release()
	{
		if (pixels)
		{
			stbi_image_free(pixels);
		}
		if (hdrFpPixels)
		{
			stbi_image_free(hdrFpPixels);
		}
	}

	ImageData::ImageData(unsigned char* pixels, uint32_t imageSize, uint32_t texWidth, uint32_t texHeight) :
		pixels(pixels),
		imageSize(imageSize),
		texWidth(texWidth),
		texHeight(texHeight)
	{

	}

	ImageData::ImageData(float* pixels, uint32_t imageSize, uint32_t texWidth, uint32_t texHeight) :
		hdrFpPixels(pixels),
		imageSize(imageSize),
		texWidth(texWidth),
		texHeight(texHeight)
	{
	}

	ImageData::~ImageData()
	{
	}

}