#ifndef __INK19_JSONCPP_HPP__
#define __INK19_JSONCPP_HPP__

#include "jsoncpp_detail.hpp"
#include <boost/json.hpp>
#include <boost/pfr.hpp>
#include <memory>
#include <type_traits>
#include <concepts>
#include <stdexcept>
#include <optional>

namespace bj = boost::json;

namespace jsoncpp {

// 自定义异常类型
class jsoncpp_exception : public std::runtime_error {
public:
    explicit jsoncpp_exception(const std::string& msg) : std::runtime_error(msg) {}
};

class type_conversion_exception : public jsoncpp_exception {
public:
    explicit type_conversion_exception(const std::string& msg) : jsoncpp_exception(msg) {}
};

template <typename T> class transform {
public:
  static void trans(const bj::value &jv, T &t) {
    if (!jv.is_object()) {
      throw type_conversion_exception("Expected JSON object for class type");
    }
    
    bj::object const &jo = jv.as_object();
    boost::pfr::for_each_field(t, [&](auto &&field, auto index) {
      using FieldType = std::decay_t<decltype(field)>;
      std::string_view field_name = boost::pfr::get_name<index, T>();
      if constexpr (detail::HasAliasFieldName<T>::value) {
        field_name = T::__jsoncpp_alias_name(field_name);
      }

      if (jo.contains(field_name)) {
        const bj::value &field_jv = jo.at(field_name);
        try {
          transform<FieldType>::trans(field_jv, field);
        } catch (const std::exception& e) {
          throw type_conversion_exception(std::string("Failed to convert field '") + 
                                         std::string(field_name) + "': " + e.what());
        }
      }
    });
  }

  static bj::value to_json(const T &t) {
    bj::object obj;
    boost::pfr::for_each_field(t, [&](auto &&field, auto index) {
      using FieldType = std::decay_t<decltype(field)>;
      std::string_view field_name = boost::pfr::get_name<index, T>();
      if constexpr (detail::HasAliasFieldName<T>::value) {
        field_name = T::__jsoncpp_alias_name(field_name);
      }
      obj[std::string(field_name)] = transform<FieldType>::to_json(field);
    });
    return obj;
  }
};

template <> class transform<std::string> {
public:
  static void trans(const bj::value &jv, std::string &t) {
    if (jv.is_string()) {
      t = jv.as_string().c_str();
    } else if (jv.is_int64()) {
      t = std::to_string(jv.as_int64());
    } else if (jv.is_uint64()) {
      t = std::to_string(jv.as_uint64());
    } else if (jv.is_double()) {
      t = std::to_string(jv.as_double());
    } else if (jv.is_bool()) {
      t = jv.as_bool() ? "true" : "false";
    } else {
      throw type_conversion_exception("Cannot convert JSON value to string");
    }
  }

  static bj::value to_json(const std::string &t) {
    return bj::string(t);
  }
};

template <typename MV> class transform<std::map<std::string, MV>> {
public:
  static void trans(const bj::value &jv, std::map<std::string, MV> &t) {
    if (!jv.is_object()) {
      throw type_conversion_exception("Expected JSON object for map");
    }
    
    bj::object const &jo = jv.as_object();
    for (auto &[key, value] : jo) {
      MV vt;
      transform<MV>::trans(value, vt);
      t[key] = std::move(vt);
    }
  }

  static bj::value to_json(const std::map<std::string, MV> &t) {
    bj::object obj;
    for (const auto &[key, value] : t) {
      obj[key] = transform<MV>::to_json(value);
    }
    return obj;
  }
};

template <typename T> class transform<std::shared_ptr<T>> {
public:
  static void trans(const bj::value &jv, std::shared_ptr<T> &t) {
    t = std::make_shared<T>();
    transform<T>::trans(jv, *t);
  }

  static bj::value to_json(const std::shared_ptr<T> &t) {
    if (!t) {
      return bj::value{};
    }
    return transform<T>::to_json(*t);
  }
};

template <typename AV> class transform<std::vector<AV>> {
public:
  static void trans(const bj::value &jv, std::vector<AV> &t) {
    if (!jv.is_array()) {
      throw type_conversion_exception("Expected JSON array for vector");
    }
    
    bj::array const &ja = jv.as_array();
    for (auto &value : ja) {
      AV vt;
      transform<AV>::trans(value, vt);
      t.push_back(std::move(vt));
    }
  }

  static bj::value to_json(const std::vector<AV> &t) {
    bj::array arr;
    for (const auto &item : t) {
      arr.push_back(transform<AV>::to_json(item));
    }
    return arr;
  }
};

template <>
class transform<bool> {
public:
  static void trans(const bj::value &jv, bool &t) {
    if (jv.is_string()) {
      std::string fv = jv.as_string().c_str();
      if (!fv.empty()) {
        // 支持多种真值表示
        if (fv == "true" || fv == "1" || fv == "yes" || fv == "on") {
          t = true;
        } else if (fv == "false" || fv == "0" || fv == "no" || fv == "off") {
          t = false;
        } else {
          // 对于其他字符串，尝试转换为数字再判断
          try {
            int num = std::stoi(fv);
            t = num != 0;
          } catch (const std::exception&) {
            throw type_conversion_exception("Invalid boolean string: " + fv);
          }
        }
      } else {
        t = false;
      }
    } else if (jv.is_bool()) {
      t = jv.as_bool();
    } else if (jv.is_int64()) {
      t = jv.as_int64() != 0;
    } else {
      throw type_conversion_exception("Cannot convert JSON value to boolean");
    }
  }

  static bj::value to_json(const bool &t) {
    return t;
  }
};

template <std::integral T>
class transform<T> {
public:
  static void trans(const bj::value &jv, T &t) {
    if (jv.is_string()) {
      std::string fv = jv.as_string().c_str();
      if (!fv.empty()) {
        try {
          t = static_cast<T>(std::stoll(fv));
        } catch (const std::exception&) {
          throw type_conversion_exception("Invalid integer string: " + fv);
        }
      } else {
        t = 0;
      }
    } else if (jv.is_int64()) {
      t = static_cast<T>(jv.as_int64());
    } else if (jv.is_uint64()) {
      t = static_cast<T>(jv.as_uint64());
    } else if (jv.is_double()) {
      t = static_cast<T>(jv.as_double());
    } else {
      throw type_conversion_exception("Cannot convert JSON value to integer");
    }
  }

  static bj::value to_json(const T &t) {
    return static_cast<int64_t>(t);
  }
};

template <std::floating_point T>
class transform<T> {
public:
  static void trans(const bj::value &jv, T &t) {
    if (jv.is_string()) {
      std::string fv = jv.as_string().c_str();
      if (!fv.empty()) {
        try {
          t = static_cast<T>(std::stod(fv));
        } catch (const std::exception&) {
          throw type_conversion_exception("Invalid float string: " + fv);
        }
      } else {
        t = 0.0;
      }
    } else if (jv.is_int64()) {
      t = static_cast<T>(jv.as_int64());
    } else if (jv.is_uint64()) {
      t = static_cast<T>(jv.as_uint64());
    } else if (jv.is_double()) {
      t = static_cast<T>(jv.as_double());
    } else {
      throw type_conversion_exception("Cannot convert JSON value to float");
    }
  }

  static bj::value to_json(const T &t) {
    return static_cast<double>(t);
  }
};

class StringEnum {
public:
  StringEnum(const std::string &value) : value(value) {}

  bool operator==(const StringEnum &other) const {
    return value == other.value;
  }

  bool operator!=(const StringEnum &other) const {
    return value != other.value;
  }

private:
  const std::string value;
};

template <typename T> std::shared_ptr<T> from_json(const std::string &json) {
  try {
    auto jv = bj::parse(json);
    auto t = std::make_shared<T>();
    transform<T>::trans(jv, *t);
    return t;
  } catch (const type_conversion_exception& e) {
    // 直接重新抛出type_conversion_exception
    throw;
  } catch (const std::exception& e) {
    throw jsoncpp_exception(std::string("Failed to parse JSON: ") + e.what());
  }
}

template <typename T> std::string to_json(const T &obj) {
  try {
    bj::value jv = transform<T>::to_json(obj);
    return bj::serialize(jv);
  } catch (const std::exception& e) {
    throw jsoncpp_exception(std::string("Failed to serialize to JSON: ") + e.what());
  }
}

template <typename T> std::string to_json(const std::shared_ptr<T> &obj) {
  if (!obj) {
    return "null";
  }
  return to_json(*obj);
}

}; // namespace jsoncpp

#endif // __INK19_JSONCPP_HPP__
