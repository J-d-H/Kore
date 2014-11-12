#pragma once

#include "Color.h"

namespace Kore {

	class Image {
	public:
		enum Format {
			RGBA32,
			Grey8,
			BGRA32
		};

		template <Image::Format F> struct ColorType;
		template<> struct ColorType <Format::Grey8> {
			typedef u8 type;
		};
		template<> struct ColorType <Format::RGBA32> {
			typedef rgba32 type;
		};
		template<> struct ColorType <Format::BGRA32> {
			typedef bgra32 type;
		};

#ifdef OPENGL
		static Format const default_format = Format::RGBA32;
#else
		static Format const default_format = Format::BGRA32;
#endif
		
		static int sizeOf(Image::Format format);
		int sizeOfFormat() const { return sizeOf(format); }
		
		Image(int width, int height, Format format);
		Image(const char* filename, Format format = default_format);
		virtual ~Image();

		//void convert(Format newFormat);

		int at(int x, int y) const;

		int width, height;
		Format format;
		bool readable;
		void* data;
	};


	template <Image::Format F = Image::default_format> class TypedImage {
	public:
		typedef typename Image::ColorType<F>::type color_type;
		typedef typename Image::Format Format;

		static Format const format = F;
		static inline int sizeOfFormat() { return sizeof(color_type); }


		TypedImage(int width, int height) : width(width), height(height), readable(true) { data = new color_type[width*height]; }
		TypedImage(const char* filename) : readable(true) {
			auto tmpImage = Image(filename, format);
			width = tmpImage.width;
			height = tmpImage.height;
			data = (color_type*)tmpImage.data;
			tmpImage.data = nullptr;
		}
		virtual ~TypedImage() {
			delete[] data;
			data = nullptr;
		}

		int width;
		int height;
		bool readable;
		color_type* data;

		//color_type& at(int x, int y) { return data[y * width + x]; }
		color_type  at(int x, int y) const { return data[y * width + x]; }
	};

	typedef TypedImage<Image::default_format> DefaultImage;
}
