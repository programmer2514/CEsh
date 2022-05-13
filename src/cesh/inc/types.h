#ifndef CESH_TYPES
#define CESH_TYPES

// Used to retain styles for screen buffer
typedef struct __char_styled__ {
    char character : 8;
    bool bold      : 1;
    bool italic    : 1;
    bool underline : 1;
    uint8_t fg_col : 4;
    uint8_t bg_col : 4;
} char_styled_t;

#endif