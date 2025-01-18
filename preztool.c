#ifdef _WIN32
#include "preztool_win32.h"
#define OS_VERTICAL_FLIP (-1)
#define PIXEL_FORMAT GL_BGRA
#define RL_PIXEL_FORMAT PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
#endif // !_WIN32

#ifdef linux
#include "preztool_x11.h"
#define OS_VERTICAL_FLIP (1)
#define PIXEL_FORMAT GL_BGRA
#define RL_PIXEL_FORMAT PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
#endif // !linux

/* OPENGL */
#if !defined(GRAPHICS_API_OPENGL_11) && !defined(GRAPHICS_API_OPENGL_21) &&    \
    !defined(GRAPHICS_API_OPENGL_33) && !defined(GRAPHICS_API_OPENGL_43) &&    \
    !defined(GRAPHICS_API_OPENGL_ES2) && !defined(GRAPHICS_API_OPENGL_ES3)
#define GRAPHICS_API_OPENGL_33
#endif

#if defined(GRAPHICS_API_OPENGL_11)
#if defined(__APPLE__)
#include <OpenGL/gl.h>    // OpenGL 1.1 library for OSX
#include <OpenGL/glext.h> // OpenGL extensions library
#else
// APIENTRY for OpenGL function pointer declarations is required
#if !defined(APIENTRY)
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif
// WINGDIAPI definition. Some Windows OpenGL headers need it
#if !defined(WINGDIAPI) && defined(_WIN32)
#define WINGDIAPI __declspec(dllimport)
#endif

#include <GL/gl.h> // OpenGL 1.1 library
#endif
#endif // !GRAPHICS_API_OPENGL_11

#if defined(GRAPHICS_API_OPENGL_33)
#include "vendor/external/glad.h" // GLAD extensions loading library, includes OpenGL headers
#endif                            // !GRAPHICS_API_OPENGL_33

// NOTE(andrew): no support for OpenGL ES
#if defined(GRAPHICS_API_OPENGL_ES3)
#endif // !GRAPHICS_API_OPENGL_ES3

#if defined(GRAPHICS_API_OPENGL_ES2)
#endif // !GRAPHICS_API_OPENGL_ES2
/* CLOSE OPENGL */

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>

enum prez_flags {
  FLAGS_MIRROR_X = 1 << 0,
  FLAGS_MIRROR_Y = 1 << 1,
  FLAGS_FLASHLIGHT = 1 << 2,
  FLAGS_HIGHLIGHT = 1 << 3,
};

struct light {
  Vector2 pos;
  Vector3 color;
  float inner;
  float innerAlpha;
  float outer;
  float outerAlpha;

  unsigned int posLoc;
  unsigned int colorLoc;
  unsigned int innerLoc;
  unsigned int innerAlphaLoc;
  unsigned int outerLoc;
  unsigned int outerAlphaLoc;
};

void copy_light_shader_from_struct(Shader shader, struct light *light);

void screenshot_as_texture(unsigned char *data, int srcWidth, int srcHeight,
                           Texture *screenTexture) {
  screenTexture->width = srcWidth;
  screenTexture->height = srcHeight;
  screenTexture->format = RL_PIXEL_FORMAT; // WARN(andrew): wrong one
  screenTexture->mipmaps = 1;
  glBindTexture(GL_TEXTURE_2D, 0); // Free any old binding
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &screenTexture->id); // Generate texture id
  glBindTexture(GL_TEXTURE_2D, screenTexture->id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenTexture->width,
               screenTexture->height, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  free(data);
}

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

  Texture screenTexture = {0};
  screenshot_as_texture(data, srcWidth, srcHeight, &screenTexture);

  /* prez data */
  unsigned char prezFlags = 0;
  // flashlight
  Shader shdrFlashlight =
      LoadShader(NULL, "assets/shaders/flashlight.frag.glsl");
  struct light flashlight = {
      .pos = {0, 0},
      .posLoc = GetShaderLocation(shdrFlashlight, "flashlight.pos"),
      .color = {0, 0, 0},
      .colorLoc = GetShaderLocation(shdrFlashlight, "flashlight.color"),
      .inner = (float)srcWidth / 15,
      .innerLoc = GetShaderLocation(shdrFlashlight, "flashlight.inner"),
      .innerAlpha = 0.0,
      .innerAlphaLoc =
          GetShaderLocation(shdrFlashlight, "flashlight.innerAlpha"),
      .outer = (float)srcWidth / 15 * 1.5,
      .outerLoc = GetShaderLocation(shdrFlashlight, "flashlight.outer"),
      .outerAlpha = 1.0,
      .outerAlphaLoc =
          GetShaderLocation(shdrFlashlight, "flashlight.outerAlpha"),
  };
  copy_light_shader_from_struct(shdrFlashlight, &flashlight);

  // highlight
  Shader shdrHighlight =
      LoadShader(NULL, "assets/shaders/flashlight.frag.glsl");
  struct light highlight = {
      .pos = {0, 0},
      .posLoc = GetShaderLocation(shdrHighlight, "flashlight.pos"),
      .color = {255, 255, 0},
      .colorLoc = GetShaderLocation(shdrHighlight, "flashlight.color"),
      .inner = (float)srcWidth / 100,
      .innerLoc = GetShaderLocation(shdrHighlight, "flashlight.inner"),
      .innerAlpha = 0.8,
      .innerAlphaLoc =
          GetShaderLocation(shdrHighlight, "flashlight.innerAlpha"),
      .outer = (float)srcWidth / 100 * 1.5,
      .outerLoc = GetShaderLocation(shdrHighlight, "flashlight.outer"),
      .outerAlpha = 0.0,
      .outerAlphaLoc =
          GetShaderLocation(shdrHighlight, "flashlight.outerAlpha"),
  };
  copy_light_shader_from_struct(shdrHighlight, &highlight);

  // camera
  Camera2D camera = {0};
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
    // reset camera
    if (IsKeyPressed(KEY_Z)) {
      camera.zoom = 1.0;
      camera.offset = (Vector2){0, 0};
      camera.target = (Vector2){0, 0};
      camera.rotation = 0;
    }

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
    if (IsKeyPressed(KEY_H)) {
      prezFlags ^= FLAGS_HIGHLIGHT;
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

    // highlight
    highlight.pos = GetMousePosition();
    highlight.pos.y = srcHeight - highlight.pos.y;
    SetShaderValue(shdrHighlight, highlight.posLoc, &highlight.pos,
                   SHADER_UNIFORM_VEC2);

    // render
    BeginDrawing();
    {
      ClearBackground(BLACK);

      BeginMode2D(camera);
      {
        DrawTextureRec(screenTexture,
                       (Rectangle){0, 0, srcWidth * flipVector.x,
                                   OS_VERTICAL_FLIP * srcHeight * flipVector.y},
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
      // highlight
      if (prezFlags & FLAGS_HIGHLIGHT) {
        BeginShaderMode(shdrHighlight);
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

void copy_light_shader_from_struct(Shader shader, struct light *light) {
  SetShaderValue(shader, light->posLoc, &light->pos, SHADER_UNIFORM_VEC3);
  SetShaderValue(shader, light->colorLoc, &light->color, SHADER_UNIFORM_VEC3);
  SetShaderValue(shader, light->innerAlphaLoc, &light->innerAlpha,
                 SHADER_UNIFORM_FLOAT);
  SetShaderValue(shader, light->outerAlphaLoc, &light->outerAlpha,
                 SHADER_UNIFORM_FLOAT);
  SetShaderValue(shader, light->innerLoc, &light->inner, SHADER_UNIFORM_FLOAT);
  SetShaderValue(shader, light->outerLoc, &light->outer, SHADER_UNIFORM_FLOAT);
}
