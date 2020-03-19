#define STB_TRUETYPE_IMPLEMENTATION

#define STBTT_malloc(x,u)    malloc(x)
#define STBTT_free(x,u)      free(x)
#define STBTT_assert(...)    do {} while(0)

#include "stb_truetype.h"
