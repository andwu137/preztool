#include "preztool_x11.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // screenshot
  unsigned char *data;
  int dataWidth;
  int dataHeight;
  screenshot(&data, &dataWidth, &dataHeight);

  // start window
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST |
                 FLAG_BORDERLESS_WINDOWED_MODE | FLAG_MSAA_4X_HINT);
  InitWindow(dataWidth, dataHeight, "preztool");
  SetWindowPosition(0, 0);
  SetTargetFPS(60);

  // load screenshot as texture
  Image img = {.width = dataWidth,
               .height = dataHeight,
               .data = data,
               .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
               .mipmaps = 1};
  Texture screenTexture = LoadTextureFromImage(img);
  UnloadImage(img);

  /* prez data */
  // camera
  Camera2D camera = {};
  camera.zoom = 1.0f;
  // draw
  RenderTexture2D drawTarget = LoadRenderTexture(dataWidth, dataHeight);
  Color drawColors[] = {RED, GREEN, BLUE, WHITE, BLACK};
  int drawColorSelected = 0;
  // mouse
  Vector2 mousePos = GetMousePosition();
  Vector2 prevMousePos;

  while (!WindowShouldClose()) {
    prevMousePos = mousePos;
    mousePos = GetMousePosition();
    Vector2 prevMouseWorldPos = GetScreenToWorld2D(prevMousePos, camera);
    Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);

    // translate
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / camera.zoom);
      camera.target = Vector2Add(camera.target, delta);
    }

    // zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      // cursor relation
      camera.offset = mousePos;
      camera.target = mouseWorldPos;

      // zoom
      float scaleFactor = 1.0f + (0.25f * fabsf(wheel));
      if (wheel < 0)
        scaleFactor = 1.0f / scaleFactor;
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
      ClearBackground(WHITE);

      BeginMode2D(camera);
      {
        rlPushMatrix();
        {
          rlTranslatef(0, 25 * 50, 0);
          rlRotatef(90, 1, 0, 0);
        }
        rlPopMatrix();
        DrawTexture(screenTexture, 0, 0, WHITE);
        DrawTextureRec(drawTarget.texture,
                       (Rectangle){0, 0, (float)drawTarget.texture.width,
                                   (float)-drawTarget.texture.height},
                       (Vector2){0, 0}, WHITE);
      }
      EndMode2D();

      DrawFPS(0, 0);
    }
    EndDrawing();
  }
  UnloadTexture(screenTexture);
  UnloadRenderTexture(drawTarget);

  CloseWindow();
  return EXIT_SUCCESS;
}
