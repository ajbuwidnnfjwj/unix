#ifndef UNIX_UTILS_H
#define UNIX_UTILS_H

#include <memory>
#include <fstream>

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

bool is_locked(std::string const &filename) {
    std::ifstream in(".auth");
    std::string path, pw;
    while (in >> path >> pw) {
        if (path == filename)
            return true;
    }
    return false;
}

bool unlock(std::string const &filename, std::string const &password) {
    std::ifstream in(".auth");
    std::string path, pw;
    while (in >> path >> pw) {
        if (path == filename) {
            if (pw != password) {
                std::cout << "incorrect password" << std::endl;
                break;
            }
            return true;
        }
    }
    return false;
}

#endif //UNIX_UTILS_H