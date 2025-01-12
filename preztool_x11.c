#include "preztool_x11.h"
#include "preztool.h"
#include <X11/X.h>
#include <X11/Xlib.h>
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

  // reorganize data for rgba format
  // PERF(andrew): simd?
  unsigned char tempColor;
  for (int i = 0; (i + 3) < 4 * img->width * img->height; i += 4) {
    // tempColor = img->data[i];
    // img->data[i] = img->data[i + 2];
    // img->data[i + 2] = tempColor;
    img->data[i + 3] = ~(0L);
  }

  *data = (unsigned char *)img->data;
  *dataWidth = img->width;
  *dataHeight = img->height;
}
