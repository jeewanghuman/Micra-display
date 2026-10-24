#pragma once

#include "core/battery.h"

// Host stand-in: an idle battery at 82%, so the sim previews the percent view.
// set_hw(false) poses a board with no battery circuit (the round 3.4C).

namespace host {

class FakeBattery : public core::IBattery {
 public:
  void set_hw(bool on) { hw_ = on; }

  core::BatteryState battery() const override {
    core::BatteryState s;
    if (!hw_) {
      s.hw = false;
      s.usb = true;
      return s;
    }
    s.present = true;
    s.percent = 82;
    s.volts = 3.9f;
    return s;  // idle on battery (no USB) for the sim preview
  }

 private:
  bool hw_ = true;
};

}  // namespace host
