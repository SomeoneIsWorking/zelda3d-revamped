// A layout's widgets placed as the game's own setters place them (decomp UiWidget, docs/ui.md).
#pragma once

#include "../common/guest.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::mm3d::ui {

using FindWidgetFn = uint8_t *(*)(void *layout, const char *name);

// A translate or scale field set with its flag in flags, and in activeFlags whether it holds its
// default, so the game relays the widget.
inline void set(uint8_t *widget, uint32_t offset, float value, uint32_t flag, float fallback) {
  if (guest::read<float>(widget, offset) == value) {
    return;
  }
  guest::write(widget, offset, value);
  guest::write(widget, kUiWidgetFlags, guest::read<uint32_t>(widget, kUiWidgetFlags) | flag);
  const auto active = guest::read<uint32_t>(widget, kUiWidgetActiveFlags) & ~flag;
  guest::write(widget, kUiWidgetActiveFlags, value == fallback ? active | flag : active);
}

inline void moveX(uint8_t *widget, float x) {
  set(widget, kUiWidgetTranslate, x, kUiWidgetTranslateX, 0.0F);
}

inline void place(uint8_t *widget, float x, float y, float scale) {
  moveX(widget, x);
  set(widget, kUiWidgetTranslate + sizeof(float), y, kUiWidgetTranslateY, 0.0F);
  set(widget, kUiWidgetScale, scale, kUiWidgetScaleX, 1.0F);
  set(widget, kUiWidgetScale + sizeof(float), scale, kUiWidgetScaleY, 1.0F);
}

inline uint8_t *root(void *layout) {
  return static_cast<uint8_t *>(layout) + kUiLayoutRoot;
}

inline uint8_t *find(void *layout, const char *name) {
  return guest::function<FindWidgetFn>(kUiLayoutFindWidget)(layout, name);
}

// Where the widget was last drawn, across the screen.
inline float drawnX(const uint8_t *widget) {
  return guest::read<float>(widget, kUiWidgetWorld + 3 * sizeof(float));
}

} // namespace z3dmods::mm3d::ui
