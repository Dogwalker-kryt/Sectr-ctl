// scf_type_traits.hpp
#pragma once

#include "scf_cutils.hpp"

namespace scf::type_traits {

// --- Base types ---
struct false_type { static constexpr bool value = false; };
struct true_type { static constexpr bool value = true; };

template<bool B>
struct bool_constant : false_type {};
template<>
struct bool_constant<true> : true_type {};

template<bool B>
inline constexpr bool bool_constant_v = bool_constant<B>::value;

// --- remove_reference ---
template<typename T>
struct remove_reference { using type = T; };
template<typename T>
struct remove_reference<T&> { using type = T; };
template<typename T>
struct remove_reference<T&&> { using type = T; };
template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

// --- is_lvalue_reference ---
template<typename T>
struct is_lvalue_reference : false_type {};
template<typename T>
struct is_lvalue_reference<T&> : true_type {};
template<typename T>
inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

// --- is_rvalue_reference ---
template<typename T>
struct is_rvalue_reference : false_type {};
template<typename T>
struct is_rvalue_reference<T&&> : true_type {};
template<typename T>
inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

// --- declval ---
template<typename T>
T&& declval() noexcept;

// --- is_constructible ---
template<typename T, typename... Args>
struct is_constructible {
    template<typename U, typename... Us>
    static auto test(int) -> decltype(U(declval<Us>()...), true_type{});
    template<typename, typename...>
    static false_type test(...);

    static constexpr bool value = decltype(test<T, Args...>(0))::value;
};
template<typename T, typename... Args>
inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

// --- is_nothrow_move_constructible ---
template<typename T>
struct is_nothrow_move_constructible {
    static constexpr bool value = noexcept(T(declval<T&&>()));
};
template<typename T>
inline constexpr bool is_nothrow_move_constructible_v = is_nothrow_move_constructible<T>::value;

// --- is_nothrow_swappable ---
template<typename T>
struct is_nothrow_swappable {
    static constexpr bool value = noexcept(swap(declval<T&>(), declval<T&>()));
};
template<typename T>
inline constexpr bool is_nothrow_swappable_v = is_nothrow_swappable<T>::value;

// --- is_swappable ---
template<typename T>
struct is_swappable {
    static constexpr bool value = noexcept(swap(declval<T&>(), declval<T&>()));
};
template<typename T>inline constexpr bool is_swappable_v = is_swappable<T>::value;

// --- is_reference_v ---
template<typename T>
inline constexpr bool is_reference_v = is_lvalue_reference_v<T> || is_rvalue_reference_v<T>;

// --- is_nothrow_move_assignable_v ---
template<typename T>
struct is_nothrow_move_assignable {
    static constexpr bool value = noexcept(declval<T&>() = declval<T&&>());
};
template<typename T>inline constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<T>::value;

// --- static_assert ---
template<bool B, typename T = void>
struct static_assertion {};
template<typename T>
struct static_assertion<true, T> { using type = T; };
template<bool B, typename T = void>
using static_assertion_t = typename static_assertion<B, T>::type;

template<typename T>
constexpr T&& forward(remove_reference_t<T>& x) noexcept;

template<typename T>
constexpr T&& forward(remove_reference_t<T>&& x) noexcept;

// --- in_place ---
template<typename T, typename... Args>
inline T in_place(Args&&... args) {
    return T(forward<Args>(args)...);
};

// --- initializer_list ---
template<class T>
class initializer_list {
public:
    using value_type      = T;
    using reference       = const T&;
    using const_reference = const T&;
    using size_type       = size_t;
    using iterator        = const T*;
    using const_iterator  = const T*;

    constexpr initializer_list(const T* b, size_t s)
        : begin_(b), size_(s) {}

    constexpr size_t size() const noexcept { return size_; }
    constexpr const T* begin() const noexcept { return begin_; }
    constexpr const T* end() const noexcept { return begin_ + size_; }

private:
    const T* begin_;
    size_t size_;
};

template<typename T, size_t N>
constexpr initializer_list<T> make_initializer_list(const T (&arr)[N]) {
    return initializer_list<T>(arr, N);
};

// --- enable_if_t ---
template<bool B, typename T = void>
struct scf_enable_if_t {};
template<typename T>
struct scf_enable_if_t<true, T> { using type = T; };
template<bool B, typename T = void>using scf_enable_if = typename scf_enable_if_t<B, T>::type;

// --- is_arithmetic_v

// // --- remove_reference_t ---
// template<typename T>
// struct remove_reference {
//     using type = T;
// };

// template<typename T>
// struct remove_reference<T&> {
//     using type = T;
// };

// template<typename T>
// struct remove_reference<T&&> {
//     using type = T;
// };

// --- is_same ---
template<typename A, typename B>
struct is_same : false_type {};

template<typename A>
struct is_same<A, A> : true_type {};

template<typename A, typename B>
inline constexpr bool is_same_v = is_same<A, B>::value;

// --- is_integral ---
template<typename T> struct is_integral : false_type {};

template<> struct is_integral<bool> : true_type {};
template<> struct is_integral<char> : true_type {};
template<> struct is_integral<signed char> : true_type {};
template<> struct is_integral<unsigned char> : true_type {};
template<> struct is_integral<short> : true_type {};
template<> struct is_integral<unsigned short> : true_type {};
template<> struct is_integral<int> : true_type {};
template<> struct is_integral<unsigned int> : true_type {};
template<> struct is_integral<long> : true_type {};
template<> struct is_integral<unsigned long> : true_type {};
template<> struct is_integral<long long> : true_type {};
template<> struct is_integral<unsigned long long> : true_type {};

template<typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

// --- is_floating_point ---
template<typename T> struct is_floating_point : false_type {};
template<> struct is_floating_point<float> : true_type {};
template<> struct is_floating_point<double> : true_type {};
template<> struct is_floating_point<long double> : true_type {};

template<typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

// --- is_signed / is_unsigned ---
template<typename T>
struct is_signed : false_type {};

template<typename T>
inline constexpr bool is_signed_v = (T(-1) < T(0));

template<typename T>
struct is_unsigned : false_type {};

template<typename T>
inline constexpr bool is_unsigned_v = !is_signed_v<T>;

// --- is_pointer ---
template<typename T>
struct is_pointer : false_type {};

template<typename T>
struct is_pointer<T*> : true_type {};

template<typename T>
inline constexpr bool is_pointer_v = is_pointer<T>::value;

// --- is_array ---
template<typename T>
struct is_array : false_type {};

template<typename T>
struct is_array<T[]> : true_type {};

template<typename T, int N>
struct is_array<T[N]> : true_type {};

template<typename T>
inline constexpr bool is_array_v = is_array<T>::value;

// --- is_arithmetic ---
template<typename T>
struct is_arithmetic
    : bool_constant<is_integral_v<T> || is_floating_point_v<T>> {};

template<typename T>
inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

}