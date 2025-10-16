#include "jsoncpp.hpp"
#include <chrono>
#include <iostream>
#include <vector>

class performance_data {
public:
    int id;
    std::string name;
    std::vector<int> scores;
    bool active;
    std::map<std::string, std::string> metadata;
    
    constexpr static std::string_view __jsoncpp_alias_name(const std::string_view& name) {
        if (name == "metadata") {
            return "meta";
        }
        return name;
    }
};

void run_performance_test() {
    std::cout << "=== JSON C++ 库性能测试 ===\n";
    
    // 创建测试数据
    performance_data data;
    data.id = 12345;
    data.name = "Performance Test Object";
    data.scores = {95, 87, 92, 78, 85, 96, 88, 91, 84, 79};
    data.active = true;
    data.metadata = {
        {"version", "1.0.0"},
        {"author", "JSON C++ Library"},
        {"timestamp", "2024-01-01"}
    };
    
    const int iterations = 1000;
    
    // 序列化性能测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto json_str = jsoncpp::to_json(data);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto serialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 反序列化性能测试
    std::string test_json = R"({
        "id": 999,
        "name": "Test Object",
        "scores": [1, 2, 3, 4, 5],
        "active": true,
        "meta": {
            "key1": "value1",
            "key2": "value2"
        }
    })";
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto parsed = jsoncpp::from_json<performance_data>(test_json);
    }
    end = std::chrono::high_resolution_clock::now();
    auto deserialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "序列化性能 (" << iterations << " 次迭代):\n";
    std::cout << "  总时间: " << serialize_duration.count() << " 微秒\n";
    std::cout << "  平均时间: " << serialize_duration.count() / iterations << " 微秒/次\n\n";
    
    std::cout << "反序列化性能 (" << iterations << " 次迭代):\n";
    std::cout << "  总时间: " << deserialize_duration.count() << " 微秒\n";
    std::cout << "  平均时间: " << deserialize_duration.count() / iterations << " 微秒/次\n\n";
    
    // 功能演示
    std::cout << "=== 功能演示 ===\n";
    std::string demo_json = R"({
        "id": "42",
        "name": "Demo Object",
        "scores": ["10", "20", "30"],
        "active": "true",
        "meta": {
            "count": "5",
            "enabled": "1"
        }
    })";
    
    try {
        auto demo_obj = jsoncpp::from_json<performance_data>(demo_json);
        std::cout << "反序列化成功:\n";
        std::cout << "  ID: " << demo_obj->id << "\n";
        std::cout << "  Name: " << demo_obj->name << "\n";
        std::cout << "  Scores 数量: " << demo_obj->scores.size() << "\n";
        std::cout << "  Active: " << (demo_obj->active ? "true" : "false") << "\n";
        std::cout << "  Metadata 数量: " << demo_obj->metadata.size() << "\n";
        
        // 序列化回 JSON
        std::string serialized = jsoncpp::to_json(*demo_obj);
        std::cout << "\n序列化结果 (前100字符): " << serialized.substr(0, 100) << "...\n";
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << "\n";
    }
}

int main() {
    run_performance_test();
    return 0;
}
