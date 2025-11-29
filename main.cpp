#include <iostream>
#include <string>
#include <sstream>

#include "include/init.h"
#include "include/states/state.h"

#define STORAGE_DIR "storage"
#define CMD_BUF_SIZE 1024


std::string VALID_ID = "user1";
std::string VALID_PW = "1234";

std::string commands[2] = { "upload", "download" };

int main(int argc, const char* argv[]) {

    // login process
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

    if (id != VALID_ID || pw != VALID_PW) {
        std::cout << "Invalid id or password\n";
        return 0;
    }

    global_state = make_unique<struct global_state>();
    global_state->auth_fd = ::open(".auth", O_RDWR | O_CREAT | O_APPEND, 0600);
    if (global_state->auth_fd < 0) {
        perror("open .auth");
        std::cout << "fail to init program\n";
        return 0;
    }
    global_state->auth_fp = fdopen(global_state->auth_fd, "a+");
    if (!global_state->auth_fp) {
        perror("fdopen .auth");
        std::cout << "fail to init program\n";
        return 0;
    }

    std::cout << "Login success. Welcome, " << id << "!\n";

    context ctx(id);
    std::unique_ptr<state> current = make_unique<idle_state>();

    while (current) {
        current = current->handle(ctx);
    }

    // clean up auth file
    fclose(global_state->auth_fp);

    return 0;
}