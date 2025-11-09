#include "jsoncpp.hpp"
#include <gtest/gtest.h>

class ext_data {
public:
    int data;
};

class main_data {
public:
    int a;
    std::string b;
    std::vector<int> c;
    bool d;
    std::map<std::string, int> e;
    ext_data f;
    
    constexpr static std::string_view __jsoncpp_alias_name(const std::string_view& name) {
        if (name == "f") {
            return "alias_f";
        }
        return name;
    }
};

// 为嵌套对象测试定义类
class nested_data {
public:
    int nested_int;
    std::string nested_str;
};

class container_data {
public:
    nested_data nested;
    std::vector<nested_data> nested_list;
};

// 为嵌套类注册转换器（移到全局作用域）
namespace jsoncpp {
    template<>
    struct transform<nested_data> {
        static void trans(const bj::value &jv, nested_data &t) {
            bj::object const &jo = jv.as_object();
            if (jo.contains("nested_int")) {
                t.nested_int = jo.at("nested_int").as_int64();
            }
            if (jo.contains("nested_str")) {
                t.nested_str = jo.at("nested_str").as_string().c_str();
            }
        }

        static bj::value to_json(const nested_data &t) {
            bj::object obj;
            obj["nested_int"] = t.nested_int;
            obj["nested_str"] = t.nested_str;
            return obj;
        }
    };
}

namespace jsoncpp {

template<>
struct transform<ext_data> {
    static void trans(const bj::value &jv, ext_data &t) {
        t.data = 1024;
    }

    static bj::value to_json(const ext_data &t) {
        bj::object obj;
        obj["data"] = t.data;
        return obj;
    }
};

}

TEST(JsonCppTest, Test1) {
    std::string json_str = "{\"a\":1, \"b\":\"hello\", \"c\":[1, \"2\", 3], \"d\": true, \"e\": {\"a\": 1, \"b\": \"2\"}, \"alias_f\": \"1.23456789012345678901234567890\"}";
    auto test = jsoncpp::from_json<main_data>(json_str);

    EXPECT_EQ(test->a, 1);
    EXPECT_EQ(test->b, "hello");
    EXPECT_EQ(test->c.size(), 3);
    EXPECT_EQ(test->c[0], 1);
    EXPECT_EQ(test->c[1], 2);
    EXPECT_EQ(test->c[2], 3);
    EXPECT_EQ(test->d, true);
    EXPECT_EQ(test->e.size(), 2);
    EXPECT_EQ(test->e["a"], 1);
    EXPECT_EQ(test->e["b"], 2);
    EXPECT_EQ(test->f.data, 1024);
}

TEST(JsonCppTest, BasicTypesTest) {
    // 测试基本类型转换
    std::string json_str = "{\"a\":42, \"b\":\"world\", \"d\":false}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    
    EXPECT_EQ(test->a, 42);
    EXPECT_EQ(test->b, "world");
    EXPECT_EQ(test->d, false);
    EXPECT_TRUE(test->c.empty());
    EXPECT_TRUE(test->e.empty());
}

TEST(JsonCppTest, EmptyContainersTest) {
    // 测试空容器
    std::string json_str = "{\"a\":0, \"b\":\"\", \"c\":[], \"d\":false, \"e\":{}, \"alias_f\":\"0\"}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    
    EXPECT_EQ(test->a, 0);
    EXPECT_EQ(test->b, "");
    EXPECT_TRUE(test->c.empty());
    EXPECT_FALSE(test->d);
    EXPECT_TRUE(test->e.empty());
}

TEST(JsonCppTest, ComplexVectorTest) {
    // 测试复杂向量
    std::string json_str = "{\"c\":[10, \"20\", 30, \"40\", 50]}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    
    EXPECT_EQ(test->c.size(), 5);
    EXPECT_EQ(test->c[0], 10);
    EXPECT_EQ(test->c[1], 20);
    EXPECT_EQ(test->c[2], 30);
    EXPECT_EQ(test->c[3], 40);
    EXPECT_EQ(test->c[4], 50);
}

