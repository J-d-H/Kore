#pragma once

#include <Kore/TextureImpl.h>
#include <Kore/Graphics/Image.h>

namespace Kore {
	class TextureUnit : public TextureUnitImpl {

	};

	class Texture : public Image, public TextureImpl {
	public:
		Texture(int width, int height, Format format, bool readable);
		Texture(const char* filename, bool readable = false);
		void set(TextureUnit unit);
		u8* lock();
		void unlock();
		int texWidth;
		int texHeight;
	};

	template <Image::Format F = Image::default_format>
	class TypedTexture : public TypedImage<F>, public TextureImpl
	{
	public:
		TypedTexture(int width, int height, bool readable);
		TypedTexture(const char* filename, bool readable = false);

		void set(TextureUnit unit);
		color_type* lock();
		void unlock();

		int texWidth;
		int texHeight;
		float texWidthRatio;
		float texHeightRatio;
	};
	
	typedef TypedTexture<Image::default_format> DefaultTexture;
}
