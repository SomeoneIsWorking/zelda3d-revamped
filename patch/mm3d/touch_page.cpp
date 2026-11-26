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

void PageFrame::keepFullSize(uint8_t *group) const {
  if (group == nullptr) {
    return;
  }
  ui::place(group, -left / scale, -top / scale, 1.0F / scale);
}

void FrameRoots::place(void *layout, const PageFrame &frame) {
  uint8_t *root = ui::root(layout);
  Entry &entry = find(root);
  const Place now = read(root);
  const bool known = entry.root == root;
  entry.root = root;
  for (size_t i = 0; i < kFields; ++i) {
    if (!known || now.fields[i] != entry.placed.fields[i]) {
      entry.game.fields[i] = now.fields[i];
    }
  }
  const float *game = entry.game.fields;
  ui::place(root, frame.x(game[0]), frame.y(game[1]), frame.scale * game[2], frame.scale * game[3]);
  entry.placed = read(root);
  entry.used = ++clock_;
}

FrameRoots::Place FrameRoots::read(const uint8_t *root) {
  return {{guest::read<float>(root, kUiWidgetTranslate),
           guest::read<float>(root, kUiWidgetTranslate + sizeof(float)),
           guest::read<float>(root, kUiWidgetScale),
           guest::read<float>(root, kUiWidgetScale + sizeof(float))}};
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
