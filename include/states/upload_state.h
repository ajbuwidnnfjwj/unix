#ifndef UNIX_UPLOAD_STATE_H
#define UNIX_UPLOAD_STATE_H

#include <string>
#include <fcntl.h>
#include <sys/stat.h>

#include "state.h"
#include "../utils.h"

struct upload_state : public state {
    std::string local_path;
    std::string remote_path; // cloud_root 기준 상대 경로

    upload_state(std::string local, std::string remote)
        : local_path(std::move(local)), remote_path(std::move(remote)) {}

    std::unique_ptr<state> handle(context& ctx) override;
};

inline std::unique_ptr<state> upload_state::handle(context& ctx) {
    const std::string src  = local_path;
    const std::string dest = ctx.cloud_root + "/" + remote_path;

    int src_fd = ::open(src.c_str(), O_RDONLY);
    if (src_fd < 0) {
        std::perror("fail to open src file");
        return make_unique<idle_state>();
    }

    struct stat st{};
    if (::stat(src.c_str(), &st) < 0) {
        std::perror("fail to stat src file");
        ::close(src_fd);
        return make_unique<idle_state>();
    }

    int dest_fd = ::open(dest.c_str(),
                         O_WRONLY | O_CREAT | O_TRUNC,
                         st.st_mode & 0777);
    if (dest_fd < 0) {
        std::perror("대상 파일 열기 실패");
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
                std::perror("fail to write to dest file");
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

    std::cout << "upload success: " << src << " -> " << dest << "\n";
    // 작업 끝나면 다시 idle 상태로
    return make_unique<idle_state>();
}

#endif //UNIX_UPLOAD_STATE_H