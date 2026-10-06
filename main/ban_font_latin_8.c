/*******************************************************************************
 * Size: 8 px
 * Bpp: 1
 * Opts: --font assets/fonts/PressStart2P-Regular.ttf --symbols  $%+-.0123456789?ABCDEFGHIKLMNOPRSTUVWYZabcdefghilmnoprstuyz --size 8 --bpp 1 --format lvgl --no-compress --lv-include lvgl.h --lv-font-name ban_font_latin_8 --output main/ban_font_latin_8.c --autohint-off --no-kerning
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef BAN_FONT_LATIN_8
#define BAN_FONT_LATIN_8 1
#endif

#if BAN_FONT_LATIN_8

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0024 "$" */
    0x10, 0xfb, 0x43, 0xe1, 0x7f, 0x84, 0x0,

    /* U+0025 "%" */
    0x63, 0x4b, 0x20, 0x82, 0x69, 0x63, 0x0,

    /* U+002B "+" */
    0x30, 0xcf, 0xcc, 0x30,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xf0,

    /* U+0030 "0" */
    0x38, 0x9b, 0x1e, 0x3c, 0x6c, 0x8e, 0x0,

    /* U+0031 "1" */
    0x31, 0xc3, 0xc, 0x30, 0xcf, 0xc0,

    /* U+0032 "2" */
    0x7d, 0x8c, 0x39, 0xe7, 0x9c, 0x3f, 0x80,

    /* U+0033 "3" */
    0x7e, 0x18, 0x61, 0xe0, 0x78, 0xdf, 0x0,

    /* U+0034 "4" */
    0x1c, 0x79, 0xb6, 0x6f, 0xe1, 0x83, 0x0,

    /* U+0035 "5" */
    0xfd, 0x83, 0xf0, 0x30, 0x78, 0xdf, 0x0,

    /* U+0036 "6" */
    0x3c, 0xc3, 0x7, 0xec, 0x78, 0xdf, 0x0,

    /* U+0037 "7" */
    0xff, 0x8c, 0x30, 0xc3, 0x6, 0xc, 0x0,

    /* U+0038 "8" */
    0x79, 0x8b, 0x93, 0xc9, 0xf0, 0xdf, 0x0,

    /* U+0039 "9" */
    0x7d, 0x8f, 0x1b, 0xf0, 0x61, 0x9e, 0x0,

    /* U+003F "?" */
    0x7d, 0xff, 0x18, 0x63, 0x80, 0xe, 0x0,

    /* U+0041 "A" */
    0x38, 0xdb, 0x1e, 0x3f, 0xf8, 0xf1, 0x80,

    /* U+0042 "B" */
    0xfd, 0x8f, 0x1f, 0xec, 0x78, 0xff, 0x0,

    /* U+0043 "C" */
    0x3c, 0xcf, 0x6, 0xc, 0xc, 0xcf, 0x0,

    /* U+0044 "D" */
    0xf9, 0x9b, 0x1e, 0x3c, 0x79, 0xbe, 0x0,

    /* U+0045 "E" */
    0xff, 0x83, 0x7, 0xec, 0x18, 0x3f, 0x80,

    /* U+0046 "F" */
    0xff, 0x83, 0x7, 0xec, 0x18, 0x30, 0x0,

    /* U+0047 "G" */
    0x3e, 0xc3, 0x6, 0x7c, 0x6c, 0xcf, 0x80,

    /* U+0048 "H" */
    0xc7, 0x8f, 0x1f, 0xfc, 0x78, 0xf1, 0x80,

    /* U+0049 "I" */
    0xfc, 0xc3, 0xc, 0x30, 0xcf, 0xc0,

    /* U+004B "K" */
    0xc7, 0x9b, 0x67, 0x8f, 0x9b, 0xb3, 0x80,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xf, 0xc0,

    /* U+004D "M" */
    0xc7, 0xdf, 0xfe, 0xbd, 0x78, 0xf1, 0x80,

    /* U+004E "N" */
    0xc7, 0xcf, 0xde, 0xfc, 0xf8, 0xf1, 0x80,

    /* U+004F "O" */
    0x7d, 0x8f, 0x1e, 0x3c, 0x78, 0xdf, 0x0,

    /* U+0050 "P" */
    0xfd, 0x8f, 0x1e, 0x3f, 0xd8, 0x30, 0x0,

    /* U+0052 "R" */
    0xfd, 0x8f, 0x1e, 0x7f, 0x9b, 0xb3, 0x80,

    /* U+0053 "S" */
    0x7d, 0x8f, 0x3, 0xe0, 0x78, 0xdf, 0x0,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0x0,

    /* U+0055 "U" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xdf, 0x0,

    /* U+0056 "V" */
    0xc7, 0x8f, 0x1f, 0x77, 0xc7, 0x4, 0x0,

    /* U+0057 "W" */
    0xd7, 0xaf, 0x5e, 0xbf, 0xfd, 0xd1, 0x0,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xde, 0x30, 0xc3, 0x0,

    /* U+005A "Z" */
    0xfe, 0x1c, 0x71, 0xc7, 0x1c, 0x3f, 0x80,

    /* U+0061 "a" */
    0x7c, 0xd, 0xfe, 0x37, 0xe0,

    /* U+0062 "b" */
    0xc1, 0x83, 0xf6, 0x3c, 0x78, 0xdf, 0x0,

    /* U+0063 "c" */
    0x7f, 0x83, 0x6, 0x7, 0xe0,

    /* U+0064 "d" */
    0x6, 0xd, 0xfe, 0x3c, 0x78, 0xdf, 0x80,

    /* U+0065 "e" */
    0x7d, 0x8f, 0xfe, 0x7, 0xc0,

    /* U+0066 "f" */
    0x1c, 0xcf, 0xcc, 0x30, 0xc3, 0x0,

    /* U+0067 "g" */
    0x7f, 0x8f, 0x1b, 0xf0, 0x6f, 0x80,

    /* U+0068 "h" */
    0xc1, 0x83, 0xf6, 0x3c, 0x78, 0xf1, 0x80,

    /* U+0069 "i" */
    0x30, 0x7, 0xc, 0x30, 0xcf, 0xc0,

    /* U+006C "l" */
    0x70, 0xc3, 0xc, 0x30, 0xcf, 0xc0,

    /* U+006D "m" */
    0xfd, 0x6e, 0xdd, 0xbb, 0x60,

    /* U+006E "n" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x60,

    /* U+006F "o" */
    0x7d, 0x8f, 0x1e, 0x37, 0xc0,

    /* U+0070 "p" */
    0xfd, 0x8f, 0x1f, 0xec, 0x18, 0x0,

    /* U+0072 "r" */
    0xdf, 0x8c, 0x30, 0xc0,

    /* U+0073 "s" */
    0x7d, 0x81, 0xf0, 0x3f, 0xc0,

    /* U+0074 "t" */
    0x30, 0xcf, 0xcc, 0x30, 0xc3, 0x0,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x37, 0xe0,

    /* U+0079 "y" */
    0xc7, 0x8f, 0x1b, 0xf0, 0x6f, 0x80,

    /* U+007A "z" */
    0xfe, 0x38, 0xe3, 0x8f, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 8, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 15, .adv_w = 128, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 19, .adv_w = 128, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 20, .adv_w = 128, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 21, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 28, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 34, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 41, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 48, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 55, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 62, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 69, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 76, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 83, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 90, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 97, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 104, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 111, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 118, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 125, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 132, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 139, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 146, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 153, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 159, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 166, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 172, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 179, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 186, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 193, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 200, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 207, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 214, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 220, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 227, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 234, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 241, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 247, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 254, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 259, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 266, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 271, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 278, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 283, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 289, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 302, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 308, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 314, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 319, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 324, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 329, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 128, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 339, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 344, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 350, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 355, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 0, 0, 0, 1, 2, 0, 0,
    0, 0, 0, 3, 0, 4, 5, 0,
    6, 7, 8, 9, 10, 11, 12, 13,
    14, 15, 0, 0, 0, 0, 0, 16,
    0, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 0, 26, 27, 28, 29, 30,
    31, 0, 32, 33, 34, 35, 36, 37,
    0, 38, 39, 0, 0, 0, 0, 0,
    0, 40, 41, 42, 43, 44, 45, 46,
    47, 48, 0, 0, 49, 50, 51, 52,
    53, 0, 54, 55, 56, 57, 0, 0,
    0, 58, 59
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 91, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 91, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ban_font_latin_8 = {
#else
lv_font_t ban_font_latin_8 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 8,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if BAN_FONT_LATIN_8*/

