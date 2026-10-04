module;

#include <expected>
#include <string>

export module mitool.core.error;

export namespace mitool::core {

enum class ErrorCode {
  InvalidArgument,
  ParseError,
  NetworkError,
  NoResponse,
  NoUpdateFound,
  CryptoError,
  IoError,
  HashMismatch,
  NotImplemented,
};

struct Error {
  ErrorCode code;
  std::string message;
};

template <typename T> using Result = std::expected<T, Error>;

} // namespace mitool::core
