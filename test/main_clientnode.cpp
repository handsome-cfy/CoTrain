#include"../node/node.h"
#include"../config/config.h"

using namespace CoTrain;
using namespace std;

int main(void){
    ClientNodeConfig::ptr config = ClientNodeConfig::ptr(new ClientNodeConfig("/Users/chenfeiyang/Documents/vscode/cfyserver/setting/ClientNode.json"));
    ClientNode::ptr client = std::make_shared<ClientNode>(config);    
    client->connect();
    
    client->sendfile("/Users/chenfeiyang/Documents/vscode/cfyserver/setting/ClientNode.json");
    client->startScript();
    // client->sendfile("/Users/chenfeiyang/Documents/vscode/cfyserver/experiment/file_2024-03-28_15-46-13.json");

    client->alive();
    // Thread::ptr alivethread = Thread::ptr(new Thread("alive",[client](){
    //     client->alive();
    // }));
    // alivethread->join();
    // client->alive();

    cout << config->getMachineID() << endl;
    
    while(true);
    return 0;
}