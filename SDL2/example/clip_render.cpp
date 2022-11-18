// The MIT License (MIT)
//
// Copyright (c) 2022 M1ralce
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

#include "common.h"


int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Img img(IMG_INIT_JPG);

  Window window(
      "clip",
      SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED, .y = SDL_WINDOWPOS_UNDEFINED, .w = 800, .h = 600},
      SDL_WINDOW_SHOWN);
  window.SetResizeable();

  Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

  Surface surface("./example/dots.png");
  surface.SetColorKey(0, 0xFF, 0xFF);

  Texture texture(renderer, surface);

  SDL_Event event;

  SDL_Rect sprite[4] = {
      {0, 0, 100, 100}, {100, 0, 100, 100}, {0, 100, 100, 100}, {100, 100, 100, 100}};

  bool is_quit = false;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      }
    }

    renderer.SetDrawColor(0xFF, 0xFF, 0xFF, 0xFF);
    renderer.ClearWindow();

    auto window_rect = window.GetWindowFormat();

    SDL_Rect concer[4] = {
        {0, 0, sprite[0].w, sprite[0].h},
        {window_rect.w - sprite[1].w, 0, sprite[1].w, sprite[1].h},
        {0, window_rect.h - sprite[2].h, sprite[2].w, sprite[2].h},
        {window_rect.w - sprite[3].w, window_rect.h - sprite[3].h, sprite[3].w, sprite[3].h}};

    for (int i = 0; i < 4; ++i) {
      renderer.CopyTexture(texture, &sprite[i], &concer[i]);
    }

    renderer.Show();
  }

  return 0;
}
