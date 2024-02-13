#include"../node/node.h"
#include"../config/config.h"

using namespace CoTrain;
using namespace std;

int main(void){
    ClientNodeConfig::ptr config = ClientNodeConfig::ptr(new ClientNodeConfig("/Users/chenfeiyang/Documents/vscode/cfyserver/setting/ClientNode.json"));
    ClientNode::ptr client = std::make_shared<ClientNode>(config);    
    client->connect();
    client->alive();
    cout << config->getMachineID() << endl;
    
    return 0;
}