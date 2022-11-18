#include <SDL.h>
#include <SDL_image.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <cstdio>
#include <cstdlib>

constexpr int WindowWidth = 800;
constexpr int WindowHeight = 600;



class Wrap {

 public:
  Wrap() = default;

  ~Wrap() {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(windows_);
    SDL_Quit();
  }

  bool Init() {

    int ret = SDL_Init(SDL_INIT_EVERYTHING);

    if (ret != 0)  {
      std::printf("SDL_Init Failed err=%s", SDL_GetError());
      return false;
    }

    windows_ = SDL_CreateWindow("example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);
    if (windows_ == nullptr) {
      std::printf("SDL_CreateWindow Failed err=%s", SDL_GetError());
      return false;
    }

    renderer_ = SDL_CreateRenderer(windows_, -1, 0);
    if (renderer_ == nullptr) {
      std::printf("SDL_CreateRenderer Failed err=%s", SDL_GetError());
      return false;
    }

    SDL_SetWindowResizable(windows_, SDL_TRUE);

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
      std::printf("IMG_Init Failed err=%s", IMG_GetError());
    }

    return true;
  }

  void DrawGeometry(int real_width, int real_height) {

    SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer_);

    SDL_SetRenderDrawColor(renderer_, 0xFF, 0, 0, 0xFF);
    SDL_Rect fill_rect = { .x = real_width / 4, .y = real_height / 4, .w = real_width / 2, .h = real_height / 2 };
    SDL_RenderFillRect(renderer_, &fill_rect);

    SDL_SetRenderDrawColor(renderer_, 0, 0xFF, 0, 0xFF);
    SDL_Rect outline_rect = {.x = real_width / 8, .y = real_height / 8, .w = real_width * 3 / 4, .h = real_height * 3 / 4 };
    SDL_RenderDrawRect(renderer_, &outline_rect);

    // SDL_RenderDrawLine(renderer_, 0, WindowHeight / 2, int x2, int y2)
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0xFF, 0xFF);
    SDL_RenderDrawLine(renderer_, 0, real_height / 2, real_width, real_height / 2);


    SDL_SetRenderDrawColor(renderer_, 0, 0xFF, 0, 0);
    for (int i = 0; i < real_height; i += 4) {
      SDL_RenderDrawPoint(renderer_, real_width / 2 , i);
    }
  }


  bool Viewport(int real_width, int real_height) {
    // SDL_Rect 
    //
    png_texture_ = IMG_LoadTexture(renderer_, "./example/viewport.png");
    if (png_texture_ == nullptr) {
      std::printf("IMG_LoadTexture Failed err=%s", IMG_GetError());
      return false;
    }

    SDL_Rect left_angle = { .x = 0, .y = 0, .w = real_width / 2, .h = real_height / 2 };
    SDL_Rect right_angle = { .x = real_width / 2, .y = 0, .w = real_width / 2, .h = real_height / 2 };
    SDL_Rect bottom = { .x = 0, .y = real_height / 2, .w = real_width, .h = real_height / 2};

    int ret = SDL_RenderSetViewport(renderer_, &left_angle);
    if (ret != 0) {
      return false;
    }
    SDL_RenderCopy(renderer_, png_texture_, nullptr, nullptr);

    ret = SDL_RenderSetViewport(renderer_, &right_angle);
    if (ret != 0) {
      return false;
    }
    SDL_RenderCopy(renderer_, png_texture_, nullptr, nullptr);

    ret = SDL_RenderSetViewport(renderer_, &bottom);
    if (ret != 0) {
      return false;
    }
    SDL_RenderCopy(renderer_, png_texture_, nullptr, nullptr);

    return true;
  }

  bool LoadFooBackGround() {

    auto foo_surface = IMG_Load("./example/foo.png");
    if (foo_surface == nullptr) {
      std::printf("LoadFooBackGround failed %s", IMG_GetError());
      return false;
    }

    auto background_surface = IMG_Load("./example/background.png");
    if (background_surface == nullptr) {
      std::printf("LoadFooBackGround failed %s", IMG_GetError());
      return false;
    }

    background_texture_ = SDL_CreateTextureFromSurface(renderer_, background_surface);
    SDL_SetColorKey(foo_surface, SDL_TRUE, SDL_MapRGB(foo_surface->format, 0, 0xFF, 0xFF));
    png_texture_ = SDL_CreateTextureFromSurface(renderer_, foo_surface);
    
    w_ = foo_surface->w; 
    h_ = foo_surface->h;

    SDL_FreeSurface(foo_surface);
    SDL_FreeSurface(background_surface);

    return true;
  }

  bool FooBackground(int real_width, int real_height) {
    SDL_Rect dst_rect {.x = real_width / 2, .y = real_height * 3 / 8, .w = w_, .h = h_ };
    // SDL_RenderCopy(renderer_, background_texture_, nullptr, nullptr);
    SDL_RenderCopy(renderer_, png_texture_, nullptr, &dst_rect);

    return true;
  }

  void Loop() {

    if (!LoadFooBackGround()) {
      std::abort();
    }

    while (!is_quit_) {

      while (SDL_PollEvent(&event_)) {
        if (event_.type == SDL_QUIT) {
          is_quit_ = true;
        }

        SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer_);

        int real_width, real_height;
        SDL_GetWindowSize(windows_, &real_width, &real_height);

        // DrawGeometry(real_width, real_height);

        // Viewport(real_width, real_height);

        FooBackground(real_height, real_height);

        SDL_RenderPresent(renderer_);
      }
    }
  }

 private:

  int w_, h_;

  bool is_quit_{false};

  SDL_Texture* background_texture_;
  SDL_Texture* png_texture_;
  SDL_Event event_;
  SDL_Renderer* renderer_;
  SDL_Window* windows_;
};



int main (int argc, char *argv[]) {
  
  Wrap w;
  if (!w.Init()) {
    return -1;
  }

  w.Loop();

  return 0;
}
