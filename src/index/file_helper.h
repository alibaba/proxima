/**
 *   Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    Helper class for index file or directory operations
 */

#pragma once

#include <stdint.h>
#include <string>
#include <ailego/io/file.h>
#include <ailego/utility/file_helper.h>
#include <ailego/utility/string_helper.h>

namespace proxima {
namespace be {
namespace index {

/*
 * File type and id
 */
enum class FileID : uint32_t {
  UNDEFINED = 0,
  ID_FILE,
  DELETE_FILE,
  FORWARD_FILE,
  PROXIMA_FILE,
  SEGMENT_FILE,
  LSN_FILE,
  MANIFEST_FILE
};

/*
 * File name coresponding to file id
 */
static const char *GetFileName(FileID t) {
  switch (t) {
    case FileID::ID_FILE:
      return "data.id";
    case FileID::DELETE_FILE:
      return "data.del";
    case FileID::FORWARD_FILE:
      return "data.fwd";
    case FileID::PROXIMA_FILE:
      return "data.pxa";
    case FileID::SEGMENT_FILE:
      return "data.seg";
    case FileID::LSN_FILE:
      return "data.lsn";
    case FileID::MANIFEST_FILE:
      return "data.manifest";
    default:
      return "UnknownFile";
  };
}

/*
 * This helper class is mainly to wrapper filesystem operations.
 */
class FileHelper {
 public:
  //! Make file path with ${prefix_path}/${file_name}
  static std::string MakeFilePath(const std::string &prefix_path,
                                  FileID file_id) {
    return ailego::StringHelper::Concat(prefix_path, "/", GetFileName(file_id));
  }

  //! Make file path with ${prefix_path}/${file_name}.${number}
  static std::string MakeFilePath(const std::string &prefix_path,
                                  FileID file_id, uint32_t number) {
    return ailego::StringHelper::Concat(prefix_path, "/", GetFileName(file_id),
                                        ".", number);
  }

  //! Make file path with ${prefix_path}/${file_name}.${suffix_name}.${number}
  static std::string MakeFilePath(const std::string &prefix_path,
                                  FileID file_id, uint32_t number,
                                  const std::string &suffix_name) {
    return ailego::StringHelper::Concat(prefix_path, "/", GetFileName(file_id),
                                        ".", suffix_name, ".", number);
  }

  //! Create directory
  static bool CreateDirectory(const std::string &dir_path) {
    return ailego::File::MakePath(dir_path);
  }

  //! Remove directory
  static bool RemoveDirectory(const std::string &dir_path) {
    return ailego::File::RemoveDirectory(dir_path);
  }

  //! Remove file
  static bool RemoveFile(const std::string &file_path) {
    return ailego::File::Delete(file_path);
  }

  //! Check if file exists
  static bool FileExists(const std::string &file_path) {
    return ailego::File::IsExist(file_path);
  }

  //! Check if directory exists
  static bool DirectoryExists(const std::string &dir_path) {
    return ailego::File::IsExist(dir_path);
  }

  //! Return file size
  static size_t FileSize(const std::string &file_path) {
    return ailego::FileHelper::FileSize(file_path.c_str());
  }
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
