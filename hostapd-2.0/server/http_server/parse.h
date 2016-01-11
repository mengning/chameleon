#ifndef PARSE_H
#define PARSE_H

#include "http-tree.h"

void *Parse_parse(int fd, int reqOnly);
Body_t *Parse_getBody(int *n);

#endif
