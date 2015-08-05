/**
   \file monitor.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "monitor.hpp"

#include "monitor-font.hpp"

#include <array>
#include <algorithm>

namespace nebula {

using std::chrono::duration_cast;

static const std::unordered_map<Word, MonitorOperation> kOperationMap{
    {0u, MonitorOperation::MapVideoMemory},
    {1u, MonitorOperation::MapFontMemory},
    {2u, MonitorOperation::MapPaletteMemory},
    {3u, MonitorOperation::SetBorderColor},
    {4u, MonitorOperation::DumpFont},
    {5u, MonitorOperation::DumpPalette}};

void Monitor::handleInterrupt(MonitorOperation operation,
                              ProcessorState& processor_state) {
  Word b = processor_state.read(Register::B);

  switch (operation) {
    case MonitorOperation::MapVideoMemory: {
      LOG(MONITOR, info) << "'MapVideoMemory'";

      if (b != 0u) {
        state_.is_connected = true;
        state_.video_memory_offset = b;

        LOG(MONITOR, info) << format("Video memory offset is 0x%04x.") % b;
      } else {
        state_.is_connected = false;
        state_.since_last_blink = std::chrono::microseconds{0};
        LOG(MONITOR, info) << "Disconnected.";
      }
    } break;

    case MonitorOperation::MapFontMemory: {
      LOG(MONITOR, info) << "'MapFontMemory'";

      state_.font_memory_offset = b;
      LOG(MONITOR, info) << format("Font memory offset is 0x%04x.") % b;
    } break;

    case MonitorOperation::MapPaletteMemory: {
      LOG(MONITOR, info) << "'MapPaletteMemory'";

      state_.palette_memory_offset = b;
      LOG(MONITOR, info) << format("Palette memory offset is 0x%04x.") % b;
    } break;

    case MonitorOperation::SetBorderColor: {
      LOG(MONITOR, info) << "'SetBorderColor'";

      auto offset = static_cast<std::uint8_t>(b & 0xf);
      state_.border_color_offset = offset;

      LOG(MONITOR, info) << format("Border color offset is 0x%04x.") % offset;
    } break;

    case MonitorOperation::DumpFont: {
      LOG(MONITOR, info) << "'DumpFont'";

      Word offset = b;

      for (const auto& pair : kDefaultFont) {
        memory_->write(offset++, pair.first);
        memory_->write(offset++, pair.second);
      }

      processor_state.tickCycleCount(256u);
      LOG(MONITOR, info) << "Finished dumping font.";
    } break;

    case MonitorOperation::DumpPalette: {
      LOG(MONITOR, info) << "'DumpPalette'";

      Word offset = b;
      for (const auto& word : kMonitorDefaultPalette) {
        memory_->write(offset++, word);
      }

      processor_state.tickCycleCount(16u);
      LOG(MONITOR, info) << "Finished dumping palette.";
    } break;
  }
}

graphics::Color Monitor::colorFromOffset(std::uint8_t offset) {
  Word color_code;

  if (state_.palette_memory_offset != 0u) {
    color_code = memory_->read(state_.palette_memory_offset + offset);
  } else {
    color_code = kMonitorDefaultPalette[offset];
  }

  // Since only 4 bits are provided for each channel, we shift
  // everything to the left by 4 bits so that the bits that ARE provided
  // are interpreted as high-order bits.
  graphics::Red red{static_cast<std::uint8_t>((color_code & 0x0f00) >> 4)};
  graphics::Green green{static_cast<std::uint8_t>(color_code & 0x00f0)};
  graphics::Blue blue{static_cast<std::uint8_t>((color_code & 0x000f) << 4)};

  return graphics::Color{red, green, blue};
}

void Monitor::drawPixel(const graphics::Window& window,
                        units::Quantity<units::simulated::Width> x,
                        units::Quantity<units::simulated::Height> y) {

  units::Quantity<units::real::Length> x_offset{x};
  units::Quantity<units::real::Length> y_offset{y};

  units::Quantity<units::real::Length> width{1_spw};
  units::Quantity<units::real::Length> height{1_sph};

  graphics::drawRectangle(
      window, kBorderWidth + x_offset, kBorderHeight + y_offset, width, height);
}

void Monitor::drawCell(const graphics::Window& window,
                       std::size_t cell_x,
                       std::size_t cell_y,
                       CharacterOffset ch_offset,
                       ForegroundColorOffset fg_offset,
                       BackgroundColorOffset bg_offset,
                       bool do_blink) {

  auto fg = colorFromOffset(fg_offset);
  auto bg = colorFromOffset(bg_offset);

  auto base_offset_x = cell_x * kCellWidth;
  auto base_offset_y = cell_y * kCellHeight;

  // Draw a single column of data for the given cell.
  auto draw_column = [&](std::uint8_t column_data,
                         units::Quantity<units::simulated::Width> x) {

    for (auto y = 0_sph; y < kCellHeight; y += 1_sph) {
      bool is_fg = (column_data & (1 << y.value())) &&
                   ((do_blink && state_.is_blink_visible) || (!do_blink));

      graphics::setDrawingColor(window, is_fg ? fg : bg);
      drawPixel(window, base_offset_x + x, base_offset_y + y);
    }
  };

  std::pair<Word, Word> ch;

  if (state_.font_memory_offset != 0) {
    auto base_offset = state_.font_memory_offset;
    ch = std::make_pair(memory_->read(base_offset + (2 * ch_offset.value)),
                        memory_->read(base_offset + (2 * ch_offset.value) + 1));
  } else {
    ch = kDefaultFont[ch_offset.value];
  }

  // Split each word into a 1 byte pair, and draw all the columns for
  // the cell.
  draw_column((ch.first & 0xff00) >> 8, 0_spw);
  draw_column(ch.first & 0x00ff, 1_spw);
  draw_column((ch.second & 0xff00) >> 8, 2_spw);
  draw_column(ch.second & 0x00ff, 3_spw);
}

void Monitor::clear(const graphics::Window& window) {
  graphics::setDrawingColor(window, graphics::kColorBlack);

  graphics::drawRectangle(window,
                          kBorderWidth,
                          kBorderHeight,
                          kScreenHorizontalResolution,
                          kScreenVerticalResolution);
}

void Monitor::drawBorder(const graphics::Window& window) {
  auto border_color =
      colorFromOffset(static_cast<std::uint8_t>(state_.border_color_offset));
  graphics::setDrawingColor(window, border_color);

  // Left.
  graphics::drawRectangle(
      window, 0_px, 0_px, kBorderWidth, kWindowVerticalResolution);

  // Right.
  graphics::drawRectangle(window,
                          kWindowHorizontalResolution - kBorderWidth,
                          0_px,
                          kBorderWidth,
                          kWindowVerticalResolution);

  // Top.
  graphics::drawRectangle(
      window, 0_px, 0_px, kWindowHorizontalResolution, kBorderHeight);

  // Bottom.
  graphics::drawRectangle(window,
                          0_px,
                          kWindowVerticalResolution - kBorderHeight,
                          kWindowHorizontalResolution,
                          kBorderHeight);
}

void Monitor::drawFromMemory(const graphics::Window& window) {
  for (std::size_t y = 0u; y < kCellsPerScreenHeight; ++y) {
    for (std::size_t x = 0u; x < kCellsPerScreenWidth; ++x) {
      auto memory_offset = static_cast<Word>((y * kCellsPerScreenWidth) + x +
                                             state_.video_memory_offset);
      Word w{memory_->read(memory_offset)};

      bool do_blink{(w & 0x0080) != 0};
      auto ch_offset = static_cast<std::uint8_t>(w & 0x007f);
      auto fg_offset = static_cast<std::uint8_t>((w & 0xf000) >> 12);
      auto bg_offset = static_cast<std::uint8_t>((w & 0x0f00) >> 8);

      drawCell(window,
               x,
               y,
               CharacterOffset{ch_offset},
               ForegroundColorOffset{fg_offset},
               BackgroundColorOffset{bg_offset},
               do_blink);
    }
  }
}

unique_ptr<MonitorState> Monitor::start() {
  notify();

  LOG(MONITOR, info) << "Started.";

  while (status() == SimulationStatus::Running) {
    auto now = std::chrono::system_clock::now();

    // Scope for lock guard.
    {
      std::lock_guard<std::mutex> lock{mutex_};

      if (interrupt_sink_.isActive()) {
        LOG(MONITOR, info) << "Got interrupt.";

        auto& processor_state = interrupt_sink_.processorState();
        auto a = processor_state.read(Register::A);
        auto operation = get(a, kOperationMap);

        if (operation) {
          handleInterrupt(*operation, processor_state);
        }

        interrupt_sink_.respond();
        LOG(MONITOR, info) << "Finished handling interrupt.";
      }
    }  // end lock guard.

    std::this_thread::sleep_until(now + kMonitorClockPeriod);
  }

  LOG(MONITOR, info) << "Shutting down.";

  return make_unique<MonitorState>();
}

void Monitor::renderGraphics(const graphics::Window& window) {
  std::lock_guard<std::mutex> lock{mutex_};

  drawBorder(window);

  if (state_.is_connected) {
    drawFromMemory(window);
  } else {
    clear(window);
  }

  graphics::render(window);

  auto now = std::chrono::system_clock::now();
  auto since_last_render = now - state_.last_render_time;

  if (state_.is_connected) {
    if (state_.since_last_blink >=
        duration_cast<std::chrono::microseconds>(kMonitorBlinkPeriod)) {
      state_.since_last_blink = std::chrono::microseconds{0};
      state_.is_blink_visible = !state_.is_blink_visible;
    } else {
      state_.since_last_blink +=
          duration_cast<std::chrono::microseconds>(since_last_render);
    }
  }

  state_.last_render_time = now;
}

}  // namespace nebula
