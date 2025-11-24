#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "include/state.h"

#define STORAGE_DIR "storage"
#define CMD_BUF_SIZE 1024


std::string VALID_ID = "user1";
std::string VALID_PW = "1234";

std::string commands[2] = { "upload", "download" };

int main(int argc, const char* argv[]) {

    if (argc != 5) {
        std::cout << "use correct id and password" << std::endl;
    }

    std::string argv1 = argv[1];
    std::string id = argv[2];
    std::string argv3 = argv[3];
    std::string pw = argv[4];

    if (argv[2] == "--id" && argv[3] == "--password") {
        std::cout << "Usage: " << argv[0] << " --id <user> --pw <password>\n";
    }

    std::cout << "Login success. Welcome, " << id << "!\n";

    if (id != VALID_ID || pw != VALID_PW) {
        std::cout << "Invalid id or password\n";
        return 0;
    }

    std::cout << "Login success. Welcome, " << id << "!\n";

    context ctx(id);
    std::unique_ptr<state> current = std::make_unique<idle_state>();

    while (current) {
        current = current->handle(ctx);
    }

    return 0;
}