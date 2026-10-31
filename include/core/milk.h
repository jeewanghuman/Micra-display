#pragma once

// Milk for the steam pitcher, measured by weight (round Home's Milk sheet).
//
// IMPORTANT: stays free of LVGL/Arduino/BLE/SDL like the other core headers.
//
// A drink calls for a VOLUME of cold milk in the pitcher; the scale reads
// WEIGHT, so the target is the volume times the milk's density. Densities are
// typical chilled values from published measurements (cow's milk 1.027-1.033
// g/ml with skim at the top, soy ~1.02, almond ~1.01; oat drinks span about
// 1.02-1.03). Brands differ by up to about 1 % -- a gram or two in a pitcher.
//
// ORDER IS STORED: both tables are indexed by the NVS keys behind
// IDisplaySettings::milk_drink()/milk_type(), so entries are APPEND-ONLY, and
// readers clamp an index they don't know to the default (an older build
// reading a newer table's index must land somewhere sane).

#include <cmath>

namespace core {

struct MilkDrink {
  const char* name;
  int ml;  // preset: cold milk into the pitcher, before steaming
};

struct MilkType {
  const char* name;
  float g_per_ml;
};

// Presets for a double shot; the sheet's -/+ adjusts each drink's amount and
// it is remembered per drink (IDisplaySettings::milk_ml).
inline constexpr MilkDrink kMilkDrinks[] = {
    {"Cortado", 60},  {"Flat white", 110}, {"Cappuccino", 120},
    {"Latte", 170},   {"Large latte", 260},
};
inline constexpr int kMilkDrinkCount =
    static_cast<int>(sizeof(kMilkDrinks) / sizeof(kMilkDrinks[0]));
inline constexpr int kMilkDrinkDefault = 3;  // Latte

inline constexpr MilkType kMilkTypes[] = {
    {"Whole milk", 1.030f}, {"2% milk", 1.032f},  {"Skim milk", 1.034f},
    {"Oat milk", 1.025f},   {"Soy milk", 1.020f}, {"Almond milk", 1.010f},
};
inline constexpr int kMilkTypeCount =
    static_cast<int>(sizeof(kMilkTypes) / sizeof(kMilkTypes[0]));
inline constexpr int kMilkTypeDefault = 0;  // Whole milk

// The -/+ step and the range a drink's amount can be set to.
inline constexpr int kMilkMlStep = 10;
inline constexpr int kMilkMlMin = 30;
inline constexpr int kMilkMlMax = 500;

inline int milk_drink_index(int i) {
  return (i >= 0 && i < kMilkDrinkCount) ? i : kMilkDrinkDefault;
}
inline int milk_type_index(int i) {
  return (i >= 0 && i < kMilkTypeCount) ? i : kMilkTypeDefault;
}
inline int clamp_milk_ml(int ml) {
  return ml < kMilkMlMin ? kMilkMlMin : (ml > kMilkMlMax ? kMilkMlMax : ml);
}

// Grams of milk to pour for `ml` of milk type `type` (an index, clamped).
inline int milk_target_g(int ml, int type) {
  return static_cast<int>(std::lround(static_cast<float>(ml) *
                                      kMilkTypes[milk_type_index(type)].g_per_ml));
}

}  // namespace core
