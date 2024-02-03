#include <iostream>
#include "nlohmann/json.hpp"
#include "../config/config.h"

using json = nlohmann::json;

int main() {
    // 创建一个 JSON 对象
    json data = {
        {"name", "John"},
        {"age", 30},
        {"city", "New York"}
    };

    // 将 JSON 对象转换为字符串
    std::string jsonStr = data.dump();

    std::cout << "JSON string: " << jsonStr << std::endl;

    // 从字符串解析 JSON 数据
    json parsedData = json::parse(jsonStr);

    // 访问 JSON 数据的字段
    std::string name = parsedData["name"];
    int age = parsedData["age"];

    std::cout << "Name: " << name << std::endl;
    std::cout << "Age: " << age << std::endl;

    return 0;
}