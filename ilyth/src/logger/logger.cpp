#include "logger/logger_impl.h"

#include <ilyth/logger/detail/buffer.h>

#include <cstdio>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "logger/appender_impl.h"

namespace ilyth
{

Logger::Logger(std::string name)
    : mImpl(std::make_unique<Impl>(std::move(name)))
{}

Logger::~Logger() = default;

auto Logger::shouldLog(LogLevel::Level level) const noexcept -> bool
{
    return mImpl->shouldLog(level);
}
void Logger::setFilter(LogLevel::Level level) noexcept
{
    mImpl->setFilter(level);
}
auto Logger::getFilter() const noexcept -> LogLevel::Level
{
    return mImpl->getFilter();
}
auto Logger::getName() const noexcept -> std::string_view
{
    return mImpl->getName();
}
void Logger::setFormatter(EntryFormatter formatter)
{
    mImpl->setFormatter(formatter);
}
void Logger::addAppender(AppenderPtr appender)
{
    mImpl->addAppender(std::move(appender));
}
void Logger::removeAppender(const AppenderPtr& appender)
{
    mImpl->removeAppender(appender);
}
void Logger::clearAppenders()
{
    mImpl->clearAppenders();
}
void Logger::flush()
{
    mImpl->flush();
}
void Logger::sync()
{
    mImpl->sync();
}
void Logger::submit(const LogEntry& entry) noexcept
{
    mImpl->submit(entry);
}

Logger::Impl::Impl(std::string name) : mName(std::move(name))
{
    if (mName.empty())
    {
        throw std::invalid_argument("logger name cannot be empty");
    }
}

auto Logger::Impl::shouldLog(LogLevel::Level level) const noexcept -> bool
{
    if (static_cast<types::u8>(level) < static_cast<types::u8>(getFilter()))
    {
        return false;
    }
    {
        std::shared_lock<std::shared_mutex> _(mMutex);
        if (!mAppenders.empty())
        {
            return true;
        }
    }
    return mRoot != nullptr && mRoot->shouldLog(level);
}

void Logger::Impl::submit(const LogEntry& entry) noexcept
{
    if (static_cast<types::u8>(entry.level) <
        static_cast<types::u8>(getFilter()))
    {
        return;
    }

    std::shared_lock<std::shared_mutex> lock(mMutex);
    if (mAppenders.empty())
    {
        lock.unlock();
        if (mRoot != nullptr)
        {
            mRoot->mImpl->submit(entry);
        }
        return;
    }

    // The formatter runs under the shared lock (so submissions render in
    // parallel) and before the appender mutex; only the write is serialized.
    try
    {
        detail::LoggerBuffer<detail::FormattedEntryInlineCapacity> record;
        detail::formatTwoPass(record, [&](std::span<char> out) -> std::size_t {
            return mFormatter(entry, out);
        });
        for (const AppenderPtr& appender : mAppenders)
        {
            detail::AppenderAccess::Append(appender, entry.level,
                                           record.view());
        }
    }
    catch (...)
    {
        constexpr std::string_view message =
            "ilyth logger: record formatting failed\n";
        std::fwrite(message.data(), 1, message.size(), stderr);
    }
}

void Logger::Impl::setFilter(LogLevel::Level level) noexcept
{
    mFilter.store(level, std::memory_order_release);
}

auto Logger::Impl::getFilter() const noexcept -> LogLevel::Level
{
    return mFilter.load(std::memory_order_acquire);
}

auto Logger::Impl::getName() const noexcept -> std::string_view
{
    return mName;
}

void Logger::Impl::setFormatter(EntryFormatter formatter)
{
    if (formatter == nullptr)
    {
        return;
    }
    std::unique_lock<std::shared_mutex> _(mMutex);
    mFormatter = formatter;
}

void Logger::Impl::addAppender(AppenderPtr appender)
{
    if (appender == nullptr)
    {
        return;
    }
    std::unique_lock<std::shared_mutex> _(mMutex);
    mAppenders.push_back(std::move(appender));
}

void Logger::Impl::removeAppender(const AppenderPtr& appender)
{
    std::unique_lock<std::shared_mutex> _(mMutex);
    std::erase(mAppenders, appender);
}

void Logger::Impl::clearAppenders()
{
    std::unique_lock<std::shared_mutex> _(mMutex);
    mAppenders.clear();
}

void Logger::Impl::flush()
{
    std::shared_lock<std::shared_mutex> lock(mMutex);
    if (mAppenders.empty())
    {
        lock.unlock();
        if (mRoot != nullptr)
        {
            mRoot->flush();
        }
        return;
    }
    for (const AppenderPtr& appender : mAppenders)
    {
        appender->flush();
    }
}

void Logger::Impl::sync()
{
    std::shared_lock<std::shared_mutex> lock(mMutex);
    if (mAppenders.empty())
    {
        lock.unlock();
        if (mRoot != nullptr)
        {
            mRoot->sync();
        }
        return;
    }
    for (const AppenderPtr& appender : mAppenders)
    {
        appender->sync();
    }
}

void Logger::Impl::setRoot(const LoggerPtr& root) noexcept
{
    mRoot = root;
}

void detail::LoggerAccess::SetRoot(Logger& logger,
                                   const LoggerPtr& root) noexcept
{
    logger.mImpl->setRoot(root);
}

namespace
{

/// Owns the root logger and the named logger registry.
class LoggerManager final
{
public:
    LoggerManager() : mRoot(std::make_shared<Logger>("root"))
    {
        mRoot->addAppender(MakeStdoutAppender());
        mLoggers.emplace("root", mRoot);
    }

    [[nodiscard]] auto getRoot() const noexcept -> LoggerPtr { return mRoot; }

    [[nodiscard]] auto getLogger(std::string_view name) -> LoggerPtr
    {
        if (name.empty())
        {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(mMutex);
        const auto existing = mLoggers.find(std::string(name));
        if (existing != mLoggers.end())
        {
            return existing->second;
        }
        auto logger = std::make_shared<Logger>(std::string(name));
        detail::LoggerAccess::SetRoot(*logger, mRoot);
        mLoggers.emplace(std::string(name), logger);
        return logger;
    }

private:
    mutable std::mutex mMutex;
    std::unordered_map<std::string, LoggerPtr> mLoggers;
    LoggerPtr mRoot;
};

auto manager() -> LoggerManager&
{
    static LoggerManager instance;
    return instance;
}

}  // namespace

auto GetRootLogger() -> LoggerPtr
{
    return manager().getRoot();
}

auto GetLogger(std::string_view name) -> LoggerPtr
{
    return manager().getLogger(name);
}

}  // namespace ilyth
