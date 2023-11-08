// Just a simple huffman decoding speed test

#include "gba.hpp"
#include "huffman_data.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

// Should really make this an iterator but oh well
struct bit_reader {
   bool get_bit() noexcept
   {
      if (num_bits == 0) {
         current = *data;
         ++data;
         num_bits = 8;
      }
      const auto to_output = current & 0b1000'0000;
      current <<= 1;
      num_bits -= 1;
      return to_output;
   }

   std::uint8_t current;
   std::uint_fast8_t num_bits;
   const std::uint8_t* data;
};

void decompress(
   const std::uint16_t* tree, const std::uint8_t* data, std::uint8_t* write_to, std::size_t output_size) noexcept
{
   bit_reader reader{0, 0, data};
   for (std::size_t i = 0; i < output_size; ++i) {
      std::size_t decode_loc = 0;
      while (!(tree[decode_loc] & 0b1000'0000'0000'0000)) {
         if (reader.get_bit()) {
            decode_loc += tree[decode_loc];
         }
         else {
            decode_loc += 1;
         }
      }
      const std::uint8_t to_write = tree[decode_loc] & 0x7FFF;
      *write_to = to_write;
      ++write_to;
   }
}

// This is really bad copy/paste but whatever, this is mainly for testing
void decompress16(
   const std::uint16_t* tree, const std::uint8_t* data, std::uint16_t* write_to, std::size_t output_size) noexcept
{
   bit_reader reader{0, 0, data};
   for (std::size_t i = 0; i < output_size; ++i) {
      std::size_t decode_loc = 0;
      while (!(tree[decode_loc] & 0b1000'0000'0000'0000)) {
         if (reader.get_bit()) {
            decode_loc += tree[decode_loc];
         }
         else {
            decode_loc += 1;
         }
      }
      const std::uint8_t to_write_low = tree[decode_loc] & 0x7FFF;

      decode_loc = 0;
      while (!(tree[decode_loc] & 0b1000'0000'0000'0000)) {
         if (reader.get_bit()) {
            decode_loc += tree[decode_loc];
         }
         else {
            decode_loc += 1;
         }
      }
      const std::uint8_t to_write_high = tree[decode_loc] & 0x7FFF;
      *write_to = (to_write_high << 8) | to_write_low;
      ++write_to;
   }
}

[[gnu::section(".ewram")]] std::array<std::uint32_t, 76800 / 4> image_data;

int main()
{
   gba::set_fast_mode();
   gba::keypad_status keypad;

   {
      using namespace gba::lcd_opt;
      gba::lcd.set_options(gba::lcd_options{}
                              .set(bg_mode::mode_3)
                              .set(forced_blank::off)
                              .set(display_bg0::off)
                              .set(display_bg1::off)
                              .set(display_bg2::on)
                              .set(display_bg3::off)
                              .set(display_obj::on)
                              .set(display_window_0::off)
                              .set(display_window_1::off)
                              .set(display_window_obj::off)
                              .set(obj_char_mapping::one_dimensional));
   }

   while (true) {
      keypad.update();
      if (keypad.b_pressed()) {
         std::ranges::fill(image_data, 0);
         gba::dma3_copy(
            std::begin(image_data), std::end(image_data), gba::bg_screen_loc32(gba::bg_opt::screen_base_block::b0));
      }
      else if (keypad.a_pressed()) {
         decompress(tree, compressed_image, reinterpret_cast<std::uint8_t*>(image_data.data()), sizeof(image_data));
         gba::dma3_copy(
            std::begin(image_data), std::end(image_data), gba::bg_screen_loc32(gba::bg_opt::screen_base_block::b0));
      }
      else if (keypad.l_pressed()) {
         decompress16(
            tree,
            compressed_image,
            const_cast<std::uint16_t*>(gba::bg_screen_loc(gba::bg_opt::screen_base_block::b0)),
            sizeof(image_data) / 2);
      }
   }
}
