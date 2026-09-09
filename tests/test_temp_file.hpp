#pragma once

#include <unistd.h>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>


// A temp-file name no other process is holding.
//
// The fixtures that write to /tmp used fixed names -- chesso_test_tuner_split
// .tsv and four more -- so two `ctest` runs on the same machine wrote the same
// file, and one of them read what the other had just truncated. Nothing in the
// tree runs ctest twice at once today, which is exactly why the collision would
// be found as a mystery failure rather than as a collision.
//
// mkstemp(3) creates the file, so the caller gets an existing empty file: every
// current caller opens it with std::ofstream, which is fine on one, and the
// `path_after` check in tests/test_tuner_split.cpp still sees it removed.
// A caller that wants a name that does *not* exist appends a suffix.
//
// S193, 2026-09-04_test_review-F09.
inline std::string unique_fixture_path(const std::string& prefix)
{
  const std::string tmpl =
      (std::filesystem::temp_directory_path() / (prefix + ".XXXXXX")).string();

  std::vector<char> buffer(tmpl.begin(), tmpl.end());
  buffer.push_back('\0');

  const int fd = mkstemp(buffer.data());
  if (fd < 0) { throw std::runtime_error("mkstemp failed for " + tmpl); }
  close(fd);

  return std::string(buffer.data());
}
