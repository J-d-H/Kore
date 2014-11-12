#pragma once

#include <Kore\Graphics\Image.h>

struct IDirect3DTexture9;

namespace Kore {
	class TextureUnitImpl {
	public:
		int unit;
	};

	class TextureImpl {
	public:
		IDirect3DTexture9* texture;
		virtual ~TextureImpl();

		void unmipmap();
		void unset();
		int stage;
		bool mipmap;
		//u8 pixfmt;

	protected:
		void construct(int width, int height, Image::Format format, void const * data);
	};
}
