// This file is generated. Do not edit!
// see https://github.com/hpvb/dynload-wrapper for details
// generated by ../dynload-wrapper/generate-wrapper.py 0.3 on 2023-01-25 17:46:12
// flags: ../dynload-wrapper/generate-wrapper.py --include ./thirdparty/linuxbsd_headers/wayland/wayland-cursor.h --sys-include "./thirdparty/linuxbsd_headers/wayland/wayland-cursor.h" --soname libwayland-cursor.so.0 --init-name wayland_cursor --output-header platform/linuxbsd/wayland/dynwrappers/wayland-cursor-so_wrap.h --output-implementation platform/linuxbsd/wayland/dynwrappers/wayland-cursor-so_wrap.c
//
#include <stdint.h>

#define wl_cursor_theme_load wl_cursor_theme_load_dylibloader_orig_wayland_cursor
#define wl_cursor_theme_destroy wl_cursor_theme_destroy_dylibloader_orig_wayland_cursor
#define wl_cursor_theme_get_cursor wl_cursor_theme_get_cursor_dylibloader_orig_wayland_cursor
#define wl_cursor_image_get_buffer wl_cursor_image_get_buffer_dylibloader_orig_wayland_cursor
#define wl_cursor_frame wl_cursor_frame_dylibloader_orig_wayland_cursor
#define wl_cursor_frame_and_duration wl_cursor_frame_and_duration_dylibloader_orig_wayland_cursor
#include "./thirdparty/linuxbsd_headers/wayland/wayland-cursor.h"
#undef wl_cursor_theme_load
#undef wl_cursor_theme_destroy
#undef wl_cursor_theme_get_cursor
#undef wl_cursor_image_get_buffer
#undef wl_cursor_frame
#undef wl_cursor_frame_and_duration
#include <dlfcn.h>
#include <stdio.h>
struct wl_cursor_theme* (*wl_cursor_theme_load_dylibloader_wrapper_wayland_cursor)(const char*, int,struct wl_shm*);
void (*wl_cursor_theme_destroy_dylibloader_wrapper_wayland_cursor)(struct wl_cursor_theme*);
struct wl_cursor* (*wl_cursor_theme_get_cursor_dylibloader_wrapper_wayland_cursor)(struct wl_cursor_theme*,const char*);
struct wl_buffer* (*wl_cursor_image_get_buffer_dylibloader_wrapper_wayland_cursor)(struct wl_cursor_image*);
int (*wl_cursor_frame_dylibloader_wrapper_wayland_cursor)(struct wl_cursor*, uint32_t);
int (*wl_cursor_frame_and_duration_dylibloader_wrapper_wayland_cursor)(struct wl_cursor*, uint32_t, uint32_t*);
int initialize_wayland_cursor(int verbose) {
  void *handle;
  char *error;
  handle = dlopen("libwayland-cursor.so.0", RTLD_LAZY);
  if (!handle) {
    if (verbose) {
      fprintf(stderr, "%s\n", dlerror());
    }
    return(1);
  }
  dlerror();
// wl_cursor_theme_load
  *(void **) (&wl_cursor_theme_load_dylibloader_wrapper_wayland_cursor) = dlsym(handle, "wl_cursor_theme_load");
  if (verbose) {
    error = dlerror();
    if (error != NULL) {
      fprintf(stderr, "%s\n", error);
    }
  }
// wl_cursor_theme_destroy
  *(void **) (&wl_cursor_theme_destroy_dylibloader_wrapper_wayland_cursor) = dlsym(handle, "wl_cursor_theme_destroy");
  if (verbose) {
    error = dlerror();
    if (error != NULL) {
      fprintf(stderr, "%s\n", error);
    }
  }
// wl_cursor_theme_get_cursor
  *(void **) (&wl_cursor_theme_get_cursor_dylibloader_wrapper_wayland_cursor) = dlsym(handle, "wl_cursor_theme_get_cursor");
  if (verbose) {
    error = dlerror();
    if (error != NULL) {
      fprintf(stderr, "%s\n", error);
    }
  }
// wl_cursor_image_get_buffer
  *(void **) (&wl_cursor_image_get_buffer_dylibloader_wrapper_wayland_cursor) = dlsym(handle, "wl_cursor_image_get_buffer");
  if (verbose) {
    error = dlerror();
    if (error != NULL) {
      fprintf(stderr, "%s\n", error);
    }
  }
// wl_cursor_frame
  *(void **) (&wl_cursor_frame_dylibloader_wrapper_wayland_cursor) = dlsym(handle, "wl_cursor_frame");
  if (verbose) {
    error = dlerror();
    if (error != NULL) {
      fprintf(stderr, "%s\n", error);
    }
  }
// wl_cursor_frame_and_duration
  *(void **) (&wl_cursor_frame_and_duration_dylibloader_wrapper_wayland_cursor) = dlsym(handle, "wl_cursor_frame_and_duration");
  if (verbose) {
    error = dlerror();
    if (error != NULL) {
      fprintf(stderr, "%s\n", error);
    }
  }
return 0;
}
