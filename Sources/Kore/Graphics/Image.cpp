#include "pch.h"
#include "Image.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Error.h>
#include "stb_image.h"
#include <stdio.h>

using namespace Kore;

void _swapRedAndBlue(void* data, int size);

int Image::sizeOf(Image::Format format) {
	switch (format) {
	case Image::Format::RGBA32:
	case Image::Format::BGRA32:
		return 4;
	case Image::Format::Grey8:
		return 1;
	}
	return -1;
}

Image::Image(int width, int height, Format format) : width(width), height(height), format(format), readable(true) {
	data = new u8[width * height * sizeOf(format)];
}

Image::Image(const char* filename, Format format /*= default_format*/) : format(format), readable(true) {
	printf("Image %s\n", filename);
	FileReader file(filename);
	int size = file.size();
	int comp;
	switch (format)
	{
	case Format::Grey8:
		data = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 1);
		break;
	case Format::BGRA32:
		data = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
		_swapRedAndBlue(data, width*height);
		break;
	case Format::RGBA32:
		data = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
		break;
	}
}

Image::~Image() {
	delete[] data;
}

int Image::at(int x, int y) const {
	return *(int*)&((u8*)data)[width * sizeOf(format) * y + x * sizeOf(format)];
}

void _swapRedAndBlue(void* data, int size) {
	for (int i = 0; i < size; ++i) {
		((rgba32*)data)[i] = ((bgra32*)data)[i].toRGBA();
	}
}

void* _greyTo32(void* data, int size) {
	rgba32* newData = new rgba32[size];

	for (int i = 0; i < size; ++i) {
		u8 grey = ((u8*)data)[i];
		newData[i] = rgba32(grey,grey,grey);
	}

	delete [] data;

	return newData;
}

void* _32toGrey(void* data, int size) {
	u8* newData = new u8[size];

	for (int i = 0; i < size; ++i) {
		rgba32 c = ((rgba32*)data)[i];
		u8 grey = (u8)(((u32)c.r + (u32)c.g + (u32)c.g) / 3);
		newData[i] = grey;
	}

	delete[] data;

	return newData;
}

/*
void Image::convert(Format newFormat) {
	if (format == newFormat) return;

	if (format == Format::Grey8 && (newFormat == Format::RGBA32 || newFormat == Format::BGRA32))
	{
		data = _greyTo32(data, width*height);
	}
	else if ((format == Format::RGBA32 || format == Format::BGRA32) && newFormat == Format::Grey8)
	{
		data = _32toGrey(data, width*height);
	}
	else if ((format == Format::RGBA32 && newFormat == Format::BGRA32) || (format == Format::BGRA32 && newFormat == Format::RGBA32))
	{
		_swapRedAndBlue(data, width*height);
	}
	else
	{
		Kore::error("Unknown or not supported image format conversion.");
	}
}
*/
