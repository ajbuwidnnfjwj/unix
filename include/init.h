#ifndef UNIX_INIT_H
#define UNIX_INIT_H
#include <memory>

struct global_state {
    int auth_fd;
    FILE* auth_fp;
};

std::unique_ptr<global_state> global_state;

#endif //UNIX_INIT_H