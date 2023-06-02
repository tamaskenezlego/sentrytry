#include "SentryClient.h"

#include <sentry.h>

#include <cstdio>
#include <cassert>
#include <string>
#include <atomic>
#include <filesystem>

namespace fs = std::filesystem;

namespace LEGO::Application
{
namespace
{
    std::atomic<bool> s_sentryInitialized = false;
    const char* kOfficialReleaseDSN = "https://c10075b6235d452797ab3adcc9a478b9@buggy.apps.lego.com/187";
    //const char* kDevelopmentDSN = "https://ec1a2eb3fa274c1b9d571a79d2a723ae@buggy.apps.lego.com/189";
const char* kDevelopmentDSN = "http://dbef2661b3f04be6bee1d5568a8c53ba@ec2-54-246-129-20.eu-west-1.compute.amazonaws.com:9000/3";
    const char* kSentryDatabaseFilename = "sentry-native";

    fs::path GetCrashpadHandlerPath()
    {
		return CRASHPAD_HANDLER_PROGRAM;
    }

#ifdef _WIN32

    // Copied from sentry internal function: sentry__logger_describe
    const char* SentryLevelToCString(sentry_level_t level)
    {
        switch (level)
        {
            case SENTRY_LEVEL_DEBUG:
                return "DEBUG ";
            case SENTRY_LEVEL_INFO:
                return "INFO ";
            case SENTRY_LEVEL_WARNING:
                return "WARN ";
            case SENTRY_LEVEL_ERROR:
                return "ERROR ";
            case SENTRY_LEVEL_FATAL:
                return "FATAL ";
            default:
                return "UNKNOWN ";
        }
    }

    // Based on sentry internal function: sentry__logger_defaultlogger
    void LogWithOutputDebugString(sentry_level_t level, const char* message, va_list args, void* /* userdata */)
    {
        const char* prefix = "[sentry] ";
        const char* priority = SentryLevelToCString(level);

        size_t len = strlen(prefix) + strlen(priority) + strlen(message) + 2;
        std::vector<char> format(len);
        snprintf(format.data(), len, "%s%s%s\n", prefix, priority, message);
        std::vector<char> buffer;
        int r = 127;
        for (int pass : {0, 1})
        {
            (void)pass;
            buffer.resize(r + 1);
            va_list argsCopy;
            va_copy(argsCopy, args);
            r = vsnprintf(buffer.data(), buffer.size(), format.data(), argsCopy);
            va_end(argsCopy);
            if (r < 0)
            {
                OutputDebugStringA("Failed to write sentry log, vsnprintf returned error.\n");
                assert(false);
                return;
            }
            if (r < buffer.size())
            {
                OutputDebugStringA(buffer.data());
                return;
            }
        }
        OutputDebugStringA("Failed to write sentry log, buffer is too small for vsnprintf.\n");
        assert(false);
    }
#endif
} // namespace

SentryClient::SentryClient(const fs::path& databaseParentPath, bool isOfficialRelease, const std::string& version,
                           const std::string& gitSHA)
{
    if (s_sentryInitialized.exchange(true))
    {
        throw std::runtime_error("SentryClient has already been initialized.");
    }
    sentry_options_t* options = sentry_options_new();
    sentry_options_set_dsn(options, isOfficialRelease ? kOfficialReleaseDSN : kDevelopmentDSN);
    std::string release = version;
    if (!gitSHA.empty())
    {
        release += "-" + gitSHA.substr(0, 7);
    }
    sentry_options_set_release(options, release.c_str());

#ifndef NDEBUG
    sentry_options_set_debug(options, 1);
#ifdef _WIN32
    sentry_options_set_logger(options, &LogWithOutputDebugString, nullptr);
#endif
#endif

    auto databasePath = databaseParentPath / kSentryDatabaseFilename;

#ifdef _WIN32
    sentry_options_set_handler_pathw(options, GetCrashpadHandlerPath().c_str());
    sentry_options_set_database_pathw(options, databasePath.c_str());
#else
    sentry_options_set_handler_path(options, GetCrashpadHandlerPath().c_str());
    sentry_options_set_database_path(options, databasePath.c_str());
#endif

    if (sentry_init(options))
    {
        fprintf(stderr, "ERROR: sentry_init failed.\n");
    }
}

void SentryClient::CaptureMessageEvent(const char* logger, const char* message)
{
    sentry_capture_event(sentry_value_new_message_event(SENTRY_LEVEL_INFO, logger, message));
}

SentryClient::~SentryClient()
{
    if (sentry_close())
    {
        fprintf(stderr, "ERROR: sentry_close() failed.\n");
        assert(false);
    }
}

} // namespace LEGO::Application
