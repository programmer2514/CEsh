#include "fonts.h"

/* This contains the raw data for the font. */
static const uint8_t terminus_font_data[] = {
	#include "terminusfont.inc"
};
static const uint8_t terminus_font_bold_data[] = {
	#include "terminusfontbold.inc"
};
static const uint8_t terminus_font_italic_data[] = {
	#include "terminusfontitalic.inc"
};
static const uint8_t terminus_font_bold_italic_data[] = {
	#include "terminusfontbolditalic.inc"
};

/* However, C89 does not allow us to typecast a byte array into a
fontlib_font_t pointer directly, so we have to use a second statement to do it,
though helpfully we can at least do it in the global scope. */
const fontlib_font_t *terminus_font = (fontlib_font_t *)terminus_font_data;
const fontlib_font_t *terminus_font_bold = (fontlib_font_t *)terminus_font_bold_data;
const fontlib_font_t *terminus_font_italic = (fontlib_font_t *)terminus_font_italic_data;
const fontlib_font_t *terminus_font_bold_italic = (fontlib_font_t *)terminus_font_bold_italic_data;
