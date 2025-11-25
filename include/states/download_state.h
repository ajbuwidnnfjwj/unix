#ifndef UNIX_DOWNLOAD_STATE_H
#define UNIX_DOWNLOAD_STATE_H

#include <string>
#include <fcntl.h>
#include <sys/stat.h>

#include "state.h"
#include "../utils.h"

// download_state: "download <remote> <local>" 실행
struct download_state : public state {
    std::string remote_path; // cloud_root 기준 상대 경로
    std::string local_path;

    download_state(std::string remote, std::string local)
        : remote_path(std::move(remote)), local_path(std::move(local)) {}

    std::unique_ptr<state> handle(context& ctx) override;
};

inline std::unique_ptr<state> download_state::handle(context& ctx) {
    const std::string src  = ctx.cloud_root + "/" + remote_path;
    const std::string dest = local_path;

    int src_fd = ::open(src.c_str(), O_RDONLY);
    if (src_fd < 0) {
        std::perror("fail to open remote file");
        return make_unique<idle_state>();
    }

    struct stat st{};
    if (::stat(src.c_str(), &st) < 0) {
        std::perror("fail to stat remote file");
        ::close(src_fd);
        return make_unique<idle_state>();
    }

    int dest_fd = ::open(dest.c_str(),
                         O_WRONLY | O_CREAT | O_TRUNC,
                         st.st_mode & 0777);
    if (dest_fd < 0) {
        std::perror("fail to open local dest file");
        ::close(src_fd);
        return make_unique<idle_state>();
    }

    char buffer[context::BUF_SIZE];
    ssize_t bytes;

    while ((bytes = ::read(src_fd, buffer, context::BUF_SIZE)) > 0) {
        ssize_t written = 0;
        while (written < bytes) {
            ssize_t w = ::write(dest_fd, buffer + written, bytes - written);
            if (w < 0) {
                std::perror("writing fail");
                ::close(src_fd);
                ::close(dest_fd);
                return make_unique<idle_state>();
            }
            written += w;
        }
    }

    if (bytes < 0) {
        std::perror("fail to read");
    }

    ::close(src_fd);
    ::close(dest_fd);

    std::cout << "download success: " << src << " -> " << dest << "\n";
    return make_unique<idle_state>();
}

#endif //UNIX_DOWNLOAD_STATE_H