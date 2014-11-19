#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Error.h>
#include "ogl.h"

using namespace Kore;

namespace {
	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}

	int getPower2(int i) {
		for (int power = 0; ; ++power)
			if (pow(power) >= i) return pow(power);
	}

	void convertImage(Image::Format format, void const * from, int fw, int fh, void* to, int tw, int th) {
		switch (format) {
		case Image::RGBA32:
		case Image::BGRA32:
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					((u32*)to)[tw * y + x] = ((u32*)from)[y * fw + x];
				}
				for (int x = fw; x < tw; ++x) {
					((u32*)to)[tw * y + x] = 0;
				}
			}
			for (int y = fh; y < th; ++y) {
				for (int x = 0; x < tw; ++x) {
					((u32*)to)[tw * y + x] = 0;
				}
			}
			break;
		case Image::Grey8:
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					((u8*)to)[tw * y + x] = ((u8*)from)[y * fw + x];
				}
				for (int x = fw; x < tw; ++x) {
					((u8*)to)[tw * y + x] = 0;
				}
			}
			for (int y = fh; y < th; ++y) {
				for (int x = 0; x < tw; ++x) {
					((u8*)to)[tw * y + x] = 0;
				}
			}
			break;
		}
	}

	void convertTexture(Image::Format format, void const * from, int tw, void* to, int fw, int fh) {
		switch (format) {
		case Image::RGBA32:
		case Image::BGRA32:
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					((u32*)to)[fw * y + x] = ((u32*)from)[y * tw + x];
				}
			}
			break;
		case Image::Grey8:
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					((u8*)to)[fw * y + x] = ((u8*)from)[y * tw + x];
				}
			}
			break;
		}
	}
}


#ifndef GL_LUMINANCE
#define GL_LUMINANCE GL_RED
#endif


