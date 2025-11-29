#ifndef UNIX_LOCKING_STATE_H
#define UNIX_LOCKING_STATE_H

#include <string>

#include "../init.h"
#include "../utils.h"


struct locking_state : public state {
    std::string filename;
    std::string pw;

    locking_state(std::string filename, std::string pw)
        : filename(std::move(filename)), pw(std::move(pw)) {}

    std::unique_ptr<state> handle(context& ctx) override;
};

inline std::unique_ptr<state> locking_state::handle(context& ctx) {
    const std::string src  = ctx.cloud_root + "/" + filename;
    const std::string password = pw;

    if (is_locked(src)) return make_unique<idle_state>();

    int src_fd = ::open(src.c_str(), O_RDONLY);
    if (src_fd < 0) {
        std::perror("fail to find remote file");
        return make_unique<idle_state>();
    }
    ::close(src_fd); // 존재 확인용이므로 즉시 close

    // 비밀번호 기록
    fprintf(global_state->auth_fp, "%s %s\n", src.c_str(), password.c_str());
    fflush(global_state->auth_fp);

    return make_unique<idle_state>();
}


#endif //UNIX_LOCKING_STATE_H