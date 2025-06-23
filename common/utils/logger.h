#pragma once

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"

namespace degen_crypto {
namespace logger {

// Global logger instance
extern quill::Logger* g_logger;

inline void init_logger() {
    quill::BackendOptions backend_options;
    quill::Backend::start(backend_options);

    // Create file sink
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
        "logs/" + std::string(std::getenv("TODAY")) + "/controller.log",
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
                                     "%H:%M:%S.%Qns", quill::Timezone::GmtTime});
}

} // namespace logger
} // namespace degen_crypto 