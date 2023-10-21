

#include "GUI_Paint_pp.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string_view>

// Function to convert a UTF-8 character to Unicode code point
utfCharSz utf8ToUnicode(const char *utf8Char) {
  unsigned char c = utf8Char[0];

  if (c < 0x80) {
    // ASCII character
    return {c, 1};
  } else if ((c & 0xE0) == 0xC0) {
    // 2-byte sequence
    return {((c & 0x1F) << 6) | (utf8Char[1] & 0x3F), 2};
  } else if ((c & 0xF0) == 0xE0) {
    // 3-byte sequence
    return {((c & 0x0F) << 12) | ((utf8Char[1] & 0x3F) << 6) |
                (utf8Char[2] & 0x3F),
            3};
  } else if ((c & 0xF8) == 0xF0) {
    // 4-byte sequence
    return {((c & 0x07) << 18) | ((utf8Char[1] & 0x3F) << 12) |
                ((utf8Char[2] & 0x3F) << 6) | (utf8Char[3] & 0x3F),
            4};
  } else {
    // Invalid UTF-8 sequence
    return {-1, 0};
  }
}

void Paint_DrawChar(UWORD Xpoint, UWORD Ypoint, FT_GlyphSlot glyph,
                    UWORD Color_Foreground, UWORD Color_Background) {
  UWORD Page, Column;

  if (Xpoint > Paint.Width || Ypoint > Paint.Height) {
    Debug("Paint_DrawChar Input exceeds the normal display range\r\n");
    return;
  }

  const unsigned char *ptr = glyph->bitmap.buffer;
  const UWORD Ys = Ypoint - glyph->bitmap_top;

  for (Page = 0; Page < glyph->bitmap.rows; Page++) {
    for (Column = 0; Column < glyph->bitmap.width; Column++) {

      // To determine whether the font background color and screen background
      // color is consistent
      if (FONT_BACKGROUND ==
          Color_Background) { // this process is to speed up the scan
        if (*ptr)
          Paint_SetPixel(Xpoint + Column, Ys + Page, Color_Foreground);
        // Paint_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground,
        // DOT_PIXEL_DFT, DOT_STYLE_DFT);
      } else {
        if (*ptr) {
          Paint_SetPixel(Xpoint + Column, Ys + Page, Color_Foreground);
          // Paint_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground,
          // DOT_PIXEL_DFT, DOT_STYLE_DFT);
        } else {
          Paint_SetPixel(Xpoint + Column, Ys + Page, Color_Background);
          // Paint_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Background,
          // DOT_PIXEL_DFT, DOT_STYLE_DFT);
        }
      }

      ++ptr;
    } // Write a line
  }   // Write all
}

void Paint_DrawString_UTF8(UWORD Xstart, UWORD Ystart, const char *pString,
                           FT_Face face, UWORD Color_Foreground,
                           UWORD Color_Background) {
  UWORD Xpoint = Xstart;
  UWORD Ypoint = Ystart;

  if (Xstart > Paint.Width || Ystart > Paint.Height) {
    Debug("Paint_DrawString_EN Input exceeds the normal display range\r\n");
    return;
  }

  unsigned int lastrows = 0;
  unsigned int lastcols = 0;

  while (*pString != '\0') {
    if (*pString == '\n') {
      Xpoint = Xstart;
      Ypoint += lastrows + 1;
      ++pString;
      continue;
    }

    utfCharSz charcode = utf8ToUnicode(pString);

    assert(charcode.size > 0);

    if (isspace(charcode.unicodeCh)) {
      Xpoint += (lastcols * 2) / 3;
      ++pString;
      continue;
    }

    if (FT_Load_Char(face, charcode.unicodeCh, FT_LOAD_RENDER)) {
      std::cerr << "Error: Could not load glyph" << std::endl;
      continue;
    }

    FT_GlyphSlot g = face->glyph;

    lastrows = std::max(g->bitmap.rows, lastrows);
    lastcols = std::max(g->bitmap.width, lastcols);

    // if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y
    // direction plus the Height of the character
    if ((Xpoint + g->bitmap.width) > Paint.Width) {
      Xpoint = Xstart;
      Ypoint += lastrows + 1;
    }

    // If the Y direction is full, reposition to(Xstart, Ystart)
    if ((Ypoint + g->bitmap.rows) > Paint.Height) {
      // Xpoint = Xstart;
      // Ypoint = Ystart;
      break;
    }

    /*for (int y = 0; y < g->bitmap.rows; ++y) {
        for (int x = 0; x < g->bitmap.width; ++x) {
            // Access pixel value (g->bitmap.buffer is a pointer to the pixel
    data) unsigned char pixel = g->bitmap.buffer[y * g->bitmap.width + x];
            std::cout << (pixel > 0 ? "#" : " ");
        }
        std::cout << std::endl;
    }*/
    Paint_DrawChar(Xpoint, Ypoint, g, Color_Background, Color_Foreground);

    Xpoint += 1;

    // The next character of the address
    pString += charcode.size;

    // The next word of the abscissa increases the font of the broadband
    Xpoint += g->bitmap.width;
  }
}

void Paint_DrawString_UTF8(UWORD Xstart, UWORD Ystart,
                           std::span<std::string_view> lines, FT_Face face,
                           UWORD Color_Foreground, UWORD Color_Background) {
  UWORD Xpoint = Xstart;
  UWORD Ypoint = Ystart;

  if (Xstart > Paint.Width || Ystart > Paint.Height) {
    Debug("Paint_DrawString_EN Input exceeds the normal display range\r\n");
    return;
  }

  unsigned int lastrows = 0;
  unsigned int lastcols = 0;

  for (const auto &view : lines) {
    if (view.empty()) {
      Xpoint = Xstart;
      Ypoint += lastrows + 1;
      continue;
    }

    for (auto pString = view.begin(); pString < view.end(); /**/) {
      if (*pString == '\n') {
        Xpoint = Xstart;
        Ypoint += lastrows + 1;
        ++pString;
        continue;
      }

      utfCharSz charcode = utf8ToUnicode(pString);

      assert(charcode.size > 0);

      if (isspace(charcode.unicodeCh)) {
        Xpoint += (lastcols * 2) / 3;
        ++pString;
        continue;
      }

      if (FT_Load_Char(face, charcode.unicodeCh, FT_LOAD_RENDER)) {
        std::cerr << "Error: Could not load glyph" << std::endl;
        continue;
      }

      FT_GlyphSlot g = face->glyph;

      lastrows = std::max(g->bitmap.rows, lastrows);
      lastcols = std::max(g->bitmap.width, lastcols);

      // if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y
      // direction plus the Height of the character
      if ((Xpoint + g->bitmap.width) > Paint.Width) {
        Xpoint = Xstart;
        Ypoint += lastrows + 1;
      }

      // If the Y direction is full, reposition to(Xstart, Ystart)
      if ((Ypoint + g->bitmap.rows) > Paint.Height) {
        // Xpoint = Xstart;
        // Ypoint = Ystart;
        break;
      }

      /*for (int y = 0; y < g->bitmap.rows; ++y) {
          for (int x = 0; x < g->bitmap.width; ++x) {
              // Access pixel value (g->bitmap.buffer is a pointer to the pixel
      data) unsigned char pixel = g->bitmap.buffer[y * g->bitmap.width + x];
              std::cout << (pixel > 0 ? "#" : " ");
          }
          std::cout << std::endl;
      }*/
      Paint_DrawChar(Xpoint, Ypoint, g, Color_Background, Color_Foreground);

      Xpoint += 1;

      // The next character of the address
      pString += charcode.size;

      // The next word of the abscissa increases the font of the broadband
      Xpoint += g->bitmap.width;
    }

    Xpoint = Xstart;
    Ypoint += lastrows + 1;
  }
}
