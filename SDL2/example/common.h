#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

class SDL {
 public:
  SDL(int flags) {
    if (SDL_Init(flags) != 0) {
      throw std::logic_error(SDL_GetError());
    }
  }

  ~SDL() { SDL_Quit(); }
};

class Img {
 public:
  Img(int flags) {
    if ((IMG_Init(flags) & flags) != flags) {
    }
  }
  ~Img() { IMG_Quit(); }
};

class Ttf {
 public:
  Ttf() {
    if (TTF_Init() != 0) {
    }
  }

  ~Ttf() { TTF_Quit(); }
};

class Mixer {
 public:
  Mixer(int frequency, std::uint16_t format, int channels, int chunck_size) {
    if (Mix_OpenAudio(frequency, format, channels, chunck_size) < 0) {
      throw std::logic_error(Mix_GetError());
    }
  }
  
  ~Mixer() {
    Mix_Quit();
  }
};

inline std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> MakeWindow(
    const char* tilte, const SDL_Rect& position_rect, int flags) {
  return std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
      SDL_CreateWindow(tilte, position_rect.x, position_rect.y, position_rect.w, position_rect.h,
                       flags),
      SDL_DestroyWindow);
}

class Window {
 public:
  Window(const char* title, const SDL_Rect& rect, uint32_t flags) {
    window_ = SDL_CreateWindow(title, rect.x, rect.y, rect.w, rect.h, flags);
    if (window_ == nullptr) {
      throw std::logic_error(SDL_GetError());
    }
  }

  void SetResizeable() { SDL_SetWindowResizable(window_, SDL_TRUE); }

  SDL_Rect GetWindowFormat() {
    SDL_Rect rect;
    SDL_GetWindowSize(window_, &rect.w, &rect.h);
    SDL_GetWindowPosition(window_, &rect.x, &rect.y);
    return rect;
  }

  SDL_Window* GetNativehanlde() { return window_; }

  ~Window() { SDL_DestroyWindow(window_); }

 private:
  SDL_Window* window_;
};

class Surface {
 public:
  Surface(const char* file) {
    surface_ = IMG_Load(file);
    if (surface_ == nullptr) {
    }
  }

  void SetColorKey(int r, int g, int b) {
    SDL_SetColorKey(surface_, true, SDL_MapRGB(surface_->format, r, g, b));
  }

  SDL_Surface* operator->() { return surface_; }

  SDL_Surface* GetNativehanlde() { return surface_; }

  ~Surface() { SDL_FreeSurface(surface_); }

 private:
  SDL_Surface* surface_;
};

class Renderer;
class Texture {
 public:
  Texture(Renderer& render, const char* file);

  Texture(Renderer& render, Surface& surface);

  void SetColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    SDL_SetTextureColorMod(texture_, r, g, b);
  }

  void SetBlendMode() { SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND); }

  void SetOpacity(std::uint8_t opacity) { SDL_SetTextureAlphaMod(texture_, opacity); }

  auto GetNativehanlde() { return texture_; }

 private:
  SDL_Texture* texture_;
};

inline std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> MakeRenderer(
    SDL_Window* window, int idx, int flags) {
  return std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(
      SDL_CreateRenderer(window, idx, flags), SDL_DestroyRenderer);
}

class Renderer {
 public:
  Renderer(Window& render_window, int index, int flags) : window_(&render_window) {
    renderer_ = SDL_CreateRenderer(render_window.GetNativehanlde(), index, flags);
  }

  void SetDrawColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) {
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
  }

  void ClearWindow() { SDL_RenderClear(renderer_); }

  void CopyTexture(Texture& texture, const SDL_Rect* src, const SDL_Rect* dst) {
    SDL_RenderCopy(renderer_, texture.GetNativehanlde(), src, dst);
  }

  void CopyTextureEx(Texture& texture, const SDL_Rect* src, const SDL_Rect* dst, double angle,
                     const SDL_Point* center, SDL_RendererFlip flip_type) {
    SDL_RenderCopyEx(renderer_, texture.GetNativehanlde(), src, dst, angle, center, flip_type);
  }

  void Show() { SDL_RenderPresent(renderer_); }

  SDL_Renderer* GetNativehanlde() { return renderer_; }

  ~Renderer() { SDL_DestroyRenderer(renderer_); }

 private:
  Window* window_;
  SDL_Renderer* renderer_;
};

inline Texture::Texture(Renderer& render, const char* file) {
  texture_ = IMG_LoadTexture(render.GetNativehanlde(), file);
  if (texture_ == nullptr) {
    throw std::logic_error(IMG_GetError());
  }
}

inline Texture::Texture(Renderer& render, Surface& surface) {
  texture_ = SDL_CreateTextureFromSurface(render.GetNativehanlde(), surface.GetNativehanlde());
  if (texture_ == nullptr) {
    throw std::logic_error(IMG_GetError());
  }
}

class TtfFont {
 public:
  TtfFont(const char* file) { TTF_OpenFont(file, 28); }

 private:
};
