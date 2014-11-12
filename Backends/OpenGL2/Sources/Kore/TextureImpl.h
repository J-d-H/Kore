#pragma once

#include <Kore/Graphics/Image.h>

namespace Kore {
	class Texture;

	class TextureUnitImpl {
	public:
		int unit;
	};
	
	class TextureImpl {
	protected:
		//static TreeMap<Image, Texture*> images;
		TextureImpl(bool convert) : convert(convert) {}

		unsigned int texture;

		bool const convert; // convert to power2?

	public:
		virtual ~TextureImpl();
	};
}
