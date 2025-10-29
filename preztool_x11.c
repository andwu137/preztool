#ifndef PREZTOOL_X11_C
#define PREZTOOL_X11_C

#define _GNU_SOURCE

#include "preztool.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <immintrin.h>
#include <unistd.h>

#include <sys/mman.h>

void screenshot(
    unsigned char **data,
    int *dataWidth,
    int *dataHeight) {
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "%s:  unable to open display\n", PROGRAM_NAME);
    exit(1);
  }

  int window = XDefaultRootWindow(display);
  XWindowAttributes windowAttr;
  XGetWindowAttributes(display, window, &windowAttr);
  XImage *img = XGetImage(display, window, 0, 0, windowAttr.width,
                          windowAttr.height, AllPlanes, ZPixmap);
  if (img == NULL) {
    fprintf(stderr, "%s:  unable to read screen\n", PROGRAM_NAME);
    exit(1);
  }
  XCloseDisplay(display);

  // set all alpha to full
#if defined(__AVX__)
  __m256i alphaMask =
      _mm256_set_epi32(0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                       0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000);
  for (int i = 0; (i + (4 * 8)) < 4 * img->width * img->height; i += 4 * 8) {
    __m256i imgData = _mm256_lddqu_si256((const __m256i *)&(img->data[i]));
    _mm256_storeu_si256((__m256i *)&(img->data[i]),
                        _mm256_or_si256(imgData, alphaMask));
  }
#elif defined(__SSE2__)
  __m128i alphaMask =
      _mm_set_epi32(0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000);
  for (int i = 0; (i + (4 * 4)) < 4 * img->width * img->height; i += 4 * 4) {
    __m128i imgData = _mm_lddqu_si128((const __m128i *)&(img->data[i]));
    _mm_storeu_si128((__m128i *)&(img->data[i]),
                     _mm_or_si128(imgData, alphaMask));
  }
#else
  for (int i = 0; (i + 3) < 4 * img->width * img->height; i += 4) {
    img->data[i + 3] = ~(0L);
  }
#endif

  *data = (unsigned char *)img->data;
  *dataWidth = img->width;
  *dataHeight = img->height;
}

void *make_circular_vmem_buffer(
    size_t size,
    unsigned int above,
    unsigned int below) {
  if (size % getpagesize() != 0) {
    return NULL;
  }

  int fd = memfd_create("circular_vmem_buffer", 0);
  ftruncate(fd, size);

  uint8_t *vmem = mmap(NULL, (above + below + 1) * size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (vmem == NULL) { perror("failed to allocate vmem circular buffer"); abort(); }

  for (size_t i = 0;
      i < above + 1 + below;
      i++) {
    void *temp = mmap(vmem + (size * i), size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0);
    if (temp == NULL) { perror("failed to allocate vmem circular buffer"); abort(); }
  }

  return vmem + (above * size);
}

#endif // PREZTOOL_X11_C
