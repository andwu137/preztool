#include "preztool_win32.h"
#include "preztool.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

void screenshot(unsigned char **data, int *dataWidth, int *dataHeight) {
  HDC hDc = CreateCompatibleDC(NULL);
  if (!hDc) {
    fprintf(stderr, "%s:  unable to open display context\n", PROGRAM_NAME);
    exit(1);
  }

  HDC hScrn = GetDC(NULL);
  *dataWidth = GetSystemMetrics(SM_CXSCREEN);
  *dataHeight = GetSystemMetrics(SM_CYSCREEN);
  HBITMAP hBmp = CreateCompatibleBitmap(hScrn, w, h);
  if (!hBmp) {
    fprintf(stderr, "%s:  unable to initialize bitmap\n", PROGRAM_NAME);
    exit(1);
  }

  SelectObject(hDc, hBmp);
  BitBlt(hDc, 0, 0, w, h, hScrn, 0, 0, SRCCOPY);

  // TODO(andrew): assign data

  // DeleteObject(hBmp);
  // DeleteDC(hDc);
  // ReleaseDC(NULL, hScrn);
}
