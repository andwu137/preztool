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

#define DEFAULT_DRAW_SIZE 5

enum prez_flags {
  FLAGS_MIRROR_X = 1 << 0,
  FLAGS_MIRROR_Y = 1 << 1,
  FLAGS_FLASHLIGHT = 1 << 2,
  FLAGS_HIGHLIGHT = 1 << 3,
  FLAGS_BRUSH_PREVIEW = 1 << 4,
  FLAGS_ERASE = 1 << 5,
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

void setup_light_shader(Shader shader, struct light *l);
void screenshot_as_texture(unsigned char *data, int srcWidth, int srcHeight,
                           Texture *screenTexture);

int main(int argc, char *argv[]) {
  // screenshot
  unsigned char *data = NULL;
  int srcWidth = 0;
  int srcHeight = 0;
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
  // camera
  Camera2D camera = {0};
  camera.zoom = 1.0f;

  // draw
  RenderTexture2D drawTarget = LoadRenderTexture(srcWidth, srcHeight);
  Color drawColors[] = {RED, GREEN, BLUE, WHITE, BLACK};
  int drawColorSelected = 0;
  float brushDrawSize = DEFAULT_DRAW_SIZE;

  // flashlight
  Shader shdrFlashlight =
      LoadShader(NULL, "assets/shaders/flashlight.frag.glsl");
  struct light flashlight = {
      .pos = {0, 0},
      .color = {0, 0, 0},
      .inner = (float)srcWidth / 15,
      .innerAlpha = 0.0,
      .outer = (float)srcWidth / 15 * 1.5,
      .outerAlpha = 1.0,
  };
  setup_light_shader(shdrFlashlight, &flashlight);

  // highlight
  Shader shdrHighlight =
      LoadShader(NULL, "assets/shaders/flashlight.frag.glsl");
  struct light highlight = {
      .pos = {0, 0},
      .color = {255, 255, 0},
      .inner = (float)srcWidth / 100,
      .innerAlpha = 0.8,
      .outer = (float)srcWidth / 100 * 1.5,
      .outerAlpha = 0.0,
  };
  setup_light_shader(shdrHighlight, &highlight);

  // brush preview
  Shader shdrBrushPreview =
      LoadShader(NULL, "assets/shaders/flashlight.frag.glsl");
  struct light brushPreview = {
      .pos = {0, 0},
      .color = {(float)drawColors[drawColorSelected].r / 255,
                (float)drawColors[drawColorSelected].g / 255,
                (float)drawColors[drawColorSelected].b / 255},
      .inner = DEFAULT_DRAW_SIZE,
      .innerAlpha = 1.0,
      .outer = DEFAULT_DRAW_SIZE,
      .outerAlpha = 0.0,
  };
  setup_light_shader(shdrBrushPreview, &brushPreview);

  // mouse
  Vector2 mousePos = {0, 0};
  Vector2 prevMouseWorldPos = {0, 0};
  Vector2 mouseWorldPos = {0, 0};

  while (!WindowShouldClose()) {
    // retake screenshot
    if (IsKeyPressed(KEY_T)) {
      UnloadTexture(screenTexture);
      SetWindowState(FLAG_WINDOW_HIDDEN);
      {
        WaitTime((double)3 / 30); // WARN(andrew): this could fail to wait for
                                  // the display to fully finish updating
        // PERF(andrew): should use shm, but the docs suck
        screenshot(&data, &srcWidth, &srcHeight);
        screenshot_as_texture(data, srcWidth, srcHeight, &screenTexture);
      }
      ClearWindowState(FLAG_WINDOW_HIDDEN);
      SetWindowPosition(0, 0);
    }

    // reset camera
    if (IsKeyPressed(KEY_T) || IsKeyPressed(KEY_Z)) {
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
    if (IsKeyPressed(KEY_E)) {
      prezFlags ^= FLAGS_ERASE;
    }
    if (IsKeyPressed(KEY_V)) {
      prezFlags ^= FLAGS_MIRROR_Y;

      // for prev
      mousePos.y = srcHeight - mousePos.y;
      mouseWorldPos.y = srcHeight - mouseWorldPos.y;
    }
    if (!IsKeyDown(KEY_LEFT_SHIFT)) {
      if (IsKeyPressed(KEY_F)) {
        prezFlags ^= FLAGS_FLASHLIGHT;
      }
      if (IsKeyPressed(KEY_H)) {
        prezFlags ^= FLAGS_HIGHLIGHT;
      }
      if (IsKeyPressed(KEY_P)) {
        prezFlags ^= FLAGS_BRUSH_PREVIEW;
      }
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
      if (IsKeyDown(KEY_LEFT_SHIFT)) {
        if (IsKeyDown(KEY_B)) {
          float scaleFactor = 1.15;
          if (wheel > 0) {
            scaleFactor = 1.0f / scaleFactor;
          }
          brushDrawSize *= fabsf(wheel) * scaleFactor;
          brushPreview.inner = brushDrawSize * camera.zoom;
          brushPreview.outer = brushDrawSize * camera.zoom;
          SetShaderValue(shdrBrushPreview, brushPreview.innerLoc,
                         &brushPreview.inner, SHADER_UNIFORM_FLOAT);
          SetShaderValue(shdrBrushPreview, brushPreview.outerLoc,
                         &brushPreview.outer, SHADER_UNIFORM_FLOAT);
        }
        if (IsKeyDown(KEY_F)) { // flashlight size
          float scaleFactor = 1.15;
          if (wheel > 0) {
            scaleFactor = 1.0f / scaleFactor;
          }
          flashlight.inner *= fabsf(wheel) * scaleFactor;
          flashlight.outer = flashlight.inner * 1.5;
          SetShaderValue(shdrFlashlight, flashlight.innerLoc, &flashlight.inner,
                         SHADER_UNIFORM_FLOAT);
          SetShaderValue(shdrFlashlight, flashlight.outerLoc, &flashlight.outer,
                         SHADER_UNIFORM_FLOAT);
        }
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
        brushPreview.inner = brushDrawSize * camera.zoom;
        brushPreview.outer = brushDrawSize * camera.zoom;
        SetShaderValue(shdrBrushPreview, brushPreview.innerLoc,
                       &brushPreview.inner, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shdrBrushPreview, brushPreview.outerLoc,
                       &brushPreview.outer, SHADER_UNIFORM_FLOAT);
      }
    }

    // change draw color
    {
      bool drawDirty = false;
      if (IsKeyPressed(KEY_RIGHT)) {
        drawColorSelected++;
        if (drawColorSelected >=
            (int)(sizeof(drawColors) / sizeof(*drawColors))) {
          drawColorSelected = 0;
        }
        drawDirty = true;
      } else if (IsKeyPressed(KEY_LEFT)) {
        drawColorSelected--;
        if (drawColorSelected < 0) {
          drawColorSelected = sizeof(drawColors) / sizeof(*drawColors) - 1;
        }
        drawDirty = true;
      }

      // send to shader
      if (drawDirty) {
        brushPreview.color.x = (float)drawColors[drawColorSelected].r / 255;
        brushPreview.color.y = (float)drawColors[drawColorSelected].g / 255;
        brushPreview.color.z = (float)drawColors[drawColorSelected].b / 255;
        SetShaderValue(shdrBrushPreview, brushPreview.colorLoc,
                       &brushPreview.color, SHADER_UNIFORM_VEC3);
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
      Color color = drawColors[drawColorSelected];

      BeginTextureMode(drawTarget);
      {
        if (prezFlags & FLAGS_ERASE) { // erase
          rlSetBlendFactorsSeparate(
              RL_ZERO, RL_ONE_MINUS_SRC_ALPHA,
              RL_ZERO, RL_ONE_MINUS_SRC_ALPHA,
              RL_FUNC_ADD, RL_FUNC_ADD);
          BeginBlendMode(BLEND_CUSTOM_SEPARATE);
          // color = WHITE;
        }

        DrawCircleV(mouseWorldPos, brushDrawSize, color);
        DrawLineEx(prevMouseWorldPos, mouseWorldPos, brushDrawSize * 2, color);

        if (prezFlags & FLAGS_ERASE) { // erase
          EndBlendMode();
        }
      }
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

    // brush preview
    brushPreview.pos = GetMousePosition();
    brushPreview.pos.y = srcHeight - brushPreview.pos.y;
    SetShaderValue(shdrBrushPreview, brushPreview.posLoc, &brushPreview.pos,
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
      // brush preview
      if (prezFlags & FLAGS_BRUSH_PREVIEW) {
        BeginShaderMode(shdrBrushPreview);
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

void setup_light_shader(Shader shader, struct light *l) {
  l->posLoc = GetShaderLocation(shader, "flashlight.pos");
  l->colorLoc = GetShaderLocation(shader, "flashlight.color");
  l->innerLoc = GetShaderLocation(shader, "flashlight.inner");
  l->innerAlphaLoc = GetShaderLocation(shader, "flashlight.innerAlpha");
  l->outerLoc = GetShaderLocation(shader, "flashlight.outer");
  l->outerAlphaLoc = GetShaderLocation(shader, "flashlight.outerAlpha");

  SetShaderValue(shader, l->posLoc, &l->pos, SHADER_UNIFORM_VEC2);
  SetShaderValue(shader, l->colorLoc, &l->color, SHADER_UNIFORM_VEC3);
  SetShaderValue(shader, l->innerAlphaLoc, &l->innerAlpha,
                 SHADER_UNIFORM_FLOAT);
  SetShaderValue(shader, l->outerAlphaLoc, &l->outerAlpha,
                 SHADER_UNIFORM_FLOAT);
  SetShaderValue(shader, l->innerLoc, &l->inner, SHADER_UNIFORM_FLOAT);
  SetShaderValue(shader, l->outerLoc, &l->outer, SHADER_UNIFORM_FLOAT);
}

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