TEST(JsonCppTest, ComplexMapTest) {
    // 测试复杂映射
    std::string json_str = "{\"e\":{\"key1\":1, \"key2\":\"2\", \"key3\":3, \"key4\":\"4\"}}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    
    EXPECT_EQ(test->e.size(), 4);
    EXPECT_EQ(test->e["key1"], 1);
    EXPECT_EQ(test->e["key2"], 2);
    EXPECT_EQ(test->e["key3"], 3);
    EXPECT_EQ(test->e["key4"], 4);
}

TEST(JsonCppTest, FieldAliasTest) {
    // 测试字段别名功能
    std::string json_str = "{\"alias_f\":\"999\"}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    
    EXPECT_EQ(test->f.data, 1024); // 自定义转换器固定返回1024
}

TEST(JsonCppTest, PartialDataTest) {
    // 测试部分数据缺失的情况
    std::string json_str = "{\"a\":100}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    
    EXPECT_EQ(test->a, 100);
    EXPECT_EQ(test->b, ""); // 默认值
    EXPECT_TRUE(test->c.empty()); // 默认值
    EXPECT_FALSE(test->d); // 默认值
    EXPECT_TRUE(test->e.empty()); // 默认值
}

TEST(JsonCppTest, BooleanFromStringTest) {
    // 测试字符串到布尔值的转换
    std::string json_str = "{\"d\":\"true\"}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    EXPECT_TRUE(test->d);
    
    json_str = "{\"d\":\"false\"}";
    test = jsoncpp::from_json<main_data>(json_str);
    EXPECT_FALSE(test->d);
    
    json_str = "{\"d\":\"1\"}";
    test = jsoncpp::from_json<main_data>(json_str);
    EXPECT_TRUE(test->d);
}

TEST(JsonCppTest, IntegerFromStringTest) {
    // 测试字符串到整数的转换
    std::string json_str = "{\"a\":\"123\"}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    EXPECT_EQ(test->a, 123);
}

// 错误处理测试
TEST(JsonCppTest, InvalidJsonTest) {
    // 测试无效JSON
    std::string json_str = "invalid json string";
    EXPECT_THROW(jsoncpp::from_json<main_data>(json_str), boost::system::system_error);
}

TEST(JsonCppTest, EmptyJsonTest) {
    // 测试空JSON
    std::string json_str = "{}";
    auto test = jsoncpp::from_json<main_data>(json_str);
    
    EXPECT_EQ(test->a, 0);
    EXPECT_EQ(test->b, "");
    EXPECT_TRUE(test->c.empty());
    EXPECT_FALSE(test->d);
    EXPECT_TRUE(test->e.empty());
}

TEST(JsonCppTest, NestedObjectTest) {
    std::string json_str = "{\"nested\":{\"nested_int\":999, \"nested_str\":\"nested_value\"}, \"nested_list\":[{\"nested_int\":1, \"nested_str\":\"first\"}, {\"nested_int\":2, \"nested_str\":\"second\"}]}";
    auto test = jsoncpp::from_json<container_data>(json_str);
    
    EXPECT_EQ(test->nested.nested_int, 999);
    EXPECT_EQ(test->nested.nested_str, "nested_value");
    EXPECT_EQ(test->nested_list.size(), 2);
    EXPECT_EQ(test->nested_list[0].nested_int, 1);
    EXPECT_EQ(test->nested_list[0].nested_str, "first");
    EXPECT_EQ(test->nested_list[1].nested_int, 2);
    EXPECT_EQ(test->nested_list[1].nested_str, "second");
}

// 新增：to_json功能测试
TEST(JsonCppTest, ToJsonBasicTest) {
    main_data data;
    data.a = 42;
    data.b = "hello";
    data.c = {1, 2, 3};
    data.d = true;
    data.e = {{"key1", 1}, {"key2", 2}};
    data.f.data = 1024;
    
    std::string json_str = jsoncpp::to_json(data);
    EXPECT_FALSE(json_str.empty());
    
    // 验证序列化后的JSON可以正确反序列化
    auto parsed = jsoncpp::from_json<main_data>(json_str);
    EXPECT_EQ(parsed->a, 42);
    EXPECT_EQ(parsed->b, "hello");
    EXPECT_EQ(parsed->c.size(), 3);
    EXPECT_EQ(parsed->d, true);
    EXPECT_EQ(parsed->e.size(), 2);
}

