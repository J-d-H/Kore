#pragma once

namespace Kore {
	class Image {
	public:
		enum Format {
			RGBA32,
			Grey8
		};
		
		static int sizeOf(Image::Format format);

		Image(int width, int height, Format format, bool readable);
		Image(const char* filename, bool readable);
		virtual ~Image();
		int at(int x, int y);

		int width, height;
		Format format;
		bool readable;
		void* data;
	};
}
