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

const int BUTTON_WIDTH = 300;
const int BUTTON_HEIGHT = 200;
const int TOTAL_BUTTONS = 4;

enum ButtonSprite {
  BUTTON_SPRITE_MOUSE_OUT = 0,
  BUTTON_SPRITE_MOUSE_OVER_MOTION = 1,
  BUTTON_SPRITE_MOUSE_DOWN = 2,
  BUTTON_SPRITE_MOUSE_UP = 3,
  BUTTON_SPRITE_TOTAL = 4
};

class Button {
 public:
  Button(int x, int y, SDL_Rect* loc_rect, SDL_Texture* all)
      : all_(all),
        loc_rect_(loc_rect),
        render_loc_{.x = x, .y = y, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT} {}

  void HandleMouseEvent(SDL_Event* event) {
    int x, y;
    SDL_GetMouseState(&x, &y);

    if (!(x >= render_loc_.x && x <= render_loc_.x + render_loc_.w &&
      y >= render_loc_.y && y <= render_loc_.y + render_loc_.h)) {
      status = BUTTON_SPRITE_MOUSE_OUT;
      return;
    }

    switch (event->type) {
      case SDL_MOUSEMOTION: {
        status = BUTTON_SPRITE_MOUSE_OVER_MOTION;
        break;
      }

      case SDL_MOUSEBUTTONUP: {
        status = BUTTON_SPRITE_MOUSE_UP;
        break;
      }

      case SDL_MOUSEBUTTONDOWN: {
        status = BUTTON_SPRITE_MOUSE_DOWN;
        break;
      }

      default: {
        status = BUTTON_SPRITE_MOUSE_OUT;
        break;
      }
    }
  }

  void Render(SDL_Renderer* renderer) {
      SDL_RenderCopy(renderer, all_, loc_rect_ + status, &render_loc_);
  }

 private:
  ButtonSprite status{BUTTON_SPRITE_MOUSE_OUT};
  int render_index_{0};
  SDL_Texture* all_;
  SDL_Rect* loc_rect_;
  SDL_Rect render_loc_;
};

int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Img img(IMG_INIT_JPG);

  auto window_wrap = MakeWindow("mouse_event",
                           SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED,
                                    .y = SDL_WINDOWPOS_UNDEFINED,
                                    .w = BUTTON_WIDTH * 2,
                                    .h = BUTTON_HEIGHT * 2},
                           SDL_WINDOW_SHOWN);

  auto window = window_wrap.get();

  auto renderer_wrap = MakeRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  auto renderer = renderer_wrap.get();

  SDL_Rect sprites[4] = {
      {.x = 0, .y = 0, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT},
      {.x = 0, .y = BUTTON_HEIGHT, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT},
      {.x = 0, .y = BUTTON_HEIGHT * 2, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT},
      {.x = 0, .y = BUTTON_HEIGHT * 3, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT},
  };

  SDL_Texture* all = IMG_LoadTexture(renderer, "./example/picture/button.png");
  if (all == nullptr) {
    printf("IMG_LoadTexture Failed err=%s\n", IMG_GetError());
    return -1;
  }

  Button buttons[4] = {
      Button(0, 0, sprites, all),
      Button(BUTTON_WIDTH, 0, sprites, all),
      Button(0, BUTTON_HEIGHT, sprites, all),
      Button(BUTTON_WIDTH, BUTTON_HEIGHT, sprites, all),
  };

  SDL_Event event;

  bool is_quit = false;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      }
      for (auto& button : buttons) {
        button.HandleMouseEvent(&event);
      }
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);


    for (auto& button : buttons) {
      button.Render(renderer);
    } 

    SDL_RenderPresent(renderer);
  }

  return 0;
}
