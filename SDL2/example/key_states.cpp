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
#include <SDL_hints.h>
#include <SDL_image.h>
#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_ttf.h>

#include <array>
#include <cstdint>
#include <cstdio>

#include "common.h"


enum class Key2Texture : int {
  None,
  Up,
  Down,
  Right,
  Left
};


void HandleKeyState(SDL_Renderer* renderer, SDL_Texture** table) {
  const Uint8* cur_key_state = SDL_GetKeyboardState(nullptr);

  SDL_Texture* cur = nullptr;
  if (cur_key_state[SDL_SCANCODE_UP]) {
    cur = table[static_cast<int>(Key2Texture::Up)];
  } else if (cur_key_state[SDL_SCANCODE_DOWN]) {
    cur = table[static_cast<int>(Key2Texture::Down)];
  } else if (cur_key_state[SDL_SCANCODE_LEFT]) {
    cur = table[static_cast<int>(Key2Texture::Left)];
  } else if (cur_key_state[SDL_SCANCODE_RIGHT]) {
    cur = table[static_cast<int>(Key2Texture::Right)];
  } else {
    cur = table[static_cast<int>(Key2Texture::None)];
  }

  SDL_RenderCopy(renderer, cur, nullptr, nullptr);
}


int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Img img(IMG_INIT_JPG);

  auto window_wrap = MakeWindow("mouse_event",
                           SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED,
                                    .y = SDL_WINDOWPOS_UNDEFINED,
                                    .w = 600,
                                    .h = 400},
                           SDL_WINDOW_SHOWN);

  auto window = window_wrap.get();

  auto renderer_wrap = MakeRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  auto renderer = renderer_wrap.get();

  SDL_Texture* textures[5] = {
    IMG_LoadTexture(renderer, "./example/picture/press.png"),
    IMG_LoadTexture(renderer, "./example/picture/up.png"),
    IMG_LoadTexture(renderer, "./example/picture/down.png"),
    IMG_LoadTexture(renderer, "./example/picture/right.png"),
    IMG_LoadTexture(renderer, "./example/picture/left.png"),
  };

  for (auto texture : textures) {
    if (texture == nullptr) {
      std::printf("IMG_LoadTexture Failed err=%s", IMG_GetError());
      return -1;
    }
  }

  SDL_Event event;

  bool is_quit = false;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      }
    }

    // render clear windows
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    HandleKeyState(renderer, textures);

    SDL_RenderPresent(renderer);
  }

  for (auto texture : textures) {
    SDL_DestroyTexture(texture);
  }

  return 0;
}
