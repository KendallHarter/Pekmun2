#include "gba.hpp"

#include "generated/snake.hpp"

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

int main()
{
   const auto deg_to_rad = [](float f) { return f * std::numbers::pi / 180.0; };

   gba::dma3_copy(std::begin(snake), std::end(snake), gba::base_obj_tile_addr(0));
   gba::dma3_copy(std::begin(snake_pal), std::end(snake_pal), gba::obj_palette_addr(0));
   *gba::bg_palette_addr(0) = gba::make_gba_color(0x00, 0xA0, 0xA0);

   const auto snake_obj = gba::obj{0};
   {
      using namespace gba::obj_opt;
      for (int i = 0; i < 128; ++i) {
         gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(display::disable));
      }
      snake_obj.set_attr0(gba::obj_attr0_options{}
                             .set(rot_scale::enable)
                             .set(double_size::disable)
                             .set(mode::normal)
                             .set(mosaic::disable)
                             .set(shape::vertical));
      snake_obj.set_attr1(gba::obj_attr1_options{}.set(size::v16x32));
   }

   {
      using namespace gba::lcd_opt;
      gba::lcd.set_options(gba::lcd_options{}
                              .set(bg_mode::mode_0)
                              .set(forced_blank::off)
                              .set(display_bg0::off)
                              .set(display_bg1::off)
                              .set(display_bg2::off)
                              .set(display_bg3::off)
                              .set(display_obj::on)
                              .set(display_window_0::off)
                              .set(display_window_1::off)
                              .set(display_window_obj::off)
                              .set(obj_char_mapping::one_dimensional));
   }

   gba::keypad_status keypad;

   int angle = 0;
   float x = 0;
   float y = 0;

   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      if (keypad.left_held()) {
         angle -= 1;
      }
      else if (keypad.right_held()) {
         angle += 1;
      }
      angle %= 360;

      // going forward
      if (keypad.a_held()) {
         x += sin(deg_to_rad(angle));
         y += -cos(deg_to_rad(angle));
      }

      snake_obj.set_loc(x, y);

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
