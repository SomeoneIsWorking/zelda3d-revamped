// A layout of the menus' own kind (HudItemButtons, decomp docs/pause.md) built by the patch: quads
// drawn from a sheet, placed in the screen's pixels, faded and drawn by the game.
#pragma once

#include "../common/guest.hpp"
#include "game.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {

class MenuLayout {
public:
  // `count` quads, with counts: a layout without draws its quads blended as the icons' plain white.
  void create(uint32_t count) {
    void *memory = guest::function<NewFn>(kHeapNew)(kHudItemButtonsSize);
    layout_ = reinterpret_cast<uintptr_t>(guest::function<InitFn>(kHudItemButtonsInit)(
        memory, static_cast<int32_t>(count), kItemsKind, 0));
    count_ = count;
  }

  // Its quads drawn from `texture`, `width` by `height` pixels.
  void sheet(void *texture, float width, float height) const {
    auto *size = reinterpret_cast<volatile float *>(quads() + kQuadBatchTextureSize);
    size[0] = width;
    size[1] = height;
    guest::function<SetTextureFn>(kMenuModelSetTexture)(
        guest::read<void *>(layout_ + kHudItemButtonsModel), 0, texture, kLinear, kLinear,
        kClampToEdge, kClampToEdge);
  }

  // Quad `quad`'s cell in the sheet's pixels; a negative size mirrors it.
  void cell(uint32_t quad, const float (&corner)[2], const float (&size)[2]) const {
    guest::function<SetUvsFn>(kQuadBatchSetUvs)(reinterpret_cast<void *>(quads()), corner, size, 1,
                                                static_cast<int32_t>(quad));
  }

  // Quad `quad`'s square in the screen's pixels.
  void place(uint32_t quad, int32_t x, int32_t y, int32_t width, int32_t height) const {
    guest::function<SetRectFn>(kHudItemButtonsSetRect2D)(
        reinterpret_cast<void *>(layout_), static_cast<int32_t>(quad), x, y, width, height);
  }

  // Every quad at `alpha`, uploaded with its counts.
  void fade(float alpha) const {
    auto *at = reinterpret_cast<void *>(layout_);
    for (uint32_t quad = 0; quad < count_; ++quad) {
      guest::function<SetAlphaFn>(kHudItemButtonsSetAlpha)(at, static_cast<int32_t>(quad), alpha);
    }
    guest::function<LayoutFn>(kHudItemButtonsUpdateAmmo)(at);
  }

  void draw() const {
    guest::function<LayoutFn>(kHudItemButtonsDraw)(reinterpret_cast<void *>(layout_));
  }

  [[nodiscard]] uintptr_t address() const {
    return layout_;
  }

  [[nodiscard]] uintptr_t quads() const {
    return guest::read<uintptr_t>(layout_ + kHudItemButtonsQuads);
  }

private:
  using NewFn = void *(*)(uint32_t size);
  using InitFn = void *(*)(void *layout, int32_t count, int32_t kind, int32_t noCounts);
  using LayoutFn = void (*)(void *layout);
  using SetRectFn = void (*)(void *layout, int32_t node, int32_t x, int32_t y, int32_t width,
                             int32_t height);
  using SetUvsFn = void (*)(void *batch, const float *corner, const float *size, int32_t count,
                            int32_t first);
  using SetTextureFn = void (*)(void *model, int32_t unit, void *texture, uint16_t minFilter,
                                uint16_t magFilter, uint16_t wrapS, uint16_t wrapT);
  // The game passes floats in VFP registers; the patch is built softfp.
  // NOLINTNEXTLINE(modernize-use-using): only a typedef takes the calling convention.
  typedef void (*SetAlphaFn)(void *layout, int32_t node, float alpha)
      __attribute__((pcs("aapcs-vfp")));

  // HudItemButtons_Init's kind for a layout of items.
  static constexpr int32_t kItemsKind = 0;
  // GL_LINEAR and GL_CLAMP_TO_EDGE, as ItemMenu_Init binds the screen's sheet.
  static constexpr uint16_t kLinear = 0x2601;
  static constexpr uint16_t kClampToEdge = 0x812F;

  uintptr_t layout_ = 0;
  uint32_t count_ = 0;
};

} // namespace z3dmods::oot3d
