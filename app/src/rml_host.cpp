#include "rml_host.hpp"

#include "game_window.hpp"
#include "menu_canvas.hpp"

#include <RmlUi/Core.h>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace z3d::app {
namespace {

// The window height one dp is one pixel at.
constexpr float kDpHeight = 720.0F;

} // namespace

RmlHost::RmlHost(const MenuCanvas &canvas, GameWindow &window, const std::filesystem::path &font)
    : renderer_(canvas.renderer()), window_(window.sdlWindow()), system_(window_),
      render_(renderer_) {
  Rml::SetSystemInterface(&system_);
  Rml::SetRenderInterface(&render_);
  if (!Rml::Initialise()) {
    throw std::runtime_error("RmlUi did not initialise");
  }
  std::ifstream file(font, std::ios::binary);
  font_.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  if (font_.empty() || !Rml::LoadFontFace({font_.data(), font_.size()}, kFontFamily,
                                          Rml::Style::FontStyle::Normal)) {
    Rml::Shutdown();
    throw std::runtime_error("RmlUi cannot load the menu font " + font.string());
  }
  const auto [width, height] = window.pixelSize();
  context_ =
      Rml::CreateContext("screen", {static_cast<int>(width), static_cast<int>(height)}, &render_);
  if (context_ == nullptr) {
    Rml::Shutdown();
    throw std::runtime_error("RmlUi cannot make a context");
  }
  fitToWindow();
}

RmlHost::~RmlHost() {
  Rml::Shutdown();
}

void RmlHost::feed(SDL_Event event) {
  if (event.type == SDL_EVENT_KEY_DOWN &&
      (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)) {
    // The backend types Return's newline even when a document took the key as a command.
    if (context_->ProcessKeyDown(RmlSDL::ConvertKey(static_cast<int>(event.key.key)),
                                 RmlSDL::GetKeyModifierState())) {
      context_->ProcessTextInput('\n');
    }
    return;
  }
  RmlSDL::InputEventHandler(context_, window_, event);
  if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
    fitToWindow();
  }
}

void RmlHost::draw() {
  context_->Update();
  render_.BeginFrame();
  context_->Render();
  render_.EndFrame();
  SDL_RenderPresent(renderer_);
}

void RmlHost::fitToWindow() {
  context_->SetDensityIndependentPixelRatio(static_cast<float>(context_->GetDimensions().y) /
                                            kDpHeight);
}

} // namespace z3d::app