TEST(JsonCppTest, ToJsonWithAliasTest) {
    main_data data;
    data.a = 100;
    data.f.data = 2048;
    
    std::string json_str = jsoncpp::to_json(data);
    // 检查别名字段是否正确序列化
    EXPECT_TRUE(json_str.find("alias_f") != std::string::npos);
}

TEST(JsonCppTest, ToJsonEmptyTest) {
    // 使用聚合初始化语法正确初始化所有字段
    main_data data = {0, "", {}, false, {}, {}};
    std::string json_str = jsoncpp::to_json(data);
    
    auto parsed = jsoncpp::from_json<main_data>(json_str);
    EXPECT_EQ(parsed->a, 0);
    EXPECT_EQ(parsed->b, "");
    EXPECT_TRUE(parsed->c.empty());
    EXPECT_FALSE(parsed->d);
    EXPECT_TRUE(parsed->e.empty());
}

TEST(JsonCppTest, RoundTripTest) {
    // 测试往返序列化：序列化后反序列化应该得到相同结果
    std::string original_json = "{\"a\":999, \"b\":\"roundtrip\", \"c\":[5,6,7], \"d\":false, \"e\":{\"x\":10, \"y\":20}, \"alias_f\":\"test\"}";
    
    auto parsed = jsoncpp::from_json<main_data>(original_json);
    std::string serialized_json = jsoncpp::to_json(*parsed);
    auto reparsed = jsoncpp::from_json<main_data>(serialized_json);
    
    EXPECT_EQ(reparsed->a, 999);
    EXPECT_EQ(reparsed->b, "roundtrip");
    EXPECT_EQ(reparsed->c.size(), 3);
    EXPECT_EQ(reparsed->d, false);
    EXPECT_EQ(reparsed->e.size(), 2);
}

TEST(JsonCppTest, SharedPtrTest) {
    // 测试shared_ptr支持
    auto ptr = std::make_shared<main_data>();
    ptr->a = 123;
    ptr->b = "shared_ptr_test";
    
    std::string json_str = jsoncpp::to_json(ptr);
    auto parsed_ptr = jsoncpp::from_json<std::shared_ptr<main_data>>(json_str);
    
    EXPECT_EQ((*parsed_ptr)->a, 123);
    EXPECT_EQ((*parsed_ptr)->b, "shared_ptr_test");
}

TEST(JsonCppTest, NullSharedPtrTest) {
    // 测试空shared_ptr
    std::shared_ptr<main_data> null_ptr;
    std::string json_str = jsoncpp::to_json(null_ptr);
    EXPECT_EQ(json_str, "null");
}

TEST(JsonCppTest, TypeConversionErrorTest) {
    // 测试类型转换错误
    std::string json_str = "{\"a\":\"not_a_number\"}";
    EXPECT_THROW(jsoncpp::from_json<main_data>(json_str), boost::system::system_error);
}

TEST(JsonCppTest, InvalidStructureTest) {
    // 测试无效结构
    std::string json_str = "{\"c\":\"not_an_array\"}";
    EXPECT_THROW(jsoncpp::from_json<main_data>(json_str), boost::system::system_error);
}

// 在全局命名空间为float_data定义转换器
class float_data {
public:
    float f;
    double d;
};

namespace jsoncpp {
    template<>
    struct transform<float_data> {
        static void trans(const bj::value &jv, float_data &t) {
            bj::object const &jo = jv.as_object();
            if (jo.contains("f")) {
                t.f = static_cast<float>(jo.at("f").as_double());
            }
            if (jo.contains("d")) {
                t.d = jo.at("d").as_double();
            }
        }
        
        static bj::value to_json(const float_data &t) {
            bj::object obj;
            obj["f"] = t.f;
            obj["d"] = t.d;
            return obj;
        }
    };
}

TEST(JsonCppTest, FloatingPointTest) {
    // 测试浮点数支持
    std::string json_str = "{\"f\":3.14, \"d\":2.718}";
    auto test = jsoncpp::from_json<float_data>(json_str);
    EXPECT_FLOAT_EQ(test->f, 3.14f);
    EXPECT_DOUBLE_EQ(test->d, 2.718);
    
    std::string serialized = jsoncpp::to_json(*test);
    EXPECT_TRUE(serialized.find("3.14") != std::string::npos);
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
