#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/snake2.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <cmath>
#include <numbers>

std::uint16_t float_to_fixed(float f)
{
   const auto max_fraction = 0b0'0000000'11111111;
   const auto max_integer = 0b0'1111111'00000000 >> 8;
   const bool is_negative = f < 0.0;
   float integral;
   const auto fraction = std::abs(std::modf(f, &integral));
   const auto frac_part = std::clamp(static_cast<int>(fraction * max_fraction), 0, max_fraction);
   const auto int_part = std::clamp(static_cast<int>(std::abs(integral)), 0, max_integer);
   const auto ret_val = (int_part << 8) | frac_part;
   return is_negative ? ~ret_val : ret_val;
}

volatile std::uint16_t* bg_screen_loc_at(gba::bg_opt::screen_base_block loc, int x, int y) noexcept
{
   return gba::bg_screen_loc(loc) + x + y * 32;
}

void write_at(gba::bg_opt::screen_base_block loc, const char* to_write, const int x, const int y) noexcept
{
   int write_x = x;
   int write_y = y;
   for (int i = 0; to_write[i] != '\0'; ++i) {
      if (to_write[i] == '\n') {
         write_x = x;
         write_y += 1;
      }
      else {
         *bg_screen_loc_at(loc, write_x, write_y) = to_write[i];
         write_x += 1;
      }
   }
}

// Assumes 8x8 box for first and 6x6 box for second for calling simplicity
bool collision(int x1, int y1, int x2, int y2) { return x1 < x2 + 8 && x1 + 6 > x2 && y1 < y2 + 8 && y1 + 6 > y2; }

// clang-format off
constexpr std::uint16_t box_sprite[] {
   0x2222, 0x2222,
   0x0002, 0x2000,
   0x0002, 0x2000,
   0x0002, 0x2000,
   0x0002, 0x2000,
   0x0002, 0x2000,
   0x0002, 0x2000,
   0x2222, 0x2222,
};

constexpr std::uint16_t small_box_sprite[] {
   0x2220, 0x0222,
   0x0020, 0x0200,
   0x0020, 0x0200,
   0x0020, 0x0200,
   0x0020, 0x0200,
   0x2220, 0x0222,
   0x0000, 0x0000,
   0x0000, 0x0000,
};
// clang-format on

