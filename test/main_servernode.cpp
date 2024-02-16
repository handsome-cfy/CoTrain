#include"../node/node.h"
#include"../config/config.h"

using namespace CoTrain;
using namespace std;

int main(void){
    ServerNodeConfig::ptr config = ServerNodeConfig::ptr(new ServerNodeConfig("/Users/chenfeiyang/Documents/vscode/cfyserver/setting/ServerNode.json"));
    ServerNode::ptr server = ServerNode::ptr(new ServerNode(config,true));

    server->proccess();
    while(true){
        ;
    }
    return 0;
}