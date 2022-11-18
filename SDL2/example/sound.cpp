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


#include <SDL_mixer.h>
#include <SDL_render.h>
#include <array>
#include <cstdint>
#include <cstdio>

#include "common.h"



int main() {
  SDL sdl(SDL_INIT_EVERYTHING);
  Img img(IMG_INIT_JPG);
  Mixer mixer(44100, MIX_DEFAULT_FORMAT, 2, 2048);

  auto window_wrap = MakeWindow("mouse_event",
                           SDL_Rect{.x = SDL_WINDOWPOS_UNDEFINED,
                                    .y = SDL_WINDOWPOS_UNDEFINED,
                                    .w = 600,
                                    .h = 400},
                           SDL_WINDOW_SHOWN);

  auto window = window_wrap.get();

  auto renderer_wrap = MakeRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  auto renderer = renderer_wrap.get();

  SDL_Texture* texture = IMG_LoadTexture(renderer, "./example/picture/prompt.png");
  Mix_Music* music = Mix_LoadMUS("./example/sound/beat.wav");
  if (music == nullptr) {
    std::printf("Mix_LoadMUS Failed err=%s", Mix_GetError());
    return -1;
  }

  Mix_Chunk* sounds[4] = {
    Mix_LoadWAV("./example/sound/high.wav"),
    Mix_LoadWAV("./example/sound/medium.wav"),
    Mix_LoadWAV("./example/sound/low.wav"),
    Mix_LoadWAV("./example/sound/scratch.wav"),
  };

  for (auto sound : sounds) {
    if (sound == nullptr) {
      std::printf("Mix_LoadWAV Failed err=%s", Mix_GetError());
      return -1;
    }
  }


  SDL_Event event;

  bool is_quit = false;
  while (!is_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_quit = true;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_1 : 
            Mix_PlayChannel(-1, sounds[0], 0);
            break;
          case SDLK_2 : 
            Mix_PlayChannel(-1, sounds[1], 0);
            break;

          case SDLK_3 : 
            Mix_PlayChannel(-1, sounds[2], 0);
            break;

          case SDLK_4 : 
            Mix_PlayChannel(-1, sounds[3], 0);
            break;
          case SDLK_9 : 
            if (Mix_PlayingMusic() == 0) {
              Mix_PlayMusic(music, -1 /* loop infinite*/);
            } else {
              if (Mix_PausedMusic() == 1) {
                Mix_ResumeMusic();
              } else {
                Mix_PauseMusic();
              }
            }
            break;
          case SDLK_0:
            Mix_HaltMusic();
            break;
        }
      }
    }

    // render clear windows
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(texture);
  Mix_FreeMusic(music);

  for (auto sound : sounds) {
    Mix_FreeChunk(sound);
  }

  return 0;
}
