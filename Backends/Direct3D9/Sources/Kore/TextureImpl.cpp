#include "pch.h"
#include "TextureImpl.h"
#include <Kore/WinError.h>
#include "Direct3D9.h"

using namespace Kore;

namespace {
	TextureImpl* setTextures[16] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
									 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	D3DFORMAT convert(Image::Format format) {
		switch (format) {
		case Image::Format::Grey8:
			return D3DFMT_L8;
		case Image::Format::RGBA32:
			Kore::error("Texture Format RGBA32 = D3DFMT_A8B8G8R8 not supported!");
		case Image::Format::BGRA32:
		default:
			return D3DFMT_A8R8G8B8;
		}
	}
}

void TextureImpl::construct(int width, int height, Image::Format format, void const * data) {
	stage = 0;
	mipmap = true;
	DWORD usage = 0;
	usage = D3DUSAGE_DYNAMIC;
	affirm(device->CreateTexture(width, height, 1, usage, ::convert(format), D3DPOOL_DEFAULT, &texture, 0), "Texture creation failed.");
	D3DLOCKED_RECT rect;
	affirm(texture->LockRect(0, &rect, 0, 0));
	u8* from = (u8*)data;
	u8* to = (u8*)rect.pBits;
	memcpy(to, from, width * height * Image::sizeOf(format));
	affirm(texture->UnlockRect(0));
}

Texture::Texture(const char* filename, bool readable /*= false*/) : Image(filename, default_format) {
	construct(width, height, format, data);
	texWidth = width;
	texHeight = height;

	this->readable = readable;
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format) {
	construct(width, height, format, data);
	texWidth = width;
	texHeight = height;

	this->readable = readable;
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

TextureImpl::~TextureImpl() {
	unset();
	texture->Release();
}

void Texture::set(TextureUnit unit) {
	affirm(device->SetTexture(unit.unit, texture));
	this->stage = unit.unit;
	setTextures[stage] = this;
}

void TextureImpl::unset() {
	if (setTextures[stage] == (void*)this) {
		device->SetTexture(stage, nullptr);
		setTextures[stage] = nullptr;
	}
}

u8* Texture::lock() {
	D3DLOCKED_RECT rect;
	affirm(texture->LockRect(0, &rect, 0, 0));
	return (u8*)rect.pBits;
}

void Texture::unlock() {
	affirm(texture->UnlockRect(0));
}



template<Image::Format F> 
TypedTexture<F>::TypedTexture(int width, int height, bool readable)
	: TypedImage<F>(width, height)
	, texWidth(width)
	, texHeight(height)
	, texWidthRatio(1.0f)
	, texHeightRatio(1.0f)
{
	construct(width, height, format, data);

	this->readable = readable;
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

template<Image::Format F>
TypedTexture<F>::TypedTexture(const char* filename, bool readable = false)
	: TypedImage<F>(filename)
	, texWidth(width)
	, texHeight(height)
	, texWidthRatio(1.0f)
	, texHeightRatio(1.0f)
{
	construct(width, height, format, data);

	this->readable = readable;
	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}


template<Image::Format F>
void TypedTexture<F>::set(TextureUnit unit) {
	affirm(device->SetTexture(unit.unit, texture));
	this->stage = unit.unit;
	setTextures[stage] = this;
}

template<Image::Format F>
typename TypedTexture<F>::color_type* TypedTexture<F>::lock() {
	D3DLOCKED_RECT rect;
	affirm(texture->LockRect(0, &rect, 0, 0));
	return (color_type*)rect.pBits;
}

template<Image::Format F>
void TypedTexture<F>::unlock() {
	affirm(texture->UnlockRect(0));
}

template class TypedTexture<Image::default_format>;
template class TypedTexture<Image::Format::Grey8>;
