#ifndef GBA_HPP
#define GBA_HPP

#include <cstdint>

#ifdef NDEBUG
   #define GBA_ASSERT(cond) (void)(cond)
#else
   // TODO: Improve this; currently just hangs if condition is true
   #define GBA_ASSERT(cond) \
      do {                  \
         while (!(cond)) {} \
      } while (0)
#endif

namespace gba {

inline constexpr bool is_internal_memory(std::uintptr_t loc) noexcept { return loc < 0x0800'0000; }

inline constexpr bool is_external_memory(std::uintptr_t loc) noexcept
{
   return loc >= 0x0800'0000 && loc < 0x1000'0000;
}

inline constexpr std::uintptr_t ewram_start{0x200'0000};
inline constexpr std::uintptr_t ewram_end{ewram_start + 0x40000};

inline constexpr std::uintptr_t iwram_start{0x400'0000};
inline constexpr std::uintptr_t iwram_end{iwram_start + 0x8000};

inline constexpr std::uintptr_t game_pack_start{0x800'0000};
inline constexpr std::uintptr_t game_pack_end{game_pack_start + 0x600'0000};

namespace lcd_opt {

enum class bg_mode {
   mode_0,
   mode_1,
   mode_2,
   mode_3,
   mode_4,
   mode_5
};

enum class frame_select {
   frame_0,
   frame_1
};

enum class oam_access_during_hblank {
   disabled,
   enabled
};

enum class obj_char_mapping {
   two_dimensional,
   one_dimensional
};

enum class forced_blank {
   off,
   on
};

enum class display_bg0 {
   off,
   on
};

enum class display_bg1 {
   off,
   on
};

enum class display_bg2 {
   off,
   on
};

enum class display_bg3 {
   off,
   on
};

enum class display_obj {
   off,
   on
};

enum class display_window_0 {
   off,
   on
};

enum class display_window_1 {
   off,
   on
};

enum class display_window_obj {
   off,
   on
};

} // namespace lcd_opt

namespace dma_opt {

enum class dest_addr_cntrl {
   increment,
   decrement,
   fixed,
   inc_reload
};

enum class source_addr_cntrl {
   increment,
   decrement,
   fixed
};

enum class repeat {
   off,
   on
};

enum class transfer_type {
   bits_16,
   bits_32
};

// This can't be used to access SRAM so we'll never actually use it
// enum class to_game_pak {
//    no = 0,
//    yes = 1
// };

enum class start_timing {
   immediate,
   vblank,
   hblank,
   special
};

enum class irq {
   disable,
   enable
};

enum class enable {
   off,
   on
};

} // namespace dma_opt

namespace bg_opt {

enum class priority {
   p0,
   p1,
   p2,
   p3
};

/// @brief This is the offset to BG tile data
enum class char_base_block {
   b0,
   b16,
   b32,
   b48
};

enum class mosaic {
   disable,
   enable
};

enum class colors_palettes {
   c16_p16,
   c256_p1
};

// TODO: Is there a better way of representing this?
/// @brief This is the offset to BG map data
enum class screen_base_block {
   b0,
   b2,
   b4,
   b6,
   b8,
   b10,
   b12,
   b14,
   b16,
   b18,
   b20,
   b22,
   b24,
   b26,
   b28,
   b30,
   b32,
   b34,
   b36,
   b38,
   b40,
   b42,
   b44,
   b46,
   b48,
   b50,
   b52,
   b54,
   b56,
   b58,
   b60,
   b62
};

enum class display_area_overflow {
   transparent,
   wraparound
};

enum class screen_size {
   text_256x256,
   text_512x256,
   text_256x512,
   text_512x512,

   rot_scale_128x128 = 0,
   rot_scale_256x256 = 1,
   rot_scale_512x512 = 2,
   rot_scale_1024x1024 = 3
};

} // namespace bg_opt

namespace detail {
struct dma_builder {};
} // namespace detail

#define MAKE_SET(type, shift)                                 \
   constexpr self& set(MAKE_SET_NAMESPACE::type val) noexcept \
   {                                                          \
      set_field(shift, static_cast<int>(val) & 1);            \
      return *this;                                           \
   }

#define MAKE_SET2(type, shift)                                \
   constexpr self& set(MAKE_SET_NAMESPACE::type val) noexcept \
   {                                                          \
      set_field(shift, static_cast<int>(val) & 1);            \
      set_field(shift + 1, static_cast<int>(val) >> 1);       \
      return *this;                                           \
   }

#define MAKE_SET3(type, shift)                                \
   constexpr self& set(MAKE_SET_NAMESPACE::type val) noexcept \
   {                                                          \
      set_field(shift, static_cast<int>(val) & 1);            \
      set_field(shift + 1, (static_cast<int>(val) >> 1) & 1); \
      set_field(shift + 2, (static_cast<int>(val) >> 2) & 1); \
      return *this;                                           \
   }

#define MAKE_SET5(type, shift)                                \
   constexpr self& set(MAKE_SET_NAMESPACE::type val) noexcept \
   {                                                          \
      set_field(shift, static_cast<int>(val) & 1);            \
      set_field(shift, (static_cast<int>(val) >> 1) & 1);     \
      set_field(shift, (static_cast<int>(val) >> 2) & 2);     \
      set_field(shift, (static_cast<int>(val) >> 3) & 3);     \
      set_field(shift, (static_cast<int>(val) >> 4) & 4);     \
      return *this;                                           \
   }

#define MAKE_SET_NAMESPACE lcd_opt

struct lcd_options {
   using self = lcd_options;

   MAKE_SET3(bg_mode, 0)
   MAKE_SET(frame_select, 4)
   MAKE_SET(oam_access_during_hblank, 5)
   MAKE_SET(obj_char_mapping, 6)
   MAKE_SET(forced_blank, 7)
   MAKE_SET(display_bg0, 8)
   MAKE_SET(display_bg1, 9)
   MAKE_SET(display_bg2, 10)
   MAKE_SET(display_bg3, 11)
   MAKE_SET(display_obj, 12)
   MAKE_SET(display_window_0, 13)
   MAKE_SET(display_window_1, 14)
   MAKE_SET(display_window_obj, 15)

   constexpr void set_field(int shift_amount, int value) noexcept
   {
      const std::uint16_t base_value = value << shift_amount;
      and_mask &= base_value;
      or_mask |= base_value;
   }

   std::uint16_t and_mask{0xFFFF};
   std::uint16_t or_mask{0x0000};
};

#undef MAKE_SET_NAMESPACE

#define MAKE_SET_NAMESPACE dma_opt

struct dma_options {
   using self = dma_options;

   MAKE_SET2(dest_addr_cntrl, 5)
   MAKE_SET2(source_addr_cntrl, 7)
   MAKE_SET(repeat, 9)
   MAKE_SET(transfer_type, 10)
   MAKE_SET2(start_timing, 12)
   MAKE_SET(irq, 14)
   MAKE_SET(enable, 15)

   constexpr void set_field(int shift_amount, int value) noexcept
   {
      const std::uint16_t base_value = value << shift_amount;
      and_mask &= base_value;
      or_mask |= base_value;
   }

   // The left-out bit is the to game pak option; which has no use to me
   std::uint16_t and_mask{0b1111'0111'1111'1111};
   std::uint16_t or_mask{0x0000};
};

#undef MAKE_SET_NAMESPACE

#define MAKE_SET_NAMESPACE bg_opt

struct bg_options {
   using self = bg_options;

   MAKE_SET2(priority, 0)
   MAKE_SET2(char_base_block, 2)
   MAKE_SET(mosaic, 6)
   MAKE_SET(colors_palettes, 7)
   MAKE_SET5(screen_base_block, 8)
   MAKE_SET(display_area_overflow, 13)
   MAKE_SET2(screen_size, 14)

   constexpr void set_field(int shift_amount, int value) noexcept
   {
      const std::uint16_t base_value = value << shift_amount;
      and_mask &= base_value;
      or_mask |= base_value;
   }

   // Left out bits are "must be 0"
   std::uint16_t and_mask{0b1111'1111'1100'1111};
   std::uint16_t or_mask{0x0000};
};

#undef MAKE_SET_NAMESPACE

#undef MAKE_SET
#undef MAKE_SET2
#undef MAKE_SET3
#undef MAKE_SET5

namespace detail {
struct preserve {};
} // namespace detail

inline constexpr detail::preserve preserve;

struct dma {
   constexpr dma(int num, std::uintptr_t base_addr, detail::dma_builder) noexcept
      : num{num}
      , source_addr_raw{base_addr}
      , dest_addr_raw{base_addr + 4}
      , word_count_raw{base_addr + 8}
      , control_raw{base_addr + 10}
   {}

   void set_options(detail::preserve, dma_options opt) const noexcept
   {
      auto to_set = *control();
      to_set &= opt.and_mask;
      to_set |= opt.or_mask;
      // Do some error checking
      if (num == 0) {
         const auto start_timing = (to_set & 0b0011'0000'0000'0000) >> 12;
         GBA_ASSERT(start_timing != 3);
      }
      *control() = to_set;
   }

   void set_options(dma_options opt) const noexcept
   {
      // This _shouldn't_ be needed, but apparently it is for some reason.
      // Removing the dummy read causes this to not work, whereas it works
      // just fine with it there.  I'm completely lost on this.
      const auto dummy = *control();
      (void)dummy;
      *control() = opt.or_mask;
   }

   void set_source(volatile const void* ptr) const noexcept
   {
      const auto value = reinterpret_cast<std::uint32_t>(ptr);
      if (num == 0) {
         GBA_ASSERT(is_internal_memory(value));
      }
      *source_addr() = value;
   }

   void set_source(const void* ptr) const noexcept { set_source(const_cast<volatile const void*>(ptr)); }

   void set_destination(volatile void* ptr) const noexcept
   {
      const auto value = reinterpret_cast<std::uint32_t>(ptr);
      if (num != 3) {
         GBA_ASSERT(is_internal_memory(value));
      }
      *dest_addr() = value;
   }

   void set_destination(void* ptr) const noexcept { set_destination(const_cast<volatile void*>(ptr)); }

   void set_word_count(int num_words) const noexcept
   {
      if (num != 3) {
         GBA_ASSERT(num_words <= 0x4000);
      }
      else {
         GBA_ASSERT(num_words <= 0x10000);
      }
      *word_count() = num_words;
   }

private:
   volatile std::uint32_t* source_addr() const noexcept { return reinterpret_cast<std::uint32_t*>(source_addr_raw); }
   volatile std::uint32_t* dest_addr() const noexcept { return reinterpret_cast<std::uint32_t*>(dest_addr_raw); }
   volatile std::uint16_t* word_count() const noexcept { return reinterpret_cast<std::uint16_t*>(word_count_raw); }
   volatile std::uint16_t* control() const noexcept { return reinterpret_cast<std::uint16_t*>(control_raw); }

   int num;
   std::uintptr_t source_addr_raw;
   std::uintptr_t dest_addr_raw;
   std::uintptr_t word_count_raw;
   std::uintptr_t control_raw;
};

constexpr dma dma0{0, 0x40000B0, detail::dma_builder{}};
constexpr dma dma1{1, 0x40000BC, detail::dma_builder{}};
constexpr dma dma2{2, 0x40000C8, detail::dma_builder{}};
constexpr dma dma3{3, 0x40000D4, detail::dma_builder{}};

namespace detail {

struct lcd {
   void set_options(detail::preserve, lcd_options opt) const noexcept
   {
      volatile auto* control = reinterpret_cast<std::uint16_t*>(0x400'0000);
      auto to_set = *control;
      to_set &= opt.and_mask;
      to_set |= opt.or_mask;
      *control = to_set;
   }

   void set_options(lcd_options opt) const noexcept
   {
      volatile auto* control = reinterpret_cast<std::uint16_t*>(0x400'0000);
      *control = opt.or_mask;
   }
};

} // namespace detail

constexpr detail::lcd lcd;

namespace detail {

struct bg_builder {};

} // namespace detail

struct bg {
public:
   constexpr bg(int num, detail::bg_builder) noexcept : num{num} {}

   void set_options(detail::preserve, bg_options opt) const noexcept
   {
      auto to_set = *control_addr();
      to_set &= opt.and_mask;
      to_set |= opt.or_mask;
      *control_addr() = to_set;
   }

   void set_options(bg_options opt) const noexcept { *control_addr() = opt.or_mask; }

   void set_x_scroll(int x) const noexcept { *x_scroll_addr() = x; }

   void set_y_scroll(int y) const noexcept { *y_scroll_addr() = y; }

   void set_scroll(int x, int y) const noexcept
   {
      set_x_scroll(x);
      set_y_scroll(y);
   }

private:
   volatile std::uint16_t* control_addr() const noexcept
   {
      return reinterpret_cast<std::uint16_t*>(0x0400'0008 + num * 2);
   }
   volatile std::uint16_t* x_scroll_addr() const noexcept
   {
      return reinterpret_cast<std::uint16_t*>(0x0400'0010 + num * 4);
   }
   volatile std::uint16_t* y_scroll_addr() const noexcept
   {
      return reinterpret_cast<std::uint16_t*>(0x0400'0012 + num * 4);
   }

   int num;
};

inline constexpr bg bg0{0, detail::bg_builder{}};
inline constexpr bg bg1{1, detail::bg_builder{}};
inline constexpr bg bg2{2, detail::bg_builder{}};
inline constexpr bg bg3{3, detail::bg_builder{}};

inline constexpr volatile std::uint32_t* bg_char_loc(bg_opt::char_base_block opt) noexcept
{
   return reinterpret_cast<volatile std::uint32_t*>(0x600'000 + 0x4000 * static_cast<int>(opt));
}

inline constexpr volatile std::uint16_t* bg_screen_loc(bg_opt::screen_base_block opt) noexcept
{
   return reinterpret_cast<volatile std::uint16_t*>(0x600'000 + 0x800 * static_cast<int>(opt));
}

inline constexpr volatile std::uint16_t* bg_palette_addr(int num) noexcept
{
   return reinterpret_cast<volatile std::uint16_t*>(0x500'0000 + 32 * num);
}

inline constexpr volatile std::uint16_t* obj_palette_addr(int num) noexcept
{
   return reinterpret_cast<volatile std::uint16_t*>(0x500'0200 + 32 * num);
}

inline constexpr volatile std::uint32_t* obj_tile_addr(int bg_mode) noexcept
{
   if (bg_mode == 0 || bg_mode == 1 || bg_mode == 2) {
      return reinterpret_cast<volatile std::uint32_t*>(0x601'0000);
   }
   return reinterpret_cast<volatile std::uint32_t*>(0x601'4000);
}

inline volatile std::uint32_t*
   dma3_copy(const std::uint32_t* start, const std::uint32_t* end, volatile std::uint32_t* dest) noexcept
{
   using namespace gba::dma_opt;
   gba::dma3.set_source(start);
   gba::dma3.set_destination(dest);
   gba::dma3.set_word_count(end - start);
   gba::dma3.set_options(gba::dma_options{}
                            .set(source_addr_cntrl::increment)
                            .set(dest_addr_cntrl::increment)
                            .set(repeat::off)
                            .set(transfer_type::bits_32)
                            .set(start_timing::immediate)
                            .set(irq::disable)
                            .set(enable::on));
   return dest + (end - start);
}

inline std::uint32_t* dma3_copy(const std::uint32_t* start, const std::uint32_t* end, std::uint32_t* dest) noexcept
{
   return const_cast<std::uint32_t*>(dma3_copy(start, end, const_cast<volatile std::uint32_t*>(dest)));
}

inline volatile std::uint16_t*
   dma3_copy(const std::uint16_t* start, const std::uint16_t* end, volatile std::uint16_t* dest) noexcept
{
   using namespace gba::dma_opt;
   gba::dma3.set_source(start);
   gba::dma3.set_destination(dest);
   gba::dma3.set_word_count(end - start);
   gba::dma3.set_options(gba::dma_options{}
                            .set(source_addr_cntrl::increment)
                            .set(dest_addr_cntrl::increment)
                            .set(repeat::off)
                            .set(transfer_type::bits_16)
                            .set(start_timing::immediate)
                            .set(irq::disable)
                            .set(enable::on));
   return dest + (end - start);
}

inline std::uint16_t* dma3_copy(const std::uint16_t* start, const std::uint16_t* end, std::uint16_t* dest) noexcept
{
   return const_cast<std::uint16_t*>(dma3_copy(start, end, const_cast<volatile std::uint16_t*>(dest)));
}

inline void dma3_fill(volatile std::uint32_t* start, volatile std::uint32_t* end, std::uint32_t value) noexcept
{
   using namespace gba::dma_opt;
   gba::dma3.set_source(&value);
   gba::dma3.set_destination(start);
   gba::dma3.set_word_count(end - start);
   gba::dma3.set_options(gba::dma_options{}
                            .set(source_addr_cntrl::fixed)
                            .set(dest_addr_cntrl::increment)
                            .set(repeat::off)
                            .set(transfer_type::bits_32)
                            .set(start_timing::immediate)
                            .set(irq::disable)
                            .set(enable::on));
}

inline void dma3_fill(std::uint32_t* start, std::uint32_t* end, std::uint32_t value) noexcept
{
   dma3_fill(const_cast<volatile std::uint32_t*>(start), const_cast<volatile std::uint32_t*>(end), value);
}

inline void dma3_fill(volatile std::uint16_t* start, volatile std::uint16_t* end, std::uint16_t value) noexcept
{
   using namespace gba::dma_opt;
   gba::dma3.set_source(&value);
   gba::dma3.set_destination(start);
   gba::dma3.set_word_count(end - start);
   gba::dma3.set_options(gba::dma_options{}
                            .set(source_addr_cntrl::fixed)
                            .set(dest_addr_cntrl::increment)
                            .set(repeat::off)
                            .set(transfer_type::bits_16)
                            .set(start_timing::immediate)
                            .set(irq::disable)
                            .set(enable::on));
}

inline void dma3_fill(std::uint16_t* start, std::uint16_t* end, std::uint16_t value) noexcept
{
   dma3_fill(const_cast<volatile std::uint16_t*>(start), const_cast<volatile std::uint16_t*>(end), value);
}

struct keypad_status {
   void update()
   {
      raw_val_prev = raw_val;
      raw_val = *(volatile std::uint16_t*)(0x4000130);
   }

   // TODO: Better name?
   // Pressed series only returns true if the key is newly pressed
   bool a_pressed() const { return pressed_impl(0); }
   bool b_pressed() const { return pressed_impl(1); }
   bool select_pressed() const { return pressed_impl(2); }
   bool start_pressed() const { return pressed_impl(3); }
   bool right_pressed() const { return pressed_impl(4); }
   bool left_pressed() const { return pressed_impl(5); }
   bool up_pressed() const { return pressed_impl(6); }
   bool down_pressed() const { return pressed_impl(7); }
   bool r_pressed() const { return pressed_impl(8); }
   bool l_pressed() const { return pressed_impl(9); }

   bool a_held() const { return held_impl(0); }
   bool b_held() const { return held_impl(1); }
   bool select_held() const { return held_impl(2); }
   bool start_held() const { return held_impl(3); }
   bool right_held() const { return held_impl(4); }
   bool left_held() const { return held_impl(5); }
   bool up_held() const { return held_impl(6); }
   bool down_held() const { return held_impl(7); }
   bool r_held() const { return held_impl(8); }
   bool l_held() const { return held_impl(9); }

private:
   bool pressed_impl(int bit_no) const
   {
      const std::uint16_t mask = 1 << bit_no;
      // if previously not pressed but now pressed
      return (((raw_val_prev & mask) != 0)) && ((raw_val & mask) == 0);
   }

   bool held_impl(int bit_no) const
   {
      const std::uint16_t mask = 1 << bit_no;
      return (raw_val_prev & mask) == 0;
   }

   std::uint16_t raw_val_prev{0xFFFF};
   std::uint16_t raw_val{0xFFFF};
};

} // namespace gba

#endif
