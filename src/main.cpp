#include "gba.hpp"

#include <array>

[[gnu::section(".ewram")]] std::array<std::uint32_t, 240 * 160 / 2> buffer;

int main()
{
   using namespace gba::dma_opt;
   using namespace gba::lcd_opt;
   gba::lcd.set_options(gba::lcd_options{}
                           .set(bg_mode{3})
                           .set(forced_blank::off)
                           .set(display_bg2::on)
                           .set(display_obj::on)
                           .set(display_window_0::off)
                           .set(display_window_1::off)
                           .set(display_window_obj::off)
                           .set(obj_char_mapping::one_dimensional));
   std::uint16_t part_color = 0;
   volatile std::uint32_t* vram = (std::uint32_t*)0x6000000;
   gba::dma3.set_source(buffer.data());
   gba::dma3.set_destination((void*)vram);
   gba::dma3.set_word_count(std::size(buffer));
   std::uint16_t color = 0x00000000;

   while (true) {
      // Wait for VBlank
      while (*(volatile std::uint16_t*)(0x4000006) < 160) {}
      for (auto& b : buffer) {
         b = (color << 16) | color;
      }
      // We can't just enable reload because the source continues to increment
      // through each iteration of the DMA if so
      gba::dma3.set_options(gba::dma_options{}
                               .set(dest_addr_cntrl::increment)
                               .set(source_addr_cntrl::increment)
                               .set(repeat::off)
                               .set(transfer_type::bits_32)
                               .set(start_timing::vblank)
                               .set(irq::disable)
                               .set(enable::on));
      part_color += 1;
      part_color %= 16;
      color = (part_color << 11) | (part_color << 6) | (part_color << 1);
   }
}
