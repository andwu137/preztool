#include "preztool_x11.h"
#include "preztool.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

void screenshot(unsigned char **data, int *dataWidth, int *dataHeight) {
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
  for (int i = 0; (i + 8) < 4 * img->width * img->height; i += 4 * 8) {
    __m256i imgData = _mm256_lddqu_si256((const __m256i *)&(img->data[i]));
    _mm256_storeu_si256((__m256i *)&(img->data[i]),
                        _mm256_or_si256(imgData, alphaMask));
  }
#elif defined(__SSE2__)
  __m128i alphaMask =
      _mm_set_epi32(0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000);
  for (int i = 0; (i + 8) < 4 * img->width * img->height; i += 4 * 4) {
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
