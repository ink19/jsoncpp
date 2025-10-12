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

namespace jsoncpp {

template<>
struct transform<ext_data> {
    static void trans(const bj::value &jv, ext_data &t) {
        t.data = 1024;
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

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
