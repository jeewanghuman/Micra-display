#pragma once

#include <cstdint>
#include <vector>

// Host display backend: drives LVGL into an in-memory framebuffer and writes
// it out as a PNG. This is the "screen" half of the simulator. The on-device
// build will have a parallel backend that pushes the same LVGL output to the
// real RGB panel instead. The UI layer talks to neither directly.
//
// We render at LV_COLOR_DEPTH 16 (RGB565) to match the real panel, so colors
// in the PNG reflect what the hardware will actually show. The LVGL display
// handle is kept as an opaque pointer so this header stays free of LVGL types.

namespace host {

class PngDisplay {
 public:
  PngDisplay(int width, int height);
  ~PngDisplay();

  PngDisplay(const PngDisplay&) = delete;
  PngDisplay& operator=(const PngDisplay&) = delete;

  // Synchronously render the active LVGL screen into the framebuffer.
  void render_frame();

  // Write the most recently rendered frame to `path` as a PNG. Returns false
  // on write failure. Call render_frame() first.
  bool save_png(const char* path) const;

  // Round-glass preview: dim everything outside the inscribed circle and draw
  // its outline, so a render shows what the 3.4C's circular panel hides. Call
  // after render_frame(), before save_png(). Only the PNG changes.
  void mask_round();

  // Called by the LVGL flush callback (file-local in the .cpp) to deposit the
  // finished RGB565 frame. Not intended for external use.
  void absorb_frame(const uint8_t* rgb565, int width, int height);

 private:
  int width_;
  int height_;
  void* display_;                    // lv_display_t* (opaque here)
  std::vector<uint8_t> render_buf_;  // RGB565 buffer LVGL draws into
  std::vector<uint8_t> rgb888_;      // converted, ready for PNG encoding
};

}  // namespace host
