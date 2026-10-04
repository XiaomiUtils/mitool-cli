module;

#include <filesystem>
#include <string>
#include <string_view>

export module mitool.utils.hash_verifier;

import mitool.core.error;

export namespace mitool::utils {

// Returns the file's MD5 as a lowercase hex string.
core::Result<std::string> computeMd5(const std::filesystem::path &file);

// Compares the file's MD5 with the expected value (case-insensitive).
core::Result<void> verifyMd5(const std::filesystem::path &file,
                             std::string_view expected);

} // namespace mitool::utils
