#include <ft2build.h>
#include <iostream>
#include <stdexcept>
#include FT_FREETYPE_H

struct utfCharSz {
  int unicodeCh;
  int size;
};

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

int main() {
  // Initialize FreeType library
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    std::cerr << "Error: Could not initialize FreeType library" << std::endl;
    return 1;
  }

  // Load the font file
  FT_Face face;
  if (FT_New_Face(ft, "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
                  0, &face)) {
    std::cerr << "Error: Could not load the font file" << std::endl;
    return 1;
  }

  // Set character size
  FT_Set_Pixel_Sizes(face, 0, 16);

  const char *text =
      "P. VERGILI MARONIS AENEIDOS LIBER PRIMVS\n"
      "Arma virumque canō, Trōiae quī prīmus ab ōrīs\n"
      "Ītaliam, fātō profugus, Lāvīniaque vēnit\n"
      "lītora, multum ille et terrīs iactātus et altō\n"
      "vī superum saevae memorem Iūnōnis ob īram;\n"
      "multa quoque et bellō passus, dum conderet urbem,               5\n"
      "inferretque deōs Latiō, genus unde Latīnum,\n"
      "Albānīque patrēs, atque altae moenia Rōmae.\n\n"
      "Mūsa, mihī causās memorā, quō nūmine laesō,\n"
      "quidve dolēns, rēgīna deum tot volvere cāsūs\n"
      "īnsīgnem pietāte virum, tot adīre labōrēs                               "
      "    10\n"
      "impulerit. Tantaene animīs caelestibus īrae?\n\n"
      "Urbs antīqua fuit, Tyriī tenuēre colōnī,\n"
      "Karthāgō, Ītaliam contrā Tiberīnaque longē\n"
      "ōstia, dīves opum studiīsque asperrima bellī,\n"
      "quam Iūnō fertur terrīs magis omnibus ūnam                           "
      "15\n"
      "posthabitā coluisse Samō; hīc illius arma,\n"
      "hīc currus fuit; hōc rēgnum dea gentibus esse,\n"
      "sī quā Fāta sinant, iam tum tenditque fovetque.\n"
      "Prōgeniem sed enim Trōiānō ā sanguine dūcī\n"
      "audierat, Tyriās olim quae verteret arcēs;                              "
      "     20\n"
      "hinc populum lātē regem bellōque superbum\n"
      "ventūrum excidiō Libyae: sīc volvere Parcās.\n"
      "Id metuēns, veterisque memor Sāturnia bellī,\n"
      "prīma quod ad Trōiam prō cārīs gesserat Argīs—\n"
      "necdum etiam causae īrārum saevīque dolōrēs                          "
      "25\n"
      "exciderant animō: manet altā mente repostum\n"
      "iūdicium Paridis sprētaeque iniūria fōrmae,\n"
      "et genus invīsum, et raptī Ganymēdis honōrēs.\n"
      "Hīs accēnsa super, iactātōs aequore tōtō\n"
      "Trōas, rēliquiās Danaum atque immītis Achillī,                          "
      "30\n"
      "arcēbat longē Latiō, multōsque per annōs\n"
      "errābant, āctī Fātīs, maria omnia circum.\n"
      "Tantae mōlis erat Rōmānam condere gentem!";

  while (*text) {

    // Load a UTF-8 character (e.g., 'A' in UTF-8 is represented by 0x41)
    auto charcode = utf8ToUnicode(text);

    if (charcode.size == 0) {
        throw std::runtime_error("");
    }

    text += charcode.size;
    if (FT_Load_Char(face, charcode.unicodeCh, FT_LOAD_RENDER)) {
      std::cerr << "Error: Could not load glyph" << std::endl;
      return 1;
    }

    

    // Access pixel data of the glyph
    FT_GlyphSlot g = face->glyph;

    std::cout << face->height << std::endl;
    std::cout << face->max_advance_height << std::endl;

   /* const auto &metrics = g->metrics;

        // Print FT_Glyph_Metrics information
    std::cout << "FT_Glyph_Metrics Information:\n"
              << "width: " << metrics.width << "\n"
              << "height: " << metrics.height << "\n"
              << "horiBearingX: " << metrics.horiBearingX << "\n"
              << "horiBearingY: " << metrics.horiBearingY << "\n"
              << "horiAdvance: " << metrics.horiAdvance << "\n"
              << "vertBearingX: " << metrics.vertBearingX << "\n"
              << "vertBearingY: " << metrics.vertBearingY << "\n"
              << "vertAdvance: " << metrics.vertAdvance << "\n"
              << "linear bitmap_top " << g->bitmap_top << "\n"
              << "linear bitmap_left " << g->bitmap_left << "\n"
              << std::endl;


    for (int y = 0; y < g->bitmap.rows; ++y) {
      for (int x = 0; x < g->bitmap.width; ++x) {
        // Access pixel value (g->bitmap.buffer is a pointer to the pixel data)
        unsigned char pixel = g->bitmap.buffer[y * g->bitmap.width + x];
        std::cout << (pixel > 0 ? "#" : " ");
      }
      std::cout << std::endl;
    }*/
  }

    // Cleanup
    FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return 0;
}