Texture::Texture(const char* filename, bool readable)
	: Image(filename)
	, texWidth(getPower2(width))
	, texHeight(getPower2(height))
	, TextureImpl(texWidth != width || texHeight != height)
{
	u8* conversionBuffer;
	if (convert) {
		conversionBuffer = new u8[texWidth * texHeight * sizeOf(format)];
		convertImage(format, data, width, height, conversionBuffer, texWidth, texHeight);
	}
	else {
		conversionBuffer = (u8*)data;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//float color[] = { 1, 0, 0, 0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	
	if (convert) {
		delete[] conversionBuffer;
	}

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

Texture::Texture(int width, int height, Image::Format format, bool readable)
	: Image(width, height, format)
	, texWidth(getPower2(width))
	, texHeight(getPower2(height))
	, TextureImpl(texWidth != width || texHeight != height)
{
	u8* conversionBuffer;
	if (convert) {
		conversionBuffer = new u8[texWidth * texHeight * sizeOf(format)];
		convertImage(format, data, width, height, conversionBuffer, texWidth, texHeight);
	}
	else {
		conversionBuffer = (u8*)data;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	switch (format)
	{
	case Kore::Image::RGBA32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case Kore::Image::Grey8:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, texWidth, texHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case Kore::Image::BGRA32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, texWidth, texHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	default:
		Kore::error("Format not supported!");
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (convert) {
		delete[] conversionBuffer;
	}

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

TextureImpl::~TextureImpl() {
	glDeleteTextures(1, &texture);
}

void Texture::set(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glBindTexture(GL_TEXTURE_2D, texture);
}

u8* Texture::lock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	if (data == nullptr) {
		data = new u8[width * height * sizeOf(format)];
	}

	u8* conversionBuffer;
	if (convert) {
		conversionBuffer = new u8[texWidth * texHeight * sizeOf(format)];
	} else {
		conversionBuffer = (u8*)data;
	}

	switch (format)
	{
	case RGBA32:
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case BGRA32:
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case Grey8:
		glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	default:
		Kore::error("Format not supported!");
	}

	if (convert) {
		convertTexture(format, conversionBuffer, texWidth, data, width, height);
		delete[] conversionBuffer;
	}

	return (u8*)data;
}

void Texture::unlock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	switch (format)
	{
	case RGBA32:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	case BGRA32:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data);
		break;
	case Grey8:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
		break;
	default:
		Kore::error("Format not supported!");
	}

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}







template <Image::Format F>
TypedTexture<F>::TypedTexture(const char* filename, bool readable)
	: TypedImage<F>(filename)
	, texWidth(getPower2(width))
	, texHeight(getPower2(height))
	, texWidthRatio(width / (float)texWidth)
	, texHeightRatio(height / (float)texHeight)
	, TextureImpl(texWidth != width || texHeight != height)
{
	this->readable = readable;

	color_type* conversionBuffer;
	if (convert) {
		conversionBuffer = new color_type[texWidth * texHeight];
		convertImage(format, data, width, height, conversionBuffer, texWidth, texHeight);
	}
	else {
		conversionBuffer = data;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	switch (format)
	{
	case Kore::Image::RGBA32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case Kore::Image::Grey8:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, texWidth, texHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case Kore::Image::BGRA32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, texWidth, texHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	default:
		Kore::error("Format not supported!");
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//float color[] = { 1, 0, 0, 0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

	if (convert) {
		delete[] conversionBuffer;
	}

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

template <Image::Format F>
TypedTexture<F>::TypedTexture(int width, int height, bool readable)
	: TypedImage<F>(width, height)
	, texWidth(getPower2(width))
	, texHeight(getPower2(height))
	, texWidthRatio(width / (float)texWidth)
	, texHeightRatio(height / (float)texHeight)
	, TextureImpl(texWidth != width || texHeight != height)
{
	this->readable = readable;

	color_type* conversionBuffer;
	if (convert) {
		conversionBuffer = new color_type[texWidth * texHeight];
		convertImage(format, data, width, height, conversionBuffer, texWidth, texHeight);
	} else {
		conversionBuffer = data;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	switch (format)
	{
	case Kore::Image::RGBA32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case Kore::Image::Grey8:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, texWidth, texHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	case Kore::Image::BGRA32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, texWidth, texHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, conversionBuffer);
		break;
	default:
		Kore::error("Format not supported!");
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (convert) {
		delete[] conversionBuffer;
	}

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}


template <Image::Format F>
void TypedTexture<F>::set(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glBindTexture(GL_TEXTURE_2D, texture);
}

template <>
typename TypedTexture<Image::Format::RGBA32>::color_type* TypedTexture<Image::Format::RGBA32>::lock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	if (data == nullptr) {
		data = new color_type[width * height];
	}

	if (convert) {
		color_type* conversionBuffer = new color_type[texWidth * texHeight];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
		convertTexture(format, conversionBuffer, texWidth, data, width, height);
		delete[] conversionBuffer;
	} else {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	return data;
}
template <>
typename TypedTexture<Image::Format::BGRA32>::color_type* TypedTexture<Image::Format::BGRA32>::lock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	if (data == nullptr) {
		data = new color_type[width * height];
	}

	if (convert) {
		color_type* conversionBuffer = new color_type[texWidth * texHeight];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, conversionBuffer);
		convertTexture(format, conversionBuffer, texWidth, data, width, height);
		delete[] conversionBuffer;
	} else {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	}

	return data;
}
template <>
typename TypedTexture<Image::Format::Grey8>::color_type* TypedTexture<Image::Format::Grey8>::lock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	if (data == nullptr) {
		data = new color_type[width * height];
	}

	if (convert) {
		color_type* conversionBuffer = new color_type[texWidth * texHeight];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
		convertTexture(format, conversionBuffer, texWidth, data, width, height);
		delete[] conversionBuffer;
	} else {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	}

	return data;
}

template <>
void TypedTexture<Image::Format::RGBA32>::unlock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}
template <>
void TypedTexture<Image::Format::BGRA32>::unlock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data);

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}
template <>
void TypedTexture<Image::Format::Grey8>::unlock() {
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}


template class TypedTexture<Image::default_format>;
template class TypedTexture<Image::Grey8>;
