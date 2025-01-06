#include "preztool_x11.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>

enum prez_flags {
  FLAGS_MIRROR_X = 1 << 0,
  FLAGS_MIRROR_Y = 1 << 1,
  FLAGS_FLASHLIGHT = 1 << 2,
};

struct flashlight {
  Vector2 pos;
  float inner;
  float outer;

  unsigned int posLoc;
  unsigned int innerLoc;
  unsigned int outerLoc;
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
  // flashlight
  Shader shdrFlashlight =
      LoadShader(NULL, "assets/shaders/flashlight.frag.glsl");
  struct flashlight flashlight = {
      .pos = {0, 0},
      .posLoc = GetShaderLocation(shdrFlashlight, "flashlight.pos"),
      .inner = (float)srcWidth / 15,
      .innerLoc = GetShaderLocation(shdrFlashlight, "flashlight.inner"),
      .outer = (float)srcWidth / 15 * 1.5,
      .outerLoc = GetShaderLocation(shdrFlashlight, "flashlight.outer"),
  };
  SetShaderValue(shdrFlashlight, flashlight.innerLoc, &flashlight.inner,
                 SHADER_UNIFORM_FLOAT);
  SetShaderValue(shdrFlashlight, flashlight.outerLoc, &flashlight.outer,
                 SHADER_UNIFORM_FLOAT);

  // camera
  Camera2D camera = {};
  camera.zoom = 1.0f;

  // draw
  RenderTexture2D drawTarget = LoadRenderTexture(srcWidth, srcHeight);
  Color drawColors[] = {RED, GREEN, BLUE, WHITE, BLACK};
  int drawColorSelected = 0;

  // mouse
  Vector2 mousePos;
  Vector2 prevMouseWorldPos;
  Vector2 mouseWorldPos;

  while (!WindowShouldClose()) {
    // swap flags
    if (IsKeyPressed(KEY_X)) {
      prezFlags ^= FLAGS_MIRROR_X;

      // for prev
      mousePos.x = srcWidth - mousePos.x;
      mouseWorldPos.x = srcWidth - mouseWorldPos.x;
    }
    if (IsKeyPressed(KEY_V)) {
      prezFlags ^= FLAGS_MIRROR_Y;

      // for prev
      mousePos.y = srcHeight - mousePos.y;
      mouseWorldPos.y = srcHeight - mouseWorldPos.y;
    }
    if (IsKeyPressed(KEY_F)) {
      prezFlags ^= FLAGS_FLASHLIGHT;
    }

    // mouse init
    Vector2 flipVector = Vector2One();
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

    if (wheel != 0) {
      if (IsKeyDown(KEY_S)) { // flashlight size
        float scaleFactor = 1.15;
        if (wheel > 0) {
          scaleFactor = 1.0f / scaleFactor;
        }
        flashlight.inner *= scaleFactor;
        flashlight.outer = flashlight.inner * 1.5;
        SetShaderValue(shdrFlashlight, flashlight.innerLoc, &flashlight.inner,
                       SHADER_UNIFORM_FLOAT);
        SetShaderValue(shdrFlashlight, flashlight.outerLoc, &flashlight.outer,
                       SHADER_UNIFORM_FLOAT);
      } else { // zoom
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
    }

    // change draw color
    if (IsKeyPressed(KEY_RIGHT)) {
      drawColorSelected++;
      if (drawColorSelected >=
          (int)(sizeof(drawColors) / sizeof(*drawColors))) {
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

    // flashlight
    flashlight.pos = GetMousePosition();
    flashlight.pos.y = srcHeight - flashlight.pos.y;
    SetShaderValue(shdrFlashlight, flashlight.posLoc, &flashlight.pos,
                   SHADER_UNIFORM_VEC2);

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

      // flashlight
      if (prezFlags & FLAGS_FLASHLIGHT) {
        BeginShaderMode(shdrFlashlight);
        DrawRectangle(0, 0, srcWidth, srcHeight, WHITE);
        EndShaderMode();
      }
    }
    EndDrawing();
  }
  UnloadTexture(screenTexture);
  UnloadRenderTexture(drawTarget);

  CloseWindow();
  return EXIT_SUCCESS;
}
