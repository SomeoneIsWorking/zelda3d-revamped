// RmlUi on the menu canvas's renderer, for a document over the game: RmlUi's own SDL backends
// (RmlUi/Backends) draw it and turn the window's events into its input. One context the window's
// size in pixels, its dp scaled to the window's height, and the menu font as kFontFamily.
#pragma once

#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>

#include <filesystem>
#include <vector>

namespace Rml {
class Context;
} // namespace Rml

namespace z3d::app {

class GameWindow;
class MenuCanvas;

class RmlHost {
public:
  static constexpr const char *kFontFamily = "z3d";

  RmlHost(const MenuCanvas &canvas, GameWindow &window, const std::filesystem::path &font);
  ~RmlHost();
  RmlHost(const RmlHost &) = delete;
  RmlHost &operator=(const RmlHost &) = delete;
  RmlHost(RmlHost &&) = delete;
  RmlHost &operator=(RmlHost &&) = delete;

  [[nodiscard]] Rml::Context &context() {
    return *context_;
  }
  void feed(SDL_Event event);
  // Updates the context and presents it.
  void draw();

private:
  void fitToWindow();

  SDL_Renderer *renderer_;
  SDL_Window *window_;
  SystemInterface_SDL system_;
  RenderInterface_SDL render_;
  // RmlUi reads a face from memory for as long as it is loaded.
  std::vector<unsigned char> font_;
  Rml::Context *context_ = nullptr;
};

} // namespace z3d::app
