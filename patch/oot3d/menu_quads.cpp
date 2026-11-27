#include "menu_quads.hpp"

#include "../common/guest.hpp"
#include "symbols.hpp"

namespace z3dmods::oot3d {
namespace {

using SetRectsFn = void (*)(void *batch, const float *corner, const float *size, int32_t count,
                            int32_t first);

} // namespace

void shiftQuads(uintptr_t batch, const QuadTables &tables, QuadRange quads, float dx) {
  for (uint32_t quad = quads.first; quad < quads.first + quads.count; ++quad) {
    const float corner[] = {guest::read<float>(tables.positions + quad * 2 * sizeof(float)) + dx,
                            guest::read<float>(tables.positions + (quad * 2 + 1) * sizeof(float))};
    guest::function<SetRectsFn>(kQuadBatchSetRects)(
        reinterpret_cast<void *>(batch), corner,
        reinterpret_cast<const float *>(tables.sizes + quad * 2 * sizeof(float)), 1,
        static_cast<int32_t>(quad));
  }
}

} // namespace z3dmods::oot3d
