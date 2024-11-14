#include <iostream>
#include <type_traits>
#include <sstream>
#include <string_view>
#include <format>
#include <print>
#include <unordered_map>
#include <cassert>
#include <format>
#include <chrono>
#include <ctime>
#include <map>
#include <iomanip>

// #ifndef __VA_ARGS__
// #define __VA_ARGS__ 1
// #endif

template <typename T>
concept is_printable = requires(T t) {
    { std::cout << t } -> std::same_as<std::ostream&>;
};

template <typename... T> constexpr void log(T... args) {
    static_assert((is_printable<T> && ...), "All types must be printable");
    ((std::cout << args << std::endl), ...);
}

//
// template <typename... T> auto fmt_log(std::string_view fmt, T... args) -> void {
//    static_assert((is_printable<T> && ...), "All types must be printable");
//    ((std::cout << args << std::endl), ...);
//}

template <typename... T> void fmt_log(std::string_view fmt, T... args) {
    static_assert((is_printable<T> && ...), "All types must be printable");

    std::ostringstream oss;
    const char* fmt_ptr = fmt.data();
    size_t fmt_len = fmt.size();
    size_t arg_index = 0;
    auto print_arg = [&](auto&& arg) {
        while (arg_index < fmt_len && fmt_ptr[arg_index] != '{') { oss << fmt_ptr[arg_index++]; }
        if (arg_index < fmt_len && fmt_ptr[arg_index] == '{' && fmt_ptr[arg_index + 1] == '}') {
            oss << arg;
            arg_index += 2;
        }
    };

    (print_arg(args), ...);

    while (arg_index < fmt_len) { oss << fmt_ptr[arg_index++]; }

    std::cout << oss.str() << std::endl;
}

// #define LOG(...) log(__VA_ARGS__, __PRETTY_FUNCTION__, __LINE__)
// #def ine FMT_LOG(fmt, ...) \
//    fmt_log(fmt "\t[function: {}, line: {}, file: {}]", __VA_ARGS__, __PRETTY_FUNCTION__, __LINE__, __FILE__)
// #def ine STD_FMT_LOG(fmt, ...) \
//    std::print(fmt "\t[function: {}, line: {}, file: {}]", __VA_ARGS__, __PRETTY_FUNCTION__, __LINE__, __FILE__)

enum AdditionInfo { FUNCTION = 0, LINE = 1, FILE_NAME = 2, TIME = 3 };

std::optional<AdditionInfo> get_enum(int i) {
    assert(i >= 0 && i <= 4);
    return static_cast<AdditionInfo>(i);
}

int get_idx(AdditionInfo info) { return static_cast<int>(info); }

struct AdditionInfoData {
        bool enabled = false;
        const std::string representation;
};

std::map<AdditionInfo, AdditionInfoData> addition_info_map = {
    {FUNCTION, {.enabled = false, .representation = "Func"}},
    {LINE, {.enabled = false, .representation = "Line"}},
    {FILE_NAME, {.enabled = false, .representation = "File"}},
    {TIME, {.enabled = false, .representation = "Time"}}};

void enable(AdditionInfo info) { addition_info_map.at(info).enabled = true; }

void disable(AdditionInfo info) { addition_info_map.at(info).enabled = false; }

std::string get_chunk() {
    std::string chunk;
    for (const auto& [key, value] : addition_info_map) {
        if (value.enabled) {
            if (!chunk.empty()) {
                chunk += ", ";
            } else {
                chunk += " [";
            }
            chunk += "{}";
        }
    }

    if (!chunk.empty()) { chunk += "]"; }
    return chunk;
}

#define KEYWORD_STRCAT(x, y) x##y

std::string current_time_to_string() {
    // Get the current time as a time_point
    auto now = std::chrono::system_clock::now();

    // Convert to a time_t, which is easier to work with for formatting
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to a tm struct for custom formatting
    std::tm now_tm = *std::localtime(&now_time_t);

    // Use a stringstream to format the time as a string
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

int count = 0;
std::array<std::string, 4> info_frame {};
std::string head = "[VOSS] ";
#define LOG(fmt_str, ...)                                                                                              \
    count = 0;                                                                                                         \
    info_frame.fill("");                                                                                               \
    if (addition_info_map[FUNCTION].enabled) {                                                           \
        info_frame[count] = __PRETTY_FUNCTION__;                                                                       \
        count++;                                                                                                       \
    }                                                                                                                  \
    if (addition_info_map[FILE_NAME].enabled) {                                                               \
        info_frame[count] = __FILE_NAME__;                                                                             \
        count++;                                                                                                       \
    }                                                                                                                  \
    if (addition_info_map[LINE].enabled) {                                                               \
        info_frame[count] = std::to_string(__LINE__);                                                                  \
        count++;                                                                                                       \
    }                                                                                                                  \
    if (addition_info_map[TIME].enabled) {                                                               \
        info_frame[count] = current_time_to_string();                                                                  \
        count++;                                                                                                       \
    }                                                                                                                  \
    switch (count) {                                                                                                   \
        case 1: fmt_log(head + fmt_str + get_chunk(), __VA_ARGS__, info_frame[0]); break;                              \
        case 2: fmt_log(head + fmt_str + get_chunk(), __VA_ARGS__, info_frame[0], info_frame[1]); break;               \
        case 3: fmt_log(head + fmt_str + get_chunk(), __VA_ARGS__, info_frame[0], info_frame[1], info_frame[2]);       \
            break;                                                                                                     \
        case 4:                                                                                                        \
            fmt_log(head + fmt_str + get_chunk(), __VA_ARGS__, info_frame[0], info_frame[1], info_frame[2],            \
                    info_frame[3]);                                                                                    \
            break;                                                                                                     \
        default: fmt_log(head + fmt_str, __VA_ARGS__);                                                                 \
    }

int main() {
    enable(FUNCTION);
    enable(LINE);
    enable(FILE_NAME);
    enable(TIME);
    LOG("Hello, {}! {}", "world", 10);
    disable(FUNCTION);
    LOG("Hello, {}! {}", "world", 10);
    disable(TIME);
    LOG("Hello, {}! {}", "world", 10);
    disable(FILE_NAME);
    LOG("Hello, {}! {}", "world", 10);
    disable(LINE);
    LOG("Hello, {}! {}", "world", 10);

    return 0;
}
