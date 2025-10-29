#ifndef PREZTOOL_H
#define PREZTOOL_H

#include <stddef.h>
#include <stdint.h>

#define PROGRAM_NAME "preztool"

void screenshot(unsigned char **data, int *dataWidth, int *dataHeight);

void *make_circular_vmem_buffer(size_t size, unsigned int above, unsigned int below);

#endif // !PREZTOOL_H
