#include "preztool_win32.h"
#include "preztool.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

void screenshot(unsigned char **data, int *width, int *height) {
  HDC hdc = GetDC(0);

  // Get screen dimensions
  *width = GetSystemMetrics(SM_CXSCREEN);
  *height = GetSystemMetrics(SM_CYSCREEN);

  // Create compatible DC, create a compatible bitmap and copy the screen using
  HDC hCaptureDC = CreateCompatibleDC(hdc);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdc, *width, *height);
  HGDIOBJ hOld = SelectObject(hCaptureDC, hBitmap);
  BOOL bOK = BitBlt(hCaptureDC, 0, 0, *width, *height, hdc, 0, 0, SRCCOPY);
  // | CAPTUREBLT

  SelectObject(hCaptureDC, hOld);
  DeleteDC(hCaptureDC);

  // get bitmap
  BITMAPINFO MyBMInfo = {};
  MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

  // Get the BITMAPINFO structure from the bitmap
  if (0 == GetDIBits(hdc, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS)) {
    fprintf(stderr, "%s: unable to get bitmap information\n", PROGRAM_NAME);
    exit(1);
  }

  // create the bitmap buffer
  BYTE *lpPixels = calloc(MyBMInfo.bmiHeader.biSizeImage, sizeof(char));

  // Better do this here - the original bitmap might have BI_BITFILEDS, which
  // makes it necessary to read the color table - you might not want this.
  MyBMInfo.bmiHeader.biCompression = BI_RGB;

  // get the actual bitmap buffer
  if (!GetDIBits(hdc, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, (LPVOID)lpPixels,
                 &MyBMInfo, DIB_RGB_COLORS)) {
    fprintf(stderr, "%s: unable to copy bitmap to buffer\n", PROGRAM_NAME);
    exit(1);
  }

  *data = lpPixels;
  // reorganize data for rgba format
  // PERF(andrew): simd?
  unsigned char tempColor;
  for (int i = 0; (i + 3) < 4 * (*width) * (*height); i += 4) {
    tempColor = (*data)[i];
    (*data)[i] = (*data)[i + 2];
    (*data)[i + 2] = tempColor;
  }

  // free
  DeleteObject(hBitmap);
  ReleaseDC(NULL, hdc);
}
