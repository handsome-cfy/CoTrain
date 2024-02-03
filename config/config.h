#ifndef __CoTrain_CONFIG_H__
#define __CoTrain_CONFIG_H__

#include"nlohmann/json.hpp"
#include"../log.h"
#include<fstream>
#include<sstream>

namespace CoTrain{

class Config{
public:
    typedef std::shared_ptr<Config> ptr;

    using json = nlohmann::json;
    Config(){}
    Config(std::string file_path);
    virtual ~Config() = 0;

    // 用来是的设置生效
    virtual void fitconfig() const = 0;
    // 用来保存设置
    virtual bool saveconfig(std::string file_path) = 0;

private:
    json m_data;
    LogMannager::ptr logger;
};

class TcpServerConfig : public Config{
public:

private:
};


}

#endif