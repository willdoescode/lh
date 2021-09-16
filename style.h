#ifndef _STYLE_H_
#define _STYLE_H_

// maximum length of an escape code: 1 byte for
// the escape character, 1 for the bracket,
// 3 for 0;0, since that's the longest possible
// sequence after the bracket, and one for the m
#define MAX_COLOR_LEN 6

const static char end[] = "\033[0;0m";
const static char bold[] = "\033[1m";
const static char dull[] = "\033[2m";
const static char italic[] = "\033[3m";
const static char underline[] = "\033[4m";
const static char inverted[] = "\033[7m";
const static char dark_black[] = "\033[30m";
const static char dark_red[] = "\033[31m";
const static char dark_green[] = "\033[32m";
const static char dark_yellow[] = "\033[33m";
const static char dark_blue[] = "\033[34m";
const static char dark_magenta[] = "\033[35m";
const static char dark_cyan[] = "\033[36m";
const static char dark_white[] = "\033[37m";
const static char light_black[] = "\033[90m";
const static char light_red[] = "\033[91m";
const static char light_green[] = "\033[92m";
const static char light_yellow[] = "\033[93m";
const static char light_blue[] = "\033[94m";
const static char light_magenta[] = "\033[95m";
const static char light_cyan[] = "\033[96m";
const static char light_white[] = "\033[97m";

#endif
