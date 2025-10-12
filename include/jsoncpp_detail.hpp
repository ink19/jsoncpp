#ifndef JSONCPP_DETAIL_HPP
#define JSONCPP_DETAIL_HPP

#include <vector>
#include <type_traits>
#include <memory>
#include <map>

namespace jsoncpp::detail {
// 基础模板：默认非 vector 类型
template <typename T> struct is_vector : std::false_type {};

// 偏特化：匹配所有 std::vector<T, Alloc> 类型
template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

template <typename _Tp>
inline constexpr bool is_vector_v = is_vector<_Tp>::value;


template <typename T> struct is_map : std::false_type {};

template <typename Key, typename T, typename Compare, typename Allocator>
struct is_map<typename std::map<Key, T, Compare, Allocator>> : std::true_type {};

template <typename _Tp>
inline constexpr bool is_map_v = is_map<_Tp>::value;

template <typename T> struct is_shared_ptr : std::false_type {};

// 特化模板（匹配 std::shared_ptr<T>）
template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template <typename _Tp>
inline constexpr bool is_shared_v = is_shared_ptr<_Tp>::value;

template <typename T> struct remove_shared {
  using type = T;
};

template <typename T> struct remove_shared<std::shared_ptr<T>> {
  using type = T;
};

template <typename T> using remove_shared_t = typename remove_shared<T>::type;

// 使用std::void_t的简化版本
template<typename T, typename = void>
struct HasAliasFieldName : std::false_type {};

template<typename T>
struct HasAliasFieldName<T, std::void_t<decltype(T::__jsoncpp_alias_name)>> : std::true_type {};


} // namespace jsoncpp::detail

#endif // JSONCPP_DETAIL_HPP
