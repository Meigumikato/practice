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

#include <SDL_timer.h>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <chrono>

#include "common.h"

constexpr int kWindowWidth = 600;
constexpr int kWindowHeight = 400;

SDL_Surface* RenderFont(SDL_Renderer* renderer, TTF_Font* font, const char* text) {
  SDL_Color fg{0, 0, 0, 255};
  SDL_Color bg{255, 255, 255, 255};
  auto surface = TTF_RenderUTF8(font, text, fg, bg);
  if (surface == nullptr) {
    std::printf("surface is nullptr");
    return nullptr;
  }
  return surface;
}

int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Ttf ttf;

  auto window_wrap = MakeWindow(
      "calculate_fps",
      SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED, .y = SDL_WINDOWPOS_UNDEFINED, .w = 600, .h = 400},
      SDL_WINDOW_SHOWN);

  auto window = window_wrap.get();

  auto renderer_wrap = MakeRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  auto renderer = renderer_wrap.get();

  auto font = TTF_OpenFont("/System/Library/Fonts/SFNS.ttf", 28);
  if (font == nullptr) {
    return -1;
  }

  std::uint64_t frame_count = 0;

  auto start = std::chrono::high_resolution_clock::now();

  std::ostringstream oss;
  SDL_Event event;
  bool is_quit = false;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      }
    }

    auto cur = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::chrono::seconds::period> interval = cur - start;

    // render clear windows
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    constexpr int FPS = 60.0f;
    auto cur_fps = frame_count / interval.count();
    if (cur_fps > FPS) {
      SDL_Delay((((double)frame_count / FPS) - interval.count()) * 1000);
    }

    oss.str("");
    oss << "FPS " << cur_fps;
    auto time_surface = RenderFont(renderer, font, oss.str().c_str());

    SDL_Rect location{.x = (kWindowWidth - time_surface->w) / 2,
                      .y = (kWindowHeight - time_surface->h) / 2,
                      .w = time_surface->w,
                      .h = time_surface->h};

    auto time_texture = SDL_CreateTextureFromSurface(renderer, time_surface);
    if (time_texture == nullptr) {
      return -1;
    }
    SDL_FreeSurface(time_surface);

    SDL_RenderCopy(renderer, time_texture, nullptr, &location);
    SDL_DestroyTexture(time_texture);

    SDL_RenderPresent(renderer);


    frame_count++;
  }

  TTF_CloseFont(font);

  return 0;
}
