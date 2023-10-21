#pragma once

extern "C"
{
#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "Debug.h"
}

#include <span>
#include <string_view>

#include <ft2build.h>
#include FT_FREETYPE_H

struct utfCharSz {
    int unicodeCh;
    int size;
};

utfCharSz utf8ToUnicode(const char* utf8Char);

void Paint_DrawString_UTF8(UWORD Xstart, UWORD Ystart, const char * pString,
                         FT_Face face, UWORD Color_Foreground, UWORD Color_Background);

void Paint_DrawString_UTF8(UWORD Xstart, UWORD Ystart, std::span<std::string_view> lines,
                         FT_Face face, UWORD Color_Foreground, UWORD Color_Background);


