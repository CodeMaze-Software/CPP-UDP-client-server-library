//
//  main.cpp
//  CPP_UDP_Client_Server
//
//  Created by Dariusz Adamczyk on 08/03/2021.
//  Copyright © 2021 Dariusz Adamczyk. All rights reserved.
//

#include <iostream>
#include "udp_client_server.h"

char data_to_send[] = "Hello from CPP UDP client!\n";
char reply_data[] = "Reply from CPP UDP server!\n";
char data_to_recv[1024];
std::string server_ip_addr = "192.168.0.18";
std::string client_ip_addr = "192.168.0.18";
int port = 1234;

int main(int argc, const char * argv[]) {
    
    udp_client_server::udp_server server(udp_client_server::udp_helpers::get_local_ip_address(), port);
    //udp_client_server::udp_server server(server_ip_addr, port);
    
    udp_client_server::udp_client client(client_ip_addr, port);
    
    client.send(data_to_send, strlen(data_to_send));
    server.recv(data_to_recv, sizeof(data_to_recv));
    
    printf("[SERVER]Received datagram is: %s\nFrom IP: %s\n", data_to_recv, server.get_client_addr());
    
    server.reply(reply_data, strlen(reply_data));
    
    memset(data_to_recv, 0, sizeof(data_to_recv));
    
    client.recv(data_to_recv, sizeof(data_to_recv));
    printf("[CLIENT]Received datagram is: %s\n", data_to_recv);
    
    //End
    
    
    return 0;
}
