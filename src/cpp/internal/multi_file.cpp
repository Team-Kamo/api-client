#include "include/internal/multi_file.h"

#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>

#include <algorithm>
#include <cstring>

#include "include/error_code.h"

namespace octane::internal {
  namespace {
    Result<FILE*, ErrorResponse> tar(const std::vector<FileInfo>& files) {
      archive* arch        = nullptr;
      archive_entry* entry = nullptr;
      auto file            = tmpfile();

      const auto defer = [&]() {
        if (entry) {
          archive_entry_free(entry);
          entry = nullptr;
        }
        if (arch) {
          archive_write_close(arch);
          archive_write_free(arch);
        }
      };

      int res = ARCHIVE_OK;

      arch = archive_write_new();

      if (res == ARCHIVE_OK) {
        res = archive_write_set_format_pax_restricted(arch);
      }
      if (res == ARCHIVE_OK) {
        res = archive_write_open_FILE(arch, file);
      }

      if (res == ARCHIVE_OK) {
        entry = archive_entry_new();
      }

      if (res == ARCHIVE_OK) {
        for (const auto& file : files) {
          archive_entry_set_pathname(entry, file.filename.c_str());
          archive_entry_set_size(entry, file.data.size());
          archive_entry_set_filetype(entry, AE_IFREG);
          res = archive_write_header(arch, entry);
          archive_write_data(arch, file.data.data(), file.data.size());
          archive_entry_clear(entry);
          if (res != ARCHIVE_OK) {
            break;
          }
        }
      }

      if (res != ARCHIVE_OK) {
        auto e = archive_error_string(arch);
        if (e == nullptr) {
          e = strerror(archive_errno(arch));
        }

        defer();
        return makeError(ERR_COMPRESSION_FAILED, e);
      }

      defer();

      return ok(file);
    }
  } // namespace
  std::uint8_t buf[16 * 1024 * 1024];

  Result<std::vector<uint8_t>, ErrorResponse> MultiFileCompressor::compress(
    const std::vector<FileInfo>& files) {
    auto tarResult = tar(files);
    if (!tarResult) {
      return error(tarResult.err());
    }
    auto tarFile = tarResult.get();
    auto len     = ftell(tarFile);
    fseek(tarFile, 0, SEEK_SET);

    std::vector<uint8_t> data;
    data.reserve(len);
    while (auto size = fread(buf, 1, sizeof(buf), tarFile)) {
      std::copy_n(buf, size, std::back_inserter(data));
    }
    fclose(tarFile);

    return ok(std::move(data));
  }
  Result<std::vector<FileInfo>, ErrorResponse>
  MultiFileDecompressor::decompress(const std::vector<uint8_t>& data) {
    archive* arch        = nullptr;
    archive_entry* entry = nullptr;
    std::vector<FileInfo> files;

    const auto defer = [&]() {
      if (arch) {
        archive_read_close(arch);
        archive_read_free(arch);
      }
    };

    int res = ARCHIVE_OK;

    arch = archive_read_new();

    if (res == ARCHIVE_OK) {
      res = archive_read_support_format_all(arch);
    }
    if (res == ARCHIVE_OK) {
      res = archive_read_open_memory(arch, data.data(), data.size());
    }
    while (res == ARCHIVE_OK) {
      res = archive_read_next_header(arch, &entry);
      if (res == ARCHIVE_EOF) {
        res = ARCHIVE_OK;
        break;
      }
      if (res == ARCHIVE_OK) {
        FileInfo file;
        file.filename = archive_entry_pathname(entry);

        const void* buf;
        size_t size;
        la_int64_t offset;
        while (res == ARCHIVE_OK) {
          res = archive_read_data_block(arch, &buf, &size, &offset);
          if (res == ARCHIVE_EOF) {
            res = ARCHIVE_OK;
            break;
          }
          if (res == ARCHIVE_OK) {
            file.data.reserve(file.data.size() + size);
            std::copy_n((uint8_t*)buf, size, std::back_inserter(file.data));
          }
        }

        files.push_back(std::move(file));
      }
    }

    if (res != ARCHIVE_OK) {
      auto e = archive_error_string(arch);
      if (e == nullptr) {
        e = strerror(archive_errno(arch));
      }

      defer();
      return makeError(ERR_DECOMPRESSION_FAILED, e);
    }

    defer();

    return ok(std::move(files));
  }
} // namespace octane::internal