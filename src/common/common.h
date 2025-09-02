#pragma once
/*******************************************************************************
 * \file
 * File: common.h
 * Created: 02/04/25
 * Author: Umut Sevdi
 * Description: Methods and values used across the application
 * such as helpers for networking, file system access and logging
 *
 * Project: umutsevdi/logic-circuit-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <libintl.h>
#include <string>
#include <vector>
#ifdef ERROR
#undef ERROR
#endif
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif

#define APPVERSION "0.0.2"
#define APPPKG "com.lcs.app"
#define APPNAME "Logic Circuit Simulator"
#define APPNAME_BIN "LogicCircuitSimulator"
#ifdef _WIN32
#define APPOS "win"
#elif __APPLE__
#define APPOS "apple"
#elif defined(__linux__)
#define APPOS "linux"
#elif defined(__unix__)
#define APPOS "unix"
#else
#error "Unsupported platform"
#endif

#ifdef NDEBUG
#define APPBUILD "rel"
#else
#define APPBUILD "dbg"
#endif

#ifndef API_ENDPOINT
#ifndef NDEBUG
#define API_ENDPOINT "http://localhost:8000"
#else
#define API_ENDPOINT "https://lcs2.com"
#endif
#endif
#include "errors.h"

#define _(str) gettext(str)

#define LCS_ERROR [[nodiscard("Error codes must be handled")]] Error

namespace lcs {
template <typename T> const char* to_str(T);
/**
 * A custom container class that stores a pointer to an object defined
 * within a scene. Can not be stored, copied or assigned.
 *
 * Intended use case:
 * get_node<lcs::GateNode>(id)->signal();
 * get_node<lcs::InputNode>(id)->toggle();
 */
template <typename T> class NRef {
public:
    NRef(T* v)
        : _v(v) { };
    NRef(NRef&&)                 = default;
    NRef(const NRef&)            = delete;
    NRef& operator=(NRef&&)      = delete;
    NRef& operator=(const NRef&) = delete;
    ~NRef() { }

    inline bool operator==(void* t) const { return _v == t; };
    inline bool operator!=(void* t) const { return _v != t; };
    inline T* operator->() { return _v; }
    inline const T* operator->() const { return _v; }
    inline T* operator&() { return _v; }
    inline T& operator*() { return *_v; }
    inline const T& operator*() const { return *_v; }

private:
    T* _v;
};

/******************************************************************************
                                  FILESYSTEM/
******************************************************************************/

/**
 * Single Log message. Log messages are printed to the terminal,
 * written to the test output file and displayed on the Console window.
 */
struct Message {
    enum Severity { DEBUG, INFO, WARN, ERROR, FATAL };
    Message()         = default;
    Severity severity = DEBUG;
    std::array<char, 12> time_str {};
    std::array<char, 6> log_level {};
    std::array<char, 10> obj {};
    std::array<char, 10> file {};
    std::array<char, 15> file_line {};
    std::array<char, 16> fn {};
    std::array<char, 380> expr {};
    int line = 0;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#elif defined(_MSC_VER)
#pragma warning(disable : 4477) // suppress format‑string security warning
#pragma warning(push)
#endif
    template <typename... Args>
    Message(Severity _severity, const char* _file, int _line, const char* _fn,
        const char* _fmt, Args... _args)
    {
        _set_time();
        severity = _severity;
        line     = _line;
        std::strncpy(
            log_level.data(), _severity_to_str(_severity), log_level.size());
        std::snprintf(file.data(), file.max_size(), "%s", _file);
        std::snprintf(
            file_line.data(), file_line.max_size(), "%s:%-4d", _file, _line);
        _fn_parse(_fn);
        std::snprintf(expr.data(), expr.max_size(), _fmt, _args...);
    }
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
private:
    void _fn_parse(const char* name);
    void _set_time(void);
    static constexpr const char* _severity_to_str(Severity l)
    {
        switch (l) {
        case FATAL: return "FATAL";
        case DEBUG: return "DEBUG";
        case INFO: return "INFO ";
        case WARN: return "WARN ";
        default: return "ERROR";
        }
    }
};

namespace fs {
#if defined(__GNUC__)
#define __LLOG__(STATUS, ...)                                                  \
    lcs::fs::_log(Message {                                                    \
        STATUS, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__ })
#elif defined(_MSC_VER)
#define __FILENAME__                                                           \
    (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __LLOG__(STATUS, ...)                                                  \
    lcs::fs::_log(                                                             \
        Message { STATUS, __FILENAME__, __LINE__, __FUNCSIG__, __VA_ARGS__ })
#endif

#define L_DEBUG(...) __LLOG__(Message::DEBUG, __VA_ARGS__)
#define L_INFO(...) __LLOG__(Message::INFO, __VA_ARGS__)
#define L_WARN(...) __LLOG__(Message::WARN, __VA_ARGS__)
#define L_ERROR(...) __LLOG__(Message::ERROR, __VA_ARGS__)
/** Runs an assertion. Displays an error message on failure. In debug builds
 * also crashes the application. */
#define lcs_assert(expr)                                                       \
    {                                                                          \
        try {                                                                  \
            if (!(expr)) {                                                     \
                __LLOG__(Message::FATAL, "Assertion \"" #expr "\" failed!");   \
                exit(1);                                                       \
            }                                                                  \
        } catch (const std::exception& ex) {                                   \
            __LLOG__(Message::FATAL,                                           \
                " Assertion \"" #expr                                          \
                "\" failed with an exception! Cause: %s",                      \
                ex.what());                                                    \
            exit(1);                                                           \
        } catch (const std::string& ex) {                                      \
            __LLOG__(Message::FATAL,                                           \
                "Assertion \"" #expr "\" failed with an exception! Cause: %s", \
                ex.c_str());                                                   \
            exit(1);                                                           \
        }                                                                      \
    }

#define ERROR(err) ((L_ERROR("%s(%d): %s", #err, err, errmsg(err))), err)

    /**
     * Initializes required folder structure for the application.
     * If testing flag is enabled files are saved to a temporary location.
     * @param is_testing whether testing mode is enabled or not
     */
    void init(bool is_testing = false);

    void close(void);

    /**
     * Loop over existing logs starting from the oldest.
     * @param fn iteration function
     */
    void logs_for_each(std::function<void(size_t, const Message& l)> fn);

    /** Clears all log messages. */
    void clear_log(void);

    extern bool is_testing;
    extern bool is_verbose;
    /** Root level directory where required files live.
     * - Default configuration values.
     * - Fonts
     *
     * NOTE: On Windows also contains the executable, locales and DLLs.
     */
    extern std::filesystem::path APPDATA;
    /** Path to translations. */
    extern std::filesystem::path LOCALE;
    /** Contains caches of images, package downloads. */
    extern std::filesystem::path CACHE;
    /** Path where external scenes and components are installed. */
    extern std::filesystem::path LIBRARY;
    /** Path to configuration files of the user. */
    extern std::filesystem::path CONFIG;
    /** NOTE: Active only in tests. Path to test run directory's log path. */
    extern std::filesystem::path LOGPATH;
    /** Set the target file name for logging. NOTE: Intended for tests. */
    void set_log_target(const char*);

    const char** locales(void);
    const char** localnames(void);
    size_t localsize(void);

    /** Runs an assertion, displays an error message on failure. Intended
     * for macros. */
    int __expect(std::function<bool(void)> expr, const char* function,
        const char* file, int line, const char* str_expr) noexcept;
    /**
     * Push a log message to the stack. Intended to be used by the macros
     * such as L_INFO, L_WARN, L_ERROR, L_DEBUG.
     */
    void _log(const Message& l);

    /**
     * Reads contents of the given string file and writes it to the buffer.
     * @param path to read
     * @param data to save
     * @returns whether reading is successful or not
     */
    bool read(const std::filesystem::path& path, std::string& data);

    /**
     * Reads contents of the given binary file and writes it to the buffer.
     * @param path to read
     * @param data to save
     * @returns whether reading is successful or not
     */
    bool read(
        const std::filesystem::path& path, std::vector<unsigned char>& data);

    /**
     * Write contents of data to the desired path.
     * @param path to save
     * @param data to save
     * @returns Whether the operation is successful or not
     */
    bool write(const std::filesystem::path& path, const std::string& data);

    /**
     * Write contents of data to the desired path. Used for binary files.
     * @param path to save
     * @param data to save
     * @returns Whether the operation is successful or not
     */
    bool write(
        const std::filesystem::path& path, std::vector<unsigned char>& data);

} // namespace fs

namespace net {
    void init(bool testing = false);
    void close(void);

    struct HttpResponse {
        std::vector<uint8_t> data;
        int status_code = 0;
        Error err       = Error::OK;
    };

    /** Send a GET request to targeted URL.
     * @param URL target URL.
     * @param authorization header, optional.
     *
     * @returns HttpRequestId
     */
    uint64_t get_request(
        const std::string& URL, const std::string& authorization = "");

    /** Send a GET request to targeted URL. Execute a callback upon receiving
     * response.
     * @param cb callback function.
     * @param URL target URL.
     * @param authorization header, optional.
     */
    void get_request_then(std::function<void(HttpResponse& resp)> cb,
        const std::string& URL, const std::string& authorization = "");

    /** Send a POST request to targeted URL.
     * @param URL target URL.
     * @param req request body.
     * @param authorization header, optional.
     *
     * @returns HttpRequestId
     */
    uint64_t post_request(const std::string& URL,
        const std::vector<uint8_t>& req, const std::string& authorization = "");

    /** Send a POST request to targeted URL. Execute a callback upon receiving
     * response.
     * @param cb callback function.
     * @param URL target URL.
     * @param req request body.
     * @param authorization header, optional.
     */
    void post_request_then(std::function<void(HttpResponse& resp)> cb,
        const std::string& URL, const std::vector<uint8_t>& req,
        const std::string& authorization = "");

    /**
     * Attempts  to obtain a completed request, updating session on success.
     * @param id to query.
     * @param session to update
     *
     * @returns whether request is completed or not.
     */
    bool pull_response(uint64_t id, HttpResponse& session);

} // namespace net

/**
 * Opens the provided URL on the operating system's preferred browser.
 * @param url to open
 */
void open_browser(const std::string& url);

} // namespace lcs

template <typename T, typename... Args> T get_first(T first, Args...)
{
    return first;
}

std::string strlimit(const std::string& input, size_t limit);
std::vector<std::string> split(std::string& s, const std::string& delimiter);
std::vector<std::string> split(std::string& s, const char delimiter);
std::string base64_encode(const std::string& input);
std::string base64_decode(const std::string& input);

/** Converts the unsigned integer hostlong from host byte order to network
 * byte order.
 * @param host number to convert
 * * Conforming to POSIX.1-2001.
 */
inline uint32_t htonl(uint32_t host)
{
    uint32_t test = 1;
    if (*(uint8_t*)&test == 1) {
        uint32_t network = 0;
        network |= ((host >> 24) & 0xFF);
        network |= ((host >> 8) & 0xFF00);
        network |= ((host << 8) & 0xFF0000);
        network |= ((host << 24) & 0xFF000000);
        return network;
    }
    return host;
}

/**  Converts the unsigned integer netlong from network byte order to host byte
 * order.
 * @param network number to convert
 * * Conforming to POSIX.1-2001.
 */
inline uint32_t ntohl(uint32_t network) { return htonl(network); }
