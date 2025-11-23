#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#define STORAGE_DIR "storage"
#define CMD_BUF_SIZE 1024


std::string VALID_ID = "user1";
std::string VALID_PW = "1234";


int main(int argc, const char* argv[]) {

    if (argc != 5) {
        std::cout << "use correct id and password" << std::endl;
    }

    std::string argv1 = argv[1];
    std::string id = argv[2];
    std::string argv3 = argv[3];
    std::string pw = argv[4];

    if (argv1 != "--id" || (argv3 != "--pw" && argv3 != "--password")) {
        std::cout << "Usage: " << argv[0] << " --id <user> --pw <password>\n";
    }

    if (id != VALID_ID || pw != VALID_PW) {
        std::cout << "Invalid id or password\n";
        return 0;
    }
    
    std::cout << "Login success. Welcome, " << id << "!\n";

    while (true) {
        std::string line;
        std::getline(std::cin, line);

        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (iss >> token) {
            tokens.push_back(token);
        }

        //handle empty command
        if (tokens.empty())
            continue;

        std::string& command = tokens[0];


    }

    return 0;
}

