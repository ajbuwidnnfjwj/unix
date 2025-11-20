#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#define STORAGE_DIR "storage"
#define CMD_BUF_SIZE 1024



int main(int argc, const char* argv[]) {

    if (argc != 5) {
        std::cout << "use correct id and password" << std::endl;
    }

    if (argv[2] == "--id" && argv[3] == "--password") {

    }

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

