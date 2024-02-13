#ifndef __CoTrain_CONFIG_H__
#define __CoTrain_CONFIG_H__

#include"nlohmann/json.hpp"


#include<fstream>
#include<sstream>
#include<string>


namespace CoTrain{

class Config{
public:
    typedef std::shared_ptr<Config> ptr;

    using json = nlohmann::json;
    Config(){}
    Config(std::string file_path);
    ~Config(){}
    // 用来是的设置生效
    virtual void fitconfig() const = 0;
    // 用来保存设置
    virtual bool saveconfig(std::string file_path) = 0;

protected:
    json m_data;

private:
};

class ServerNodeConfig : public Config{
public:
    typedef std::shared_ptr<ServerNodeConfig> ptr;
    uint32_t getport();
    uint16_t getMaxThreadNumber(){return m_data["MaxPoolThreadNumber"];}
private:
    
};

class ClientNodeConfig : public Config{
public:
    void fitconfig()const override{}
    bool saveconfig(std::string file_path) override{}
    typedef std::shared_ptr<ClientNodeConfig> ptr;
    std::string getServerIP(){return m_data["ServerIP"];}
    uint32_t getServerPort(){return m_data["ServerPort"];}
    uint64_t getMachineID(){return m_data["MachineID"];}

    ClientNodeConfig(std::string filepath) : Config(filepath){

    }
private:

};


}

#endif