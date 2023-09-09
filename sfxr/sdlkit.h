#ifndef SDLKIT_H
#define SDLKIT_H

#include <SDL.h>
#include <set>
#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define ERROR(x) error(__FILE__, __LINE__, #x)
#define VERIFY(x) do { if (!(x)) ERROR(x); } while (0)

static void error(const char *file, unsigned int line, const char *msg) {
  fprintf(stderr, "[!] %s:%u  %s\n", file, line, msg);
  exit(1);
}

typedef Uint32 DWORD;
typedef Uint16 WORD;

#define DIK_SPACE SDLK_SPACE
#define DIK_RETURN SDLK_RETURN
#define DDK_WINDOW 0

#define hWndMain 0
#define hInstanceMain 0

#define Sleep(x) SDL_Delay(x)

std::set<unsigned int> keys;

void ddkInit();      // Will be called on startup
bool ddkCalcFrame(); // Will be called every frame, return true to continue running or false to quit
void ddkFree();      // Will be called on shutdown

class DPInput {
public:
  DPInput(int, int) {}
  ~DPInput() {}
  static void Update() {}

#if SDL_VERSION_ATLEAST(2, 0, 0)
  static bool KeyPressed(SDL_Keycode key)
#else
  static bool KeyPressed(SDLKey key)
#endif
  {
    bool r = (keys.find(key) != keys.end());
    keys.erase(key);
    return r;
  }
};

static Uint32 *ddkscreen32;
static Uint16 *ddkscreen16;
static int ddkpitch;
static int mouse_x, mouse_y, mouse_px, mouse_py;
static bool mouse_left = false, mouse_right = false, mouse_middle = false;
static bool mouse_leftclick = false, mouse_rightclick = false, mouse_middleclick = false;

static SDL_Surface *sdlscreen = nullptr;
#if SDL_VERSION_ATLEAST(2, 0, 0)
static SDL_Window *sdlwindow = nullptr;
static SDL_Renderer *sdlrenderer = nullptr;
static SDL_Texture *sdltexture = nullptr;
#endif

static void sdlflip() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
  SDL_UpdateTexture(sdltexture, nullptr, sdlscreen->pixels, sdlscreen->pitch);
  SDL_RenderClear(sdlrenderer);
  SDL_RenderCopy(sdlrenderer, sdltexture, nullptr, nullptr);
  SDL_RenderPresent(sdlrenderer);
#else
  SDL_Flip(sdlscreen);
#endif
}

static void sdlupdate() {
  mouse_px = mouse_x;
  mouse_py = mouse_y;
  Uint8 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
  bool mouse_left_p = mouse_left;
  bool mouse_right_p = mouse_right;
  bool mouse_middle_p = mouse_middle;
  mouse_left = buttons & SDL_BUTTON(1);
  mouse_right = buttons & SDL_BUTTON(3);
  mouse_middle = buttons & SDL_BUTTON(2);
  mouse_leftclick = mouse_left && !mouse_left_p;
  mouse_rightclick = mouse_right && !mouse_right_p;
  mouse_middleclick = mouse_middle && !mouse_middle_p;
}

static bool ddkLock() {
  SDL_LockSurface(sdlscreen);
  ddkpitch = sdlscreen->pitch / (sdlscreen->format->BitsPerPixel == 32 ? 4 : 2);
  ddkscreen16 = (Uint16 *)(sdlscreen->pixels);
  ddkscreen32 = (Uint32 *)(sdlscreen->pixels);
  return true;
}

static void ddkUnlock() {
  SDL_UnlockSurface(sdlscreen);
}

