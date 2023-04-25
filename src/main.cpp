#include "gba.hpp"

#include "generated/font.hpp"

#include <array>
#include <utility>

int main()
{
   using namespace gba::dma_opt;
   using namespace gba::lcd_opt;
   volatile std::uint16_t* bg_palettes = (std::uint16_t*)0x5000000;
   volatile std::uint32_t* tiles_data = (std::uint32_t*)0x6000000;
   gba::dma3_copy(std::begin(font), std::end(font), tiles_data);
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), bg_palettes);
   // copy the font tiles and palette
   gba::lcd.set_options(gba::lcd_options{}
                           .set(bg_mode{0})
                           .set(forced_blank::off)
                           .set(display_bg1::on)
                           .set(display_obj::off)
                           .set(display_window_0::off)
                           .set(display_window_1::off)
                           .set(display_window_obj::off)
                           .set(obj_char_mapping::one_dimensional));

   const auto bg1_loc = [&](int x, int y) {
      return ((volatile std::uint16_t*)((std::uint8_t*)tiles_data + 0x800 * 31 + x * 2 + y * 0x40));
   };

   // TODO: Document these & make them options in gba.hpp
   //       This is setting video mode options
   *(volatile std::uint16_t*)(0x4000008) = 0b0001'1110'0000'0011;
   *(volatile std::uint16_t*)(0x400000A) = 0b0001'1111'0000'0010;

   gba::dma3_fill(bg1_loc(0, 0), bg1_loc(30, 20), ' ');

   const auto write_it = [&](const char* c, int x, int y) {
      for (int i = 0; c[i] != '\0'; ++i) {
         *bg1_loc(x + i, y) = c[i];
      }
   };

   write_it("The quick brown fox jumped", 0, 0);
   write_it("over the lazy brown dog.", 0, 1);
   write_it("!@#$%^&*()_+", 0, 2);
   write_it("0123456789`'\"~", 0, 3);
   write_it("012345678901234567890123456789", 0, 4);
   int scroll = 0;
   volatile std::uint16_t* bg1_x_scroll_loc = (std::uint16_t*)0x4000014;
   gba::keypad_status keypad;
   while (true) {
      // wait for vblank to end
      while ((*(volatile std::uint16_t*)(0x4000004) & 1)) {}
      // wait for vblank to start
      while (!(*(volatile std::uint16_t*)(0x4000004) & 1)) {}
      keypad.update();
      if (keypad.left_held()) {
         scroll -= 1;
      }
      else if (keypad.right_held()) {
         scroll += 1;
      }
      *bg1_x_scroll_loc = scroll;
   }
}
