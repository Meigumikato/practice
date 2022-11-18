// The MIT License (MIT)

// Copyright (c) 2022 M1racle
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_ttf.h>

#include <array>
#include <cstdio>

#include "common.h"

int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Img img(IMG_INIT_JPG);
  Ttf ttf;

  Window window(
      "rotate_and_flip",
      SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED, .y = SDL_WINDOWPOS_UNDEFINED, .w = 800, .h = 600},
      SDL_WINDOW_SHOWN);
  window.SetResizeable();

  Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Event event;


  TTF_Font* font = TTF_OpenFont("/System/Library/Fonts/SFNS.ttf", 28);
  if (font == nullptr) {
    std::printf("TTF_OpenFont Failed err=%s", TTF_GetError());
    return -1;
  }

  SDL_Color black {.r = 0, .g = 0, .b = 0, .a = 0};
  SDL_Color white {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF};

  SDL_Surface* surface = TTF_RenderUTF8(font, "你好", black, white);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer.GetNativehanlde(), surface);


  bool is_quit = false;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      }
    }
      
    auto window_rect = window.GetWindowFormat();

    SDL_Rect rect{.x = (window_rect.w - surface->w) / 2, .y = (window_rect.h - surface->h) / 2, .w = surface->w, .h = surface->h };

    renderer.SetDrawColor(0xFF, 0xFF, 0xFF, 0xFF);
    renderer.ClearWindow();

    SDL_RenderCopy(renderer.GetNativehanlde(), texture, nullptr, &rect);

    renderer.Show();
  }

  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
  TTF_CloseFont(font);

  return 0;
}
