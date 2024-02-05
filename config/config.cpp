#include "config.h"
#include <iostream>

CoTrain::Config::Config(std::string file_path)
{
    std::fstream file = std::fstream(file_path);
    if(!file.is_open()){
        // logger->instance();
        // std::stringstream ss;
        // ss << "The file " << file_path << " open failed!";
        // logger->error(
        //     logger->CreateEvent(
        //         ss.str()
        //     )
        // );
        return;
    }

    nlohmann::json jsonData;
    try {
        file >> jsonData;  // 从文件中解析 JSON
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        file.close();
        return;
    }
    m_data = jsonData;
    file.close();  // 关闭文件
}

uint32_t CoTrain::ServerNodeConfig::getport()
{
    return m_data['ServerPort'];
}

