#ifndef STATIC_VECTOR_HPP
#define STATIC_VECTOR_HPP

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <new>

// Can't use Boost so have to re-implement this
// Since exceptions are disabled for this, we can simplify the implementation somewhat
// This also isn't a 100% drop-in replacement for vector, but it does emulate most of the interface
template<typename T, std::size_t MaxElements>
struct static_vector {
public:
   // Constructors/destructors/assignment

   static_vector() noexcept : num_elements_{0} {}

   static_vector(const static_vector& other) noexcept : num_elements_{other.num_elements_}
   {
      std::copy(other.begin(), other.end(), begin());
   }

   static_vector(static_vector&& other) noexcept : num_elements_{other.num_elements_}
   {
      std::move(other.begin(), other.end(), begin());
      other.num_elements_ = 0;
   }

   static_vector& operator=(const static_vector& other) noexcept
   {
      if (&other == this) {
         return *this;
      }
      clear();
      num_elements_ = other.num_elements_;
      std::copy(other.begin(), other.end(), begin());
      return *this;
   }

   static_vector& operator=(static_vector&& other) noexcept
   {
      if (&other == this) {
         return *this;
      }
      clear();
      num_elements_ = other.num_elements_;
      std::move(other.begin(), other.end(), begin());
      other.num_elements_ = 0;
      return *this;
   }

   ~static_vector() noexcept
      requires(std::is_trivially_destructible_v<T>)
   = default;

   ~static_vector() noexcept
      requires(!std::is_trivially_destructible_v<T>)
   {
      clear();
   }

   // Element access

   T& operator[](std::size_t n) noexcept { return *(begin() + n); }
   const T& operator[](std::size_t n) const noexcept { return *(begin() + n); }

   T& front() noexcept { return *begin(); }
   const T& front() const noexcept { return *begin(); }

   T& back() noexcept { return *(end() - 1); }
   const T& back() const noexcept { return *(end() - 1); }

   T* data() noexcept { return begin(); }
   const T* data() const noexcept { return begin(); }

   // Iterators

   T* begin() noexcept { return std::launder(reinterpret_cast<T*>(storage_.begin())); }
   const T* begin() const noexcept { return std::launder(reinterpret_cast<const T*>(storage_.begin())); }
   const T* cbegin() const noexcept { return begin(); }

   T* end() noexcept { return begin() + num_elements_; }
   const T* end() const noexcept { return begin() + num_elements_; }
   const T* cend() const noexcept { return end(); }

   // Capacity

   bool empty() const noexcept { return num_elements_ == 0; }

   std::size_t size() const noexcept { return num_elements_; }

   std::size_t max_size() const noexcept { return MaxElements; }

   std::size_t capacity() const noexcept { return MaxElements; }

   // Modifiers

   void clear()
   {
      std::destroy(begin(), end());
      num_elements_ = 0;
   }

   void push_back(const T& val) noexcept
   {
      new (begin() + num_elements_) T{val};
      num_elements_ += 1;
   }

   void push_back(T&& val) noexcept
   {
      new (begin() + num_elements_) T{std::move(val)};
      num_elements_ += 1;
   }

   void pop_back() noexcept
   {
      std::destroy_at(&back());
      num_elements_ -= 1;
   }

   T* erase(const T* pos) noexcept { return erase(pos, pos + 1); }
   T* erase(const T* first, const T* last) noexcept
   {
      // convert from const T* to T*
      const auto start = begin() + (first - cbegin());
      const auto num_erased = last - first;
      const auto old_end = end();
      num_elements_ -= num_erased;
      return std::move(start + num_erased, old_end, start);
   }

private:
   alignas(T) std::array<unsigned char, sizeof(T) * MaxElements> storage_;
   std::size_t num_elements_ = 0;
};

#endif // STATIC_VECTOR_HPP
