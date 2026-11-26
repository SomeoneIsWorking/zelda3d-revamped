#include "touch_page.hpp"

#include "../common/guest.hpp"
#include "symbols.hpp"
#include "ui_widget.hpp"

namespace z3dmods::mm3d {

void PageFrame::place(void *layout) const {
  ui::place(ui::root(layout), left, top, scale);
}

void PageFrame::unscale(uint8_t *child, float touchX, float touchY, float toX, float toY) const {
  if (child == nullptr) {
    return;
  }
  ui::scale(child, 1.0F / scale, 1.0F / scale);
  const float childX = guest::read<float>(child, kUiWidgetTranslate);
  const float childY = guest::read<float>(child, kUiWidgetTranslate + sizeof(float));
  ui::place(ui::root(guest::read<void *>(child, kUiWidgetLayout)),
            toX - left - touchX + (1.0F - scale) * childX,
            toY - top - touchY + (1.0F - scale) * childY, 1.0F);
}

void PageFrame::moveArea(uint8_t *area, float touchX, float touchY, float toX, float toY) const {
  if (area == nullptr) {
    return;
  }
  ui::place(area, (toX - left - touchX) / scale, (toY - top - touchY) / scale, 1.0F / scale);
}

void FrameRoots::place(void *layout, const PageFrame &frame) {
  uint8_t *root = ui::root(layout);
  Entry &entry = find(root);
  const Place now = read(root);
  if (entry.root != root || !(now == entry.placed)) {
    entry.root = root;
    entry.game = now;
  }
  ui::place(root, frame.x(entry.game.x), frame.y(entry.game.y), frame.scale * entry.game.scaleX,
            frame.scale * entry.game.scaleY);
  entry.placed = read(root);
  entry.used = ++clock_;
}

FrameRoots::Place FrameRoots::read(const uint8_t *root) {
  return {guest::read<float>(root, kUiWidgetTranslate),
          guest::read<float>(root, kUiWidgetTranslate + sizeof(float)),
          guest::read<float>(root, kUiWidgetScale),
          guest::read<float>(root, kUiWidgetScale + sizeof(float))};
}

FrameRoots::Entry &FrameRoots::find(const uint8_t *root) {
  Entry *oldest = &entries_[0];
  for (Entry &entry : entries_) {
    if (entry.root == root) {
      return entry;
    }
    if (entry.used < oldest->used) {
      oldest = &entry;
    }
  }
  oldest->root = nullptr;
  return *oldest;
}

} // namespace z3dmods::mm3d
