#pragma once

#include <filesystem>

namespace LEGO::Application
{

class SentryClient
{
public:
    SentryClient(const std::filesystem::path& databaseParentPath, bool isOfficialRelease, const std::string& version,
                 const std::string& gitSHA);
    SentryClient(const SentryClient&) = delete;
    ~SentryClient();

    void CaptureMessageEvent(const char* logger, const char* message);
};

} // namespace LEGO::Application
