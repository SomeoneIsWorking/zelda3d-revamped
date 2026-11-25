// The options screen of one game, drawn on the title picker's canvas: the options' categories down
// the left, the chosen one's rows on the right. Up and down pick a category, A or right opens it;
// in a category up and down pick an option, left, right or A change it and B goes back to the
// categories. B on the categories, Esc, View, F10 or Guide go back. Changes are saved as they are
// made. Display also holds a Fullscreen row (game_window.hpp); with `folders`, System holds a row
// that leaves to change the games and mods folders (folder_setup.hpp).
#pragma once

#include "menu_canvas.hpp"

#include <SDL3/SDL_pixels.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace z3d::app {

class GameOptions;
class GameWindow;

class OptionsMenu {
public:
  OptionsMenu(const MenuCanvas &canvas, GameWindow &window, const PadInput &pad,
              GameOptions &options, std::string title, bool folders);

  enum class Left { Back, ForFolders };
  // Until the player goes back, asks for the folders, or the window closes.
  [[nodiscard]] Left run();

private:
  struct Row {
    enum class Kind : uint8_t { Option, Fullscreen, Folders };
    Kind kind = Kind::Option;
    // The option's index in GameOptions::items().
    size_t option = 0;
  };
  struct Category {
    std::string name;
    std::vector<Row> rows;
  };

  // Each logs the menu's new state, which the input tests wait on.
  void pickCategory(size_t index);
  void select(size_t index);
  void step(int direction);
  [[nodiscard]] const Row &row() const {
    return categories_[category_].rows[selected_];
  }
  [[nodiscard]] std::string rowId(const Row &row) const;
  void draw();
  void text(const std::string &line, float x, float y, SDL_Color color, bool rightAligned);

  const MenuCanvas &canvas_;
  SDL_Renderer *renderer_;
  TTF_TextEngine *engine_;
  TTF_Font *font_;
  GameWindow &window_;
  const PadInput &pad_;
  GameOptions &options_;
  std::string title_;
  std::vector<Category> categories_;
  size_t category_ = 0;
  // In the rows of the category, or on the categories.
  bool inRows_ = false;
  size_t selected_ = 0;
  // The first row drawn.
  size_t first_ = 0;
};

} // namespace z3d::app
