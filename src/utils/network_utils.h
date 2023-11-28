#ifndef _NETWORK_UTILS_H_
#define _NETWORK_UTILS_H_

#include <iostream>
#include <fstream>
#include <string>

#define SOCKETERROR (-1)

using namespace std;

int check(int exp, const char *msg) {
    if(exp == SOCKETERROR) {
        perror(msg);
        exit(1);
    }
    return 0;
}

int serverMessage(string serverPort){
    string command = "hostname -I | cut -d' ' -f1";
    string ipAddress;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        cerr << "Command failed." << endl;
        return -1;
    }

    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) {
            ipAddress += buffer;
        }
    }
    pclose(pipe);

    //Remove trailing newline character if present
    size_t newlinePos = ipAddress.find_last_not_of("\r\n");
    if (newlinePos != std::string::npos) {
        ipAddress = ipAddress.substr(0, newlinePos + 1);
    }
    
    cout << "+----------------------------------------------------------+" << endl;
    cout << "|███████╗██╗     ██╗██████╗ ██╗███╗   ██╗ ██████╗          |" << endl;
    cout << "|██╔════╝██║     ██║██╔══██╗██║████╗  ██║██╔════╝          |" << endl;
    cout << "|███████╗██║     ██║██║  ██║██║██╔██╗ ██║██║  ███╗         |" << endl;
    cout << "|╚════██║██║     ██║██║  ██║██║██║╚██╗██║██║   ██║         |" << endl;
    cout << "|███████║███████╗██║██████╔╝██║██║ ╚████║╚██████╔╝         |" << endl;
    cout << "|╚══════╝╚══════╝╚═╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝          |" << endl;
    cout << "|██╗    ██╗██╗███╗   ██╗██████╗  ██████╗ ██╗    ██╗███████╗|" << endl;
    cout << "|██║    ██║██║████╗  ██║██╔══██╗██╔═══██╗██║    ██║██╔════╝|" << endl;
    cout << "|██║ █╗ ██║██║██╔██╗ ██║██║  ██║██║   ██║██║ █╗ ██║███████╗|" << endl;
    cout << "|██║███╗██║██║██║╚██╗██║██║  ██║██║   ██║██║███╗██║╚════██║|" << endl;
    cout << "|╚███╔███╔╝██║██║ ╚████║██████╔╝╚██████╔╝╚███╔███╔╝███████║|" << endl;
    cout << "| ╚══╝╚══╝ ╚═╝╚═╝  ╚═══╝╚═════╝  ╚═════╝  ╚══╝╚══╝ ╚══════╝|" << endl;
    cout << "+----------------------------------------------------------+" << endl;
    cout << "Server Status: Online" << endl;
    cout << "Server IP Address: " << ipAddress << ":" << serverPort << endl;
    return 0;
}
#endif /* _NETWORK_UTILS_H_ */