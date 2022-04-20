# ----------------------------
# Program Options
# ----------------------------

NAME         ?= CESH
ICON         ?= icon.png
DESCRIPTION  ?= "CEsh - The TI-84 Plus CE Shell"
COMPRESSED   ?= YES
ARCHIVED     ?= YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

FONTDIR ?= $(SRCDIR)/fonts

include $(shell cedev-config --makefile)

# This is a roundabout way to tell make that fonts.c depends on testfont.inc.
# It does it by saying the compiled object code depends on the .inc file.
$(OBJDIR)/fonts/fonts.src: $(FONTDIR)/terminusfont.inc
$(OBJDIR)/fonts/fonts.src: $(FONTDIR)/terminusfontbold.inc
$(OBJDIR)/fonts/fonts.src: $(FONTDIR)/terminusfontitalic.inc
$(OBJDIR)/fonts/fonts.src: $(FONTDIR)/terminusfontbolditalic.inc

$(FONTDIR)/terminusfont.inc: $(FONTDIR)/terminusfont.fnt
	convfont -o carray -f $(FONTDIR)/terminusfont.fnt $(FONTDIR)/terminusfont.inc

$(FONTDIR)/terminusfontbold.inc: $(FONTDIR)/terminusfontbold.fnt
	convfont -o carray -f $(FONTDIR)/terminusfontbold.fnt $(FONTDIR)/terminusfontbold.inc

$(FONTDIR)/terminusfontitalic.inc: $(FONTDIR)/terminusfontitalic.fnt
	convfont -o carray -f $(FONTDIR)/terminusfontitalic.fnt $(FONTDIR)/terminusfontitalic.inc

$(FONTDIR)/terminusfontbolditalic.inc: $(FONTDIR)/terminusfontbolditalic.fnt
	convfont -o carray -f $(FONTDIR)/terminusfontbolditalic.fnt $(FONTDIR)/terminusfontbolditalic.inc
