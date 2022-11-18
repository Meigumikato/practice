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

#include <SDL_events.h>
#include <SDL_keycode.h>

#include <array>

#include "common.h"

int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Img img(IMG_INIT_JPG);

  Window window(
      "modulate",
      SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED, .y = SDL_WINDOWPOS_UNDEFINED, .w = 800, .h = 600},
      SDL_WINDOW_SHOWN);
  window.SetResizeable();

  Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

  Texture texture(renderer, "./example/colors.png");

  SDL_Event event;

  std::array<std::uint8_t, 3> rgb {255, 255, 255};

  bool is_quit = false;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_q:
            rgb[0] += 20;
            break;

          case SDLK_w:
            rgb[1] += 20;
            break;

          case SDLK_e:
            rgb[2] += 20;
            break;

          case SDLK_a:
            rgb[0] -= 20;
            break;

          case SDLK_s:
            rgb[1] -= 20;
            break;

          case SDLK_d:
            rgb[2] -= 20;
            break;

          default:
            break;
        }
      }
    }

    renderer.SetDrawColor(0xFF, 0xFF, 0xFF, 0xFF);
    renderer.ClearWindow();

    texture.SetColor(rgb[0], rgb[1], rgb[2]);
    renderer.CopyTexture(texture, nullptr, nullptr);

    renderer.Show();
  }

  return 0;
}
