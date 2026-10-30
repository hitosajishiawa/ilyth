#include <ilyth/logger/log.h>
#include "ilyth/logger/appender.h"
#include "ilyth/logger/formatter.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>

int main()
{
    auto logger                = std::make_shared<ilyth::Logger>("test");
    const auto stdout_appender = ilyth::MakeStdoutAppender();
    logger->addAppender(stdout_appender);

    const auto output =
        std::filesystem::temp_directory_path() / "test_logger.log";
    {
        // truncate the log in the last run
        std::ofstream _(output, std::ios::trunc);
    }
    const auto file_appender = ilyth::MakeFileAppender(output);
    logger->addAppender(file_appender);

    auto all_level = [&]() {
        ILYTH_LOG_DEBUG(logger, "debug message");
        ILYTH_LOG_INFO(logger, "info message");
        ILYTH_LOG_WARN(logger, "warning message");
        ILYTH_LOG_ERROR(logger, "error message");
        ILYTH_LOG_FATAL(logger, "fatal message");
    };

    all_level();

    logger->setFormatter(&ilyth::defaultCompactFormatter);
    ILYTH_LOG_INFO(logger,
                   "Formatter has been changed to "
                   "ilyth::defaultCompactFormatter");

    all_level();

    const auto c    = 'a';
    const auto i    = 42;
    const auto msg1 = "hello";
    const auto msg2 = std::string{"world"};
    const auto msg3 = std::string_view{"ilyth"};
    ILYTH_LOG_DEBUG(logger, "c={}  i={}  msg1={}  msg2={}  msg3={}", c, i, msg1,
                    msg2, msg3);
    logger->sync();
    logger->removeAppender(file_appender);
    ILYTH_LOG_INFO(logger, "\n> The same records have been written to: {}",
                   output.string());
}
