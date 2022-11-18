// The MIT License (MIT)
//
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

#include "common.h"

int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Img img(IMG_INIT_JPG);

  Window window(
      "animation",
      SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED, .y = SDL_WINDOWPOS_UNDEFINED, .w = 800, .h = 600},
      SDL_WINDOW_SHOWN);
  window.SetResizeable();

  Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  Surface surface("./example/foo.png");
  surface.SetColorKey(0, 0xFF, 0xFF);
  Texture texture(renderer, surface);

  SDL_Event event;

  SDL_Rect sprite[4] = {{0, 0, 64, 205}, {64, 0, 64, 205}, {128, 0, 64, 205}, {192, 0, 64, 205}};

  bool is_quit = false;

  int frame = 0;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      }
    }

    frame++;

    if (frame == 16) {
      frame = 0;
    }

    renderer.SetDrawColor(0xFF, 0xFF, 0xFF, 0xFF);
    renderer.ClearWindow();

    auto window_rect = window.GetWindowFormat();

    int idx = frame / 4;

    SDL_Rect rect{.x = (window_rect.w - sprite[idx].w) / 2,
                  .y = (window_rect.h - sprite[idx].h) / 2,
                  .w = sprite[idx].w,
                  .h = sprite[idx].h};
    renderer.CopyTexture(texture, &sprite[idx], &rect);

    renderer.Show();
  }
  return 0;
}