static void ddkSetMode(int width, int height, int bpp, int /* refreshrate */, int fullscreen, const char *title) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
  VERIFY(sdlscreen = SDL_CreateRGBSurface(0, width, height, bpp, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
  VERIFY(sdlwindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
  VERIFY(sdlrenderer = SDL_CreateRenderer(sdlwindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
  VERIFY(sdltexture = SDL_CreateTexture(sdlrenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height));
#else
  VERIFY(sdlscreen = SDL_SetVideoMode(width, height, bpp, fullscreen ? SDL_FULLSCREEN : 0));
  SDL_WM_SetCaption(title, nullptr);
#endif
}

static Uint32 RGBA(int r, int g, int b, int a) {
  return SDL_MapRGBA(sdlscreen->format, r, g, b, a);
}

using FileSelectorHandler = void (*)(const char *path);
static FileSelectorHandler FileSelectorCallback;

#ifdef HAVE_GTK

#include <gtk/gtk.h>

#if GTK_CHECK_VERSION(4, 10, 0)

static void FileSelectorLoad(FileSelectorHandler callback) {
  FileSelectorCallback = callback;

  GtkFileDialog *dialog = gtk_file_dialog_new();

  gtk_file_dialog_open(
      dialog, nullptr, nullptr,
      +[](GObject *source, GAsyncResult *result, gpointer) {
        GtkFileDialog *dialog = GTK_FILE_DIALOG(source);

        GFile *file = gtk_file_dialog_open_finish(dialog, result, nullptr);
        if (!file)
          return;

        char *path = g_file_get_path(file);
        FileSelectorCallback(path);

        g_free(path);
        g_object_unref(file);
      },
      nullptr);

  g_object_unref(dialog);

  while (g_list_model_get_n_items(gtk_window_get_toplevels()) > 0)
    g_main_context_iteration(nullptr, true);
}

static void FileSelectorSave(FileSelectorHandler callback) {
  FileSelectorCallback = callback;

  GtkFileDialog *dialog = gtk_file_dialog_new();

  gtk_file_dialog_save(
      dialog, nullptr, nullptr,
      +[](GObject *source, GAsyncResult *result, gpointer) {
        GtkFileDialog *dialog = GTK_FILE_DIALOG(source);

        GFile *file = gtk_file_dialog_save_finish(dialog, result, nullptr);
        if (!file)
          return;

        char *path = g_file_get_path(file);
        FileSelectorCallback(path);

        g_free(path);
        g_object_unref(file);
      },
      nullptr);

  g_object_unref(dialog);

  while (g_list_model_get_n_items(gtk_window_get_toplevels()) > 0)
    g_main_context_iteration(nullptr, true);
}

#elif GTK_CHECK_VERSION(3, 0, 0)

static void FileSelectorLoad(FileSelectorHandler callback) {
  FileSelectorCallback = callback;

  GtkFileChooserNative *native = gtk_file_chooser_native_new("Load file",
                                                             nullptr,
                                                             GTK_FILE_CHOOSER_ACTION_OPEN,
                                                             "_Load",
                                                             "_Cancel");

  gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(native), true);

  if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(native)) == GTK_RESPONSE_ACCEPT) {
    char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native));
    FileSelectorCallback(path);

    g_free(path);
  }

  gtk_native_dialog_destroy(GTK_NATIVE_DIALOG(native));
  g_object_unref(native);

  while (gtk_events_pending())
    gtk_main_iteration();
}

static void FileSelectorSave(FileSelectorHandler callback) {
  FileSelectorCallback = callback;

  GtkFileChooserNative *native = gtk_file_chooser_native_new("Save file",
                                                             nullptr,
                                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                                             "_Save",
                                                             "_Cancel");

  gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(native), true);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(native), true);

  if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(native)) == GTK_RESPONSE_ACCEPT) {
    char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native));
    FileSelectorCallback(path);

    g_free(path);
  }

  gtk_native_dialog_destroy(GTK_NATIVE_DIALOG(native));
  g_object_unref(native);

  while (gtk_events_pending())
    gtk_main_iteration();
}

#endif

#else

#ifndef __EMSCRIPTEN__
static void FileSelectorUnimplemented(FileSelectorHandler) {
  fprintf(stderr, "ERROR: file selector is not implemented in this build.\n");
}

#define FileSelectorLoad FileSelectorUnimplemented
#define FileSelectorSave FileSelectorUnimplemented
#endif

#endif

static void sdlquit() {
  ddkFree();
#if SDL_VERSION_ATLEAST(2, 0, 0)
  if (sdltexture != nullptr) {
    SDL_DestroyTexture(sdltexture);
  }
  if (sdlrenderer != nullptr) {
    SDL_DestroyRenderer(sdlrenderer);
  }
  if (sdlwindow != nullptr) {
    SDL_DestroyWindow(sdlwindow);
  }
#endif
  SDL_Quit();
}

static void sdlinit() {
  VERIFY(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));
#ifndef __EMSCRIPTEN__
  SDL_Surface *icon;
  icon = SDL_LoadBMP("/usr/share/sfxr/sfxr.bmp");
  if (!icon)
    icon = SDL_LoadBMP("sfxr.bmp");
  if (icon)
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_SetWindowIcon(sdlwindow, icon);
#else
    SDL_WM_SetIcon(icon, nullptr);
#endif
#endif
  atexit(sdlquit);
  keys.clear();
  ddkInit();
}

static void loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      exit(0);
    case SDL_KEYDOWN:
      keys.insert(event.key.keysym.sym);
      break;
    default:
      break;
    }
  }

  sdlupdate();

  if (!ddkCalcFrame())
    return;

  sdlflip();
}

int main() {
#ifdef HAVE_GTK
#if GTK_CHECK_VERSION(4, 0, 0)
  gtk_init();
#else
  gtk_init(nullptr, nullptr);
#endif
#endif

  sdlinit();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 0, true);
#else
  while (true)
    loop();
#endif

  return 0;
}

#endif