int main()
{
   const auto deg_to_rad = [](float f) { return f * std::numbers::pi / 180.0; };

   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(gba::bg_opt::char_base_block::b0));
   const auto start_box = gba::dma3_copy(std::begin(snake2), std::end(snake2), gba::base_obj_tile_addr(0));
   const auto start_small = gba::dma3_copy(std::begin(box_sprite), std::end(box_sprite), (std::uint16_t*)start_box);
   gba::dma3_copy(std::begin(small_box_sprite), std::end(small_box_sprite), start_small);
   gba::dma3_copy(std::begin(snake2_pal), std::end(snake2_pal), gba::obj_palette_addr(0));
   const auto start = gba::bg_screen_loc(gba::bg_opt::screen_base_block::b62);
   gba::dma3_fill(start, start + 32 * 32, ' ');

   const auto snake_obj = gba::obj{0};
   const auto box1_obj = gba::obj{1};
   const auto box2_obj = gba::obj{2};
   const auto col_obj = gba::obj{3};
   {
      using namespace gba::obj_opt;
      for (int i = 0; i < 128; ++i) {
         gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(display::disable).set(rot_scale::disable));
      }
      snake_obj.set_attr0(gba::obj_attr0_options{}
                             .set(rot_scale::enable)
                             .set(double_size::disable)
                             .set(mode::normal)
                             .set(mosaic::disable)
                             .set(shape::vertical));
      snake_obj.set_attr1(gba::obj_attr1_options{}.set(size::v8x16));
      snake_obj.set_attr2(gba::obj_attr2_options{}.set(priority::p1));
      for (const auto& box : {box1_obj, box2_obj, col_obj}) {
         box.set_attr0(gba::obj_attr0_options{}.set(display::enable));
         // Hard coded box sprite; normally bad but whatever
         box.set_tile(3);
      }
      col_obj.set_tile(2);
   }

   {
      using namespace gba::lcd_opt;
      gba::lcd.set_options(gba::lcd_options{}
                              .set(bg_mode::mode_0)
                              .set(forced_blank::off)
                              .set(display_bg0::on)
                              .set(display_bg1::off)
                              .set(display_bg2::off)
                              .set(display_bg3::off)
                              .set(display_obj::on)
                              .set(display_window_0::off)
                              .set(display_window_1::off)
                              .set(display_window_obj::off)
                              .set(obj_char_mapping::one_dimensional));
   }

   {
      using namespace gba::bg_opt;
      gba::bg0.set_options(gba::bg_options{}
                              .set(priority::p3)
                              .set(char_base_block::b0)
                              .set(mosaic::disable)
                              .set(colors_palettes::c16_p16)
                              .set(screen_base_block::b62)
                              .set(display_area_overflow::transparent)
                              .set(screen_size::text_256x256));
   }

   const auto write_bg0 = [&](const char* c, int x, int y) { write_at(gba::bg_opt::screen_base_block::b62, c, x, y); };

   gba::keypad_status keypad;

   int angle = 0;
   float x = 0;
   float y = 0;

   // For some reason there's an offset for the x/y when using a sprite under scale/rotate mode
   constexpr auto x_offset = 4;
   constexpr auto y_offset = 8;

   // Where the collision box is
   constexpr auto col_x = 120;
   constexpr auto col_y = 100;
   col_obj.set_loc(col_x, col_y);

   bool visible = true;

   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      const auto start_x = x;
      const auto start_y = y;
      const auto start_angle = angle;

      if (keypad.left_held()) {
         angle -= 1;
      }
      else if (keypad.right_held()) {
         angle += 1;
      }
      if (keypad.l_pressed()) {
         angle -= 90;
      }
      else if (keypad.r_pressed()) {
         angle += 90;
      }
      if (keypad.start_pressed()) {
         angle = 0;
      }
      angle %= 360;
      if (angle < 0) {
         angle = 360 + angle;
      }

      // going forward/backward
      if (keypad.a_held()) {
         x += sin(deg_to_rad(angle));
         y += -cos(deg_to_rad(angle));
      }
      else if (keypad.b_held()) {
         x -= sin(deg_to_rad(angle));
         y -= -cos(deg_to_rad(angle));
      }

      // Toggle hitboxes
      if (keypad.select_pressed()) {
         visible = !visible;
         for (const auto& box : {box1_obj, box2_obj}) {
            using namespace gba::obj_opt;
            box.set_attr0(gba::obj_attr0_options{}.set(display{!visible}));
         }
      }

      // These could probably be better but these work for now
      const auto base_x = [&]() {
         return 2 * sin(deg_to_rad(angle));
         // if (angle == 0) return 0;
         // if (angle == 90) return 2;
         // if (angle == 180) return 0;
         // if (angle == 270) return -2;
         // return 0;
      }();
      const auto base_y = [&]() {
         return 4 * sin(deg_to_rad(angle) / 2);
         // if (angle == 0) return 0;
         // if (angle == 90) return 2;
         // if (angle == 180) return 4;
         // if (angle == 270) return 2;
         // return 0;
      }();
      const auto hitbox1_x = x + x_offset + base_x;
      const auto hitbox1_y = y + y_offset + 2 + base_y;
      const auto hitbox2_x = x + x_offset - base_x;
      const auto hitbox2_y = y + y_offset + 8 - base_y - 2;

      const double data[]{hitbox2_x, hitbox2_y};
      const char* name[]{"x", "y"};
      for (int i = 0; i < 2; ++i) {
         std::array<char, 20> buf;
         buf.fill(' ');
         buf.back() = '\0';
         fmt::format_to_n(std::begin(buf), buf.size() - 1, "{}:{:0.5}", name[i], data[i]);
         write_bg0(buf.data(), 0, 18 + i);
      }

      snake_obj.set_loc(x, y);
      box1_obj.set_loc(hitbox1_x, hitbox1_y);
      box2_obj.set_loc(hitbox2_x, hitbox2_y);

      if (collision(hitbox1_x, hitbox1_y, col_x, col_y) || collision(hitbox2_x, hitbox2_y, col_x, col_y)) {
         x = start_x;
         y = start_y;
         angle = start_angle;
      }

      const std::array<volatile std::uint16_t*, 4> locs{
         (std::uint16_t*)0x07000006,
         (std::uint16_t*)0x0700000E,
         (std::uint16_t*)0x07000016,
         (std::uint16_t*)0x0700001E};

      //   pa = x_scale * cos(angle)
      //   pb = y_scale * sin(angle)
      //   pc = x_scale * -sin(angle)
      //   pd = y_scale * cos(angle)

      std::array<double, 4> test_values{
         std::cos(deg_to_rad(angle)),
         std::sin(deg_to_rad(angle)),
         -std::sin(deg_to_rad(angle)),
         std::cos(deg_to_rad(angle)),
      };

      for (int i = 0; i < 4; ++i) {
         *locs[i] = float_to_fixed(test_values[i]);
      }

      keypad.update();
   }
}
