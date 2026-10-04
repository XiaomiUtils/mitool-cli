module;

#include <string_view>

export module mitool.utils.version_parser;

import mitool.core.error;
import mitool.core.types;

export namespace mitool::utils {

// Parses a version string like V12.0.16.0.QCDMIXM or OS2.0.3.0.PXCMIXM.
core::Result<core::VersionData> parseVersion(std::string_view input);

} // namespace mitool::utils
