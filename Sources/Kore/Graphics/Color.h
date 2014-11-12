#pragma once

#include "pch.h"

namespace Kore {
	union bgra32;

	union rgba32 {
		u32 c32;
		struct {
#ifdef SYS_LITTLE_ENDIAN
			u8 r;
			u8 g;
			u8 b;
			u8 a;
#else
			u8 a;
			u8 b;
			u8 g;
			u8 r;
#endif
		};
		u8 c8[4];

		rgba32() : c32() {}
		rgba32(u32 color) : c32(color) {}
		rgba32(u8 r, u8 g, u8 b, u8 a = 0xFF) : r(r), g(g), b(b), a(a) {}
		rgba32(rgba32 const & other) : c32(other.c32) {}
		rgba32 const & operator == (rgba32 other) { c32 = other.c32; }

		inline bgra32 toBGRA() const;
	};
	
	union bgra32 {
		u32 c32;
		struct {
#ifdef SYS_LITTLE_ENDIAN
			u8 b;
			u8 g;
			u8 r;
			u8 a;
#else
			u8 a;
			u8 r;
			u8 g;
			u8 b;
#endif
		};
		u8 c8[4];

		bgra32() : c32() {}
		bgra32(u32 color) : c32(color) {}
		bgra32(u8 r, u8 g, u8 b, u8 a = 0xFF) : r(r), g(g), b(b), a(a) {}
		bgra32(bgra32 const & other) : c32(other.c32) {}
		bgra32 const & operator == (bgra32 other) { c32 = other.c32; }

		inline rgba32 toRGBA() const;
	};

	bgra32 rgba32::toBGRA() const { return bgra32(r, g, b, a); }
	rgba32 bgra32::toRGBA() const { return rgba32(r, g, b, a); }

	static_assert(sizeof(bgra32) == sizeof(u32), "ERROR: bgra32-size missmatch!");
	static_assert(sizeof(rgba32) == sizeof(u32), "ERROR: bgra32-size missmatch!");


	///////////////////////////////////////////////////////////////////////////////////
	// Default Colors
	///////////////////////////////////////////////////////////////////////////////////

#ifdef OPENGL
	typedef rgba32 c32;
#else
	typedef bgra32 c32;
#endif

	namespace Colors {
		extern c32 const WHITE;
		extern c32 const BLACK;
		extern c32 const RED;
		extern c32 const GREEN;
		extern c32 const BLUE;
		extern c32 const YELLOW;
	}
}