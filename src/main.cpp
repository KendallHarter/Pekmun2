#include "gba.hpp"

#include "generated/font.hpp"

#include <array>
#include <utility>

void dma3_copy(const std::uint32_t* src, std::uint32_t* dest, int count)
{
   using namespace gba::dma_opt;
   gba::dma3.set_source(src);
   gba::dma3.set_destination(dest);
   gba::dma3.set_word_count(count);
   gba::dma3.set_options(gba::dma_options{}
                            .set(dest_addr_cntrl::increment)
                            .set(source_addr_cntrl::increment)
                            .set(repeat::off)
                            .set(transfer_type::bits_32)
                            .set(start_timing::immediate)
                            .set(irq::disable)
                            .set(enable::on));
}

int main()
{
   using namespace gba::dma_opt;
   using namespace gba::lcd_opt;
   volatile std::uint16_t* bg_palettes = (std::uint16_t*)0x5000000;
   volatile std::uint32_t* tiles_data = (std::uint32_t*)0x6000000;
   // copy the debug font tiles and palette
   dma3_copy(font, (std::uint32_t*)tiles_data, std::ssize(font));
   // dma3_copy(font_pal, (std::uint32_t)bg_palettes, std::ssize(font_pal));
   // std::copy(std::begin(font), std::end(font), tiles_data);
   std::copy(std::begin(font_pal), std::end(font_pal), bg_palettes);
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
   for (int y = 0; y != 20; ++y) {
      for (int x = 0; x != 30; ++x) {
         *bg1_loc(x, y) = 32;
      }
   }
   const auto write_it = [&](const char* c, int x, int y) {
      for (int i = 0; c[i] != '\0'; ++i) {
         *bg1_loc(x + i, y) = c[i];
      }
   };
   // for (int y = 0; y != 20; ++y) {
   //    for (int x = 0; x != 30; ++x) {
   //       *bg1_loc(x, y) = x + y * 30;
   //    }
   // }
   write_it("The quick brown fox jumped", 0, 0);
   write_it("over the lazy brown dog.", 0, 1);
   write_it("!@#$%^&*()_+", 0, 2);
   write_it("0123456789`'\"~", 0, 3);
   while (true) {
      // Wait for VBlank
      while (*(volatile std::uint16_t*)(0x4000006) < 160) {}
   }
}
