# ----------------------------
# Program Options
# ----------------------------

NAME         = CESH
ICON         = icon.png
DESCRIPTION  = "CEsh - The TI-84 Plus CE Shell"
COMPRESSED   = YES
ARCHIVED     = YES

CFLAGS    = -Wall -Wextra -Oz
CXXFLAGS  = -Wall -Wextra -Oz

FONTDIR  = $(SRCDIR)/fonts

FONT          = $(FONTDIR)/terminusfont.fnt
BOLDFONT      = $(FONTDIR)/terminusfontbold.fnt
ITALFONT      = $(FONTDIR)/terminusfontitalic.fnt
BOLDITALFONT  = $(FONTDIR)/terminusfontbolditalic.fnt

FONT_INC          = $(FONTDIR)/terminusfont.inc
BOLDFONT_INC      = $(FONTDIR)/terminusfontbold.inc
ITALFONT_INC      = $(FONTDIR)/terminusfontitalic.inc
BOLDITALFONT_INC  = $(FONTDIR)/terminusfontbolditalic.inc

DEPS  = $(FONT_INC) $(BOLDFONT_INC) $(ITALFONT_INC) $(BOLDITALFONT_INC)

# ----------------------------

include $(shell cedev-config --makefile)

# ----------------------------

$(FONT_INC): $(FONT)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -c 2 -x 9 -l 0x0B -Z $@

$(BOLDFONT_INC): $(BOLDFONT)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -c 2 -x 9 -l 0x0B -Z $@

$(ITALFONT_INC): $(ITALFONT)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -c 2 -x 9 -l 0x0B -Z $@

$(BOLDITALFONT_INC): $(BOLDITALFONT)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -c 2 -x 9 -l 0x0B -Z $@