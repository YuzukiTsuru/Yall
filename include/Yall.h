#ifndef YALL_H
#define YALL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <iomanip>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#if MSVC
#define NOMINMAX
#endif

#include <Windows.h>
#include <wincon.h>

enum class cc : uint16_t {
    grey = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
    blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    green = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    cyan = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    red = FOREGROUND_RED | FOREGROUND_INTENSITY,
    magenta = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY,
    yellow = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
    white = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
    on_blue = BACKGROUND_BLUE,
    on_red = BACKGROUND_RED,
    on_magenta = BACKGROUND_BLUE | BACKGROUND_RED,
    on_grey = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
    on_green = BACKGROUND_GREEN | BACKGROUND_INTENSITY,
    on_cyan = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
    on_yellow = BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY,
    on_white = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY,
    reset
};
#else
enum class cc : uint8_t {
    grey = 30, red = 31, green = 32, yellow = 33,
    blue = 34, magenta = 35, cyan = 36, white = 37,
    on_grey = 40, on_red = 41, on_green = 42, on_yellow = 43,
    on_blue = 44, on_magenta = 45, on_cyan = 46, on_white = 47,
    reset
};
#endif

template<typename type>
type &operator<<(type &ostream, const cc color) {
#ifdef WIN32
    static const uint16_t initial_attributes = [] {
        CONSOLE_SCREEN_BUFFER_INFO buffer_info;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info);
        return buffer_info.wAttributes;
    }();
    static uint16_t background = initial_attributes & 0x00F0;
    static uint16_t foreground = initial_attributes & 0x000F;
#endif
    if (color == cc::reset) {
#ifdef WIN32
        ostream.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), initial_attributes);
#else
        ostream << "\033[m";
#endif
    } else {
#ifdef WIN32
        uint16_t set = 0;
        if (static_cast<uint16_t>(color) & 0x00F0) {
            background = static_cast<uint16_t>(color);
            set = background | foreground;
        } else if (static_cast<uint16_t>(color) & 0x000F) {
            foreground = static_cast<uint16_t>(color);
            set = background | foreground;
        }
        ostream.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), set);
#else
        ostream << "\033[" << static_cast<uint32_t>(color) << "m";
#endif
    }
    return ostream;
}

enum Yall_LEVEL {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRITICAL,
};

class Yall_Inst {
public:
    explicit Yall_Inst(Yall_LEVEL logLevel) {
        this->logLevel = logLevel;
    };

    virtual void operator<<(const std::string &msg) {};

private:
protected:
    std::string name;
    Yall_LEVEL logLevel;
    std::mutex streamMtx;
    std::vector<std::ostream *> streams;
};

class Yall_Instance : Yall_Inst {
public:
    explicit Yall_Instance(Yall_LEVEL logLevel) : Yall_Inst(logLevel) {
        streams.push_back(&std::cout);
    };

    void operator<<(const std::string &msg) override {
        std::lock_guard<std::mutex> lock(streamMtx);
        for (auto &stream: streams) {
            switch (logLevel) {
                case Yall_LEVEL::LOG_INFO:
                    *stream << cc::cyan << "[INFO]" << cc::reset;
                    break;
                case Yall_LEVEL::LOG_WARN:
                    *stream << cc::yellow << "[WARNING]" << cc::reset;
                    break;
                case Yall_LEVEL::LOG_ERROR:
                    *stream << cc::red << "[ERROR]" << cc::reset;
                    break;
                case Yall_LEVEL::LOG_CRITICAL:
                    *stream << cc::on_red << "[CRITICAL]" << cc::reset;
                    break;
                default:
                    break;
            }
            *stream << " " << msg << " " << std::endl;
        }
    };
};

class Yall_Debug_Instance : Yall_Inst {
public:
    explicit Yall_Debug_Instance(Yall_LEVEL logLevel) : Yall_Inst(logLevel) {}

    void SetDebugInfo(const std::string &file, const std::string &func, int line) {
        this->FILE = file;
        this->FUNC = func;
        this->LINE = line;
    }

    [[maybe_unused]] void EnableDebug() {
        auto it = std::find(streams.begin(), streams.end(), &std::cout);
        if (it == streams.end())
            streams.push_back(&std::cout);
    }

    [[maybe_unused]] void DisableDebug() {
        auto it = std::find(streams.begin(), streams.end(), &std::cout);
        if (it == streams.end())
            return;
        streams.erase(it);
    }

    void operator<<(const std::string &msg) override {
        std::lock_guard<std::mutex> lock(streamMtx);
        for (auto &stream: streams) {
            *stream << cc::cyan << "[FUNC] " << std::left << std::setw(23) << cc::reset << fmt(this->FUNC) << " "
                    << cc::yellow << "[FILE] " << std::setw(23) << cc::reset << fmt(this->FILE) << " "
                    << cc::green << "[LINE] " << std::setw(4) << cc::reset << this->LINE << " "
                    << cc::white << "[DEBUG] " << cc::reset << msg << " " << std::endl;
        }
    }

private:
    std::string FILE = {};
    std::string FUNC = {};
    int LINE = {};
private:
    // Get the last 20 char
    static std::string fmt(std::string_view sv) {
        if (sv.length() > 20) {
            return std::string("...") + sv.substr(sv.length() - 20, sv.length()).data();
        } else {
            return sv.data();
        }
    }
};

class Yall {
public:
    Yall(Yall const &) = delete;

    void operator=(Yall const &) = delete;

    static Yall_Instance &GetYall(Yall_LEVEL logLevel) {
        auto it = GetInstance().yall_inst.find(logLevel);
        if (it == GetInstance().yall_inst.end()) {
            auto *logger = new Yall_Instance(logLevel);
            GetInstance().yall_inst[logLevel] = logger;
            return *logger;
        }
        return *it->second;
    };

    static Yall_Debug_Instance &GetDebugYall(Yall_LEVEL logLevel, const std::string &FILE, const std::string &FUNC, int LINE) {
        auto it = GetDebugInstance().yall_debug_inst.find(logLevel);
        if (it == GetDebugInstance().yall_debug_inst.end()) {
            auto *logger = new Yall_Debug_Instance(logLevel);
            GetDebugInstance().yall_debug_inst[logLevel] = logger;
            logger->SetDebugInfo(FILE, FUNC, LINE);
            return *logger;
        }
        it->second->SetDebugInfo(FILE, FUNC, LINE);
        return *it->second;
    };

private:
    std::unordered_map<Yall_LEVEL, Yall_Instance *> yall_inst;
    std::unordered_map<Yall_LEVEL, Yall_Debug_Instance *> yall_debug_inst;

    Yall() = default;;

    static Yall &GetInstance() {
        static Yall inst;
        return inst;
    };

    static Yall &GetDebugInstance() {
        static Yall inst_d;
        return inst_d;
    };
};


#if __GNUC__
#define YALL_FUNC_        __PRETTY_FUNCTION__
#else
#define YALL_FUNC_        __func__
#endif

#define YALL_DEBUG_       Yall::GetDebugYall(Yall_LEVEL::LOG_DEBUG, __FILE__, YALL_FUNC_, __LINE__)
#define YALL_INFO_        Yall::GetYall(Yall_LEVEL::LOG_INFO)
#define YALL_WARN_        Yall::GetYall(Yall_LEVEL::LOG_WARN)
#define YALL_ERROR_       Yall::GetYall(Yall_LEVEL::LOG_ERROR)
#define YALL_CRITICAL_    Yall::GetYall(Yall_LEVEL::LOG_CRITICAL)

#endif // YALL_H
