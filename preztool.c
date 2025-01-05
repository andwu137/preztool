#ifdef _WIN32
#include "preztool_win32.h"
#endif // !_WIN32

#ifdef linux
#include "preztool_x11.h"
#endif // !linux

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>

enum prez_flags {
  FLAGS_MIRROR_X = 1 << 0,
  FLAGS_MIRROR_Y = 1 << 1,
};

int main(int argc, char *argv[]) {
  // screenshot
  unsigned char *data;
  int srcWidth;
  int srcHeight;
  screenshot(&data, &srcWidth, &srcHeight);

  // start window
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST |
                 FLAG_BORDERLESS_WINDOWED_MODE | FLAG_MSAA_4X_HINT);
  InitWindow(srcWidth, srcHeight, "preztool");
  SetWindowPosition(0, 0);
  SetTargetFPS(60);

  // load screenshot as texture
  Image img = {.width = srcWidth,
               .height = srcHeight,
               .data = data,
               .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
               .mipmaps = 1};
  Texture screenTexture = LoadTextureFromImage(img);
  UnloadImage(img);

  /* prez data */
  unsigned char prezFlags = 0;
  // camera
  Camera2D camera = {};
  camera.zoom = 1.0f;
  // draw
  RenderTexture2D drawTarget = LoadRenderTexture(srcWidth, srcHeight);
  Color drawColors[] = {RED, GREEN, BLUE, WHITE, BLACK};
  int drawColorSelected = 0;
  // mouse
  Vector2 mousePos;
  Vector2 prevMousePos;
  Vector2 prevMouseWorldPos;
  Vector2 mouseWorldPos;

  while (!WindowShouldClose()) {
    // swap flags
    if (IsKeyPressed(KEY_A)) {
      prezFlags ^= FLAGS_MIRROR_X;

      // for prev
      mousePos.x = srcWidth - mousePos.x;
      mouseWorldPos.x = srcWidth - mouseWorldPos.x;
    }
    if (IsKeyPressed(KEY_S)) {
      prezFlags ^= FLAGS_MIRROR_Y;

      // for prev
      mousePos.y = srcHeight - mousePos.y;
      mouseWorldPos.y = srcHeight - mouseWorldPos.y;
    }

    // mouse init
    Vector2 flipVector = Vector2One();
    prevMousePos = mousePos;
    mousePos = GetMousePosition();
    prevMouseWorldPos = mouseWorldPos;
    mouseWorldPos = GetScreenToWorld2D(mousePos, camera);

    Vector2 delta = GetMouseDelta();
    float wheel = GetMouseWheelMove();

    // flip
    if (prezFlags & FLAGS_MIRROR_X) {
      mousePos.x = srcWidth - mousePos.x;
      mouseWorldPos.x = srcWidth - mouseWorldPos.x;
      flipVector.x = -1;
    }
    if (prezFlags & FLAGS_MIRROR_Y) {
      mousePos.y = srcHeight - mousePos.y;
      mouseWorldPos.y = srcHeight - mouseWorldPos.y;
      flipVector.y = -1;
    }
    Vector2Multiply(delta, flipVector);

    // translate
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      delta = Vector2Scale(delta, -1.0f / camera.zoom);
      camera.target = Vector2Add(camera.target, delta);
    }

    // zoom
    if (wheel != 0) {
      // cursor relation
      Vector2 mousePos = GetMousePosition();
      Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);
      camera.offset = mousePos;
      camera.target = mouseWorldPos;

      // set
      float scaleFactor = 1.0f + (0.25f * fabsf(wheel));
      if (wheel < 0) {
        scaleFactor = 1.0f / scaleFactor;
      }
      camera.zoom = Clamp(camera.zoom * scaleFactor, 0.125f, 64.0f);
    }

    // change draw color
    if (IsKeyPressed(KEY_RIGHT)) {
      drawColorSelected++;
      if (drawColorSelected >= sizeof(drawColors) / sizeof(*drawColors)) {
        drawColorSelected = 0;
      }
    } else if (IsKeyPressed(KEY_LEFT)) {
      drawColorSelected--;
      if (drawColorSelected < 0) {
        drawColorSelected = sizeof(drawColors) / sizeof(*drawColors) - 1;
      }
    }

    // clear draw
    if (IsKeyPressed(KEY_C)) {
      BeginTextureMode(drawTarget);
      ClearBackground(BLANK);
      EndTextureMode();
    }

    // draw
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      BeginTextureMode(drawTarget);
      DrawCircleV(mouseWorldPos, 5, drawColors[drawColorSelected]);
      DrawLineEx(prevMouseWorldPos, mouseWorldPos, 10,
                 drawColors[drawColorSelected]);
      EndTextureMode();
    }

    // render
    BeginDrawing();
    {
      ClearBackground(BLACK);

      BeginMode2D(camera);
      {
        DrawTextureRec(screenTexture,
                       (Rectangle){0, 0, srcWidth * flipVector.x,
                                   srcHeight * flipVector.y},
                       (Vector2){0, 0}, WHITE);
        DrawTextureRec(drawTarget.texture,
                       (Rectangle){0, 0,
                                   drawTarget.texture.width * flipVector.x,
                                   -drawTarget.texture.height * flipVector.y},
                       (Vector2){0, 0}, WHITE);
      }
      EndMode2D();
    }
    EndDrawing();
  }
  UnloadTexture(screenTexture);
  UnloadRenderTexture(drawTarget);

  CloseWindow();
  return EXIT_SUCCESS;
}
