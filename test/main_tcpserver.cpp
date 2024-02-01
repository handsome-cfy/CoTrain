#include"../socket.h"
#include <iostream>
using namespace std;
using namespace CoTrain;

int main(void){
    cout << 'h' << endl;
    cout << '1' << endl;
    TcpServer::ptr server = TcpServer::ptr(new TcpServer());
    server->Listen(8000);

}