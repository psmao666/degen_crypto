#pragma once

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"
#include <iostream>
namespace degen_crypto {
namespace logger {

// Global logger instance
extern quill::Logger* g_logger;

inline void init_logger() {
    quill::BackendOptions backend_options;
    quill::Backend::start(backend_options);
    auto now = std::chrono::system_clock::now();
    auto zoned = std::chrono::zoned_time{std::chrono::current_zone(), now};
    auto local_today = floor<std::chrono::days>(zoned.get_local_time());
    std::chrono::year_month_day ymd{local_today};

    // Create file sink
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
        "logs/" + std::format("{:%Y%m%d}", ymd) + "/controller.log",
        []() {
            quill::FileSinkConfig cfg;
            cfg.set_open_mode('w');
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartDate);
            return cfg;
        }(),
        quill::FileEventNotifier{});

    // Create console sink
    auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink");

    // Create logger with both sinks
    g_logger = quill::Frontend::create_or_get_logger(
        "root", 
        std::vector{std::move(file_sink), std::move(console_sink)},
        quill::PatternFormatterOptions{"%(time) [%(thread_id)] %(short_source_location:<28) "
                                     "LOG_%(log_level:<9) %(logger:<12) %(message)",
                                     "%H:%M:%S.%Qns", quill::Timezone::LocalTime});
}

} // namespace logger
} // namespace degen_crypto 