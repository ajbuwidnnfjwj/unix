#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <sstream>

#include <unistd.h>
#include <cstdio>
#include <cstdlib>

#include "../utils.h"

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

static inline void trim(std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t start = s.find_first_not_of(ws);
    std::size_t end   = s.find_last_not_of(ws);
    if (start == std::string::npos) {
        s.clear();
        return;
    }
    s = s.substr(start, end - start + 1);
}



struct idle_state : public state {
    std::unique_ptr<state> handle(context& ctx) override;
};

#include "download_state.h"
#include "upload_state.h"

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

#endif // CONTROLLERS_H
