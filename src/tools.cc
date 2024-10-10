#include "tools.h"
namespace tool {
    
std::string GenerateUUID() {
    uuid_t uuid;
    uuid_generate(uuid);
    char uuidStr[37];
    uuid_unparse(uuid, uuidStr);
    return std::string(uuidStr);
}

std::string GetThreadId() {
    auto id = std::this_thread::get_id();
    std::ostringstream  oss;
    oss << id;
    return oss.str();
}

int GetLocalPidByName(const char *process_name) {
    DIR *dir;
    struct dirent *ent;
    char buf[512];
    int pid = -1;

    if (!(dir = opendir("/proc"))) {
        perror("opendir");
        return -1;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (!isdigit(*ent->d_name)) {
            continue;
        }

        snprintf(buf, sizeof(buf), "/proc/%s/cmdline", ent->d_name);
        FILE *fp = fopen(buf, "r");
        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                if (strstr(buf, process_name) != NULL) {
                    pid = atoi(ent->d_name);
                    fclose(fp);
                    break;
                }
            }
            fclose(fp);
        }
    }

    closedir(dir);
    return pid;
}

Time::Time(const std::string& function_name) : m_function_name(function_name) {
    m_start = high_resolution_clock::now();
}

Time::~Time() {
    m_end = high_resolution_clock::now();
    auto time_span = duration_cast<nanoseconds>(m_end - m_start);
    spdlog::info("{} Time: {} nanoseconds.", m_function_name, time_span.count());
}

} // namespace tool