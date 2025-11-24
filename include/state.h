#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <string>
#include <memory>
#include <iostream>
#include <utility>
#include <vector>
#include <sstream>

#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

#include "include/utils.h"

// ====== 전방 선언 ======
struct state;

// ====== context: 환경 정보 ======
struct context {
    std::string pwd;
    std::string cloud_root;

    static constexpr std::size_t BUF_SIZE = 1024;

    explicit context(const std::string& root) : cloud_root(root) {
        char* cwd = ::getcwd(nullptr, 0);
        if (cwd != nullptr) {
            pwd = std::string(cwd);
            std::free(cwd);
        } else {
            std::perror("fail to initialize path");
        }
    }
};

// ====== state 인터페이스 ======
struct state {
    virtual ~state() = default;
    // 한 번 handle 호출 → 다음 상태를 unique_ptr로 반환 (nullptr이면 종료)
    virtual std::unique_ptr<state> handle(context& ctx) = 0;
};

// 앞으로 정의할 상태들 전방 선언
struct idle_state;
struct upload_state;
struct download_state;

// ====== 각 상태 클래스 선언 ======

struct idle_state : public state {
    std::unique_ptr<state> handle(context& ctx) override;
};

// upload_state: "upload <local> <remote>" 실행
struct upload_state : public state {
    std::string local_path;
    std::string remote_path; // cloud_root 기준 상대 경로

    upload_state(std::string local, std::string remote)
        : local_path(std::move(local)), remote_path(std::move(remote)) {}

    std::unique_ptr<state> handle(context& ctx) override;
};

// download_state: "download <remote> <local>" 실행
struct download_state : public state {
    std::string remote_path; // cloud_root 기준 상대 경로
    std::string local_path;

    download_state(std::string remote, std::string local)
        : remote_path(std::move(remote)), local_path(std::move(local)) {}

    std::unique_ptr<state> handle(context& ctx) override;
};

// ====== 내부 유틸 함수(헤더 안에서만 사용) ======

inline void trim(std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t start = s.find_first_not_of(ws);
    std::size_t end   = s.find_last_not_of(ws);
    if (start == std::string::npos) {
        s.clear();
        return;
    }
    s = s.substr(start, end - start + 1);
}

// ====== idle_state 구현 ======

inline std::unique_ptr<state> idle_state::handle(context& ctx) {
    std::cout << "[pwd: " << ctx.pwd << "] $ ";

    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "입력 스트림 종료. 프로그램을 종료합니다.\n";
        return nullptr;
    }

    trim(line);
    if (line.empty()) {
        // 빈 줄이면 그냥 다시 idle
        return make_unique<idle_state>();
    }

    // 토큰 분리
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        return make_unique<idle_state>();
    }

    const std::string& command = tokens[0];

    if (command == "upload") {
        if (tokens.size() != 3) {
            std::cout << "Usage: upload <local_path> <remote_path>\n";
            return make_unique<idle_state>();
        }
        // 인자를 가진 상태 객체 생성
        return make_unique<upload_state>(tokens[1], tokens[2]);
    }
    else if (command == "download") {
        if (tokens.size() != 3) {
            std::cout << "Usage: download <remote_path> <local_path>\n";
            return make_unique<idle_state>();
        }
        return make_unique<download_state>(tokens[1], tokens[2]);
    }
    else if (command == "exit") {
        std::cout << "bye.\n";
        return nullptr;
    }
    else {
        std::cout << "Invalid command: " << command << "\n";
        return make_unique<idle_state>();
    }
}

// ====== upload_state 구현 ======

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

// ====== download_state 구현 ======

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

#endif // CONTROLLERS_H
