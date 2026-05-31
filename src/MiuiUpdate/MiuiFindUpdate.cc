#include "MiuiKeys.h"
#include "MiuiUpdate.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <format>
#include <string>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

static const json kRequestTemplate = {
    {"a",      0},
    {"c",      ""},
    {"b",      "F"},
    {"d",      ""},
    {"g",      "00000000000000000000000000000000"},
    {"i",      "0000000000000000000000000000000000000000000000000000000000000000"},
    {"isR",    "0"},
    {"f",      "1"},
    {"l",      "en_US"},
    {"n",      ""},
    {"sys",    "0"},
    {"unlock", "0"},
    {"r",      "CN"},
    {"sn",     "0x00000000"},
    {"v",      ""},
    {"bv",     ""},
    {"id",     ""}};

struct CurlSlistGuard {
    curl_slist *list = nullptr;

    curl_slist *append(const char *value) {
        list = curl_slist_append(list, value);
        return list;
    }

    ~CurlSlistGuard() {
        if (list) curl_slist_free_all(list);
    }
};

struct CurlHandleGuard {
    CURL *handle = nullptr;

    explicit CurlHandleGuard() : handle(curl_easy_init()) {}
    ~CurlHandleGuard() {
        if (handle) curl_easy_cleanup(handle);
    }

    explicit operator bool() const { return handle != nullptr; }
};

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

static std::string buildOsVersion(const DeviceData &data) {
    const std::string versionBlock =
        std::string(data.version.osVer) + std::to_string(data.version.major);

    std::string fullVersion;
    if (data.version.osVer[0] == 'V') {
        fullVersion = "MIUI-";
    }

    fullVersion += std::vformat(
        "{}.{}.{}.{}.{}{}{}{}",
        std::make_format_args(
            versionBlock,
            data.version.minor,
            data.version.patch,
            data.version.build,
            data.version.androidLetter,
            data.version.deviceLetter,
            data.version.regionLetter,
            data.version.carier));

    return fullVersion;
}

std::string MiuiUpdater::requestForUpdate(const std::string &requestData) {
    std::string response;

    auto encrypted = aes128cbcEncrypt(requestData);
    auto encoded   = base64Encrypt(encrypted);

    CurlHandleGuard curlGuard;
    if (!curlGuard) {
        std::cerr << "MiuiUpdater::requestForUpdate(): failed to initialise curl handle\n";
        return "";
    }

    CURL *curl = curlGuard.handle;

    char *escapedRaw = curl_easy_escape(curl, encoded.c_str(), static_cast<int>(encoded.length()));
    if (!escapedRaw) {
        std::cerr << "MiuiUpdater::requestForUpdate(): curl_easy_escape failed\n";
        return "";
    }
    const std::string postData = "q=" + std::string(escapedRaw) + "&t=&s=1";
    curl_free(escapedRaw);

    CurlSlistGuard headers;
    headers.append("clientId: MITUNES");
    headers.append("Connection: Keep-Alive");
    headers.append("Accept-Encoding: identity");
    headers.append("Content-Type: application/x-www-form-urlencoded");

    curl_easy_setopt(curl, CURLOPT_URL,           MIUI_OTA_URL);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,     "MiTunes_UserAgent_v3");
    curl_easy_setopt(curl, CURLOPT_POST,          1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    postData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(postData.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers.list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);

    const CURLcode r = curl_easy_perform(curl);
    if (r != CURLE_OK) {
        std::cerr << "MiuiUpdater::requestForUpdate(): " << curl_easy_strerror(r) << '\n';
        return "";
    }

    auto decryptedBase64 = base64Decrypt(response);
    return aes128cbcDecrypt(decryptedBase64);
}

std::string MiuiUpdater::getLatestUpdate(const std::string &device,
                                         const std::string &version) {
    if (!parseVersion(version))
        return "";

    json requestJson      = kRequestTemplate;
    requestJson["d"]      = device + data.regionFull;
    requestJson["c"]      = data.codebase;
    requestJson["bv"]     = data.bv;
    requestJson["v"]      = buildOsVersion(data);

    return requestForUpdate(requestJson.dump());
}
