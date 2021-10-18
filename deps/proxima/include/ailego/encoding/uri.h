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

 *   \author   Hechong.xyf
 *   \date     Nov 2020
 *   \brief    Interface of URI Parser
 */

#ifndef __AILEGO_ENCODING_URI_H__
#define __AILEGO_ENCODING_URI_H__

#include <string>

namespace ailego {

/*! URI module
 */
class Uri {
 public:
  //! Constructor (STL-string)
  Uri(const std::string &str) {
    Uri::Parse(str, this);
  }

  //! Constructor (C-string)
  Uri(const char *str) {
    Uri::Parse(str, this);
  }

  //! Constructor (Empty)
  Uri(void) {}

  //! Test if the Uri is valid
  bool is_valid(void) const {
    return valid_;
  }

  //! Retrieve the scheme part of URI
  const std::string &scheme(void) const {
    return scheme_;
  }

  //! Retrieve the authority part of URI
  const std::string &authority(void) const {
    return authority_;
  }

  //! Retrieve the username part of URI
  const std::string &username(void) const {
    return username_;
  }

  //! Retrieve the password part of URI
  const std::string &password(void) const {
    return password_;
  }

  //! Retrieve the host part of URI
  const std::string &host(void) const {
    return host_;
  }

  //! Retrieve the port part of URI
  int32_t port(void) const {
    return port_;
  }

  //! Retrieve the path part of URI
  const std::string &path(void) const {
    return path_;
  }

  //! Retrieve the query part of URI
  const std::string &query(void) const {
    return query_;
  }

  //! Retrieve the fragment part of URI
  const std::string &fragment(void) const {
    return fragment_;
  }

  //! Parse a URI string (C-string)
  static bool Parse(const char *str, Uri *out);

  //! Parse a URI string (STL-string)
  static bool Parse(const std::string &str, Uri *out) {
    return Uri::Parse(str.c_str(), out);
  }

 private:
  bool valid_{false};
  std::string scheme_{};
  std::string authority_{};
  std::string username_{};
  std::string password_{};
  std::string host_{};
  int32_t port_{};
  std::string path_{};
  std::string query_{};
  std::string fragment_{};
};


}  // namespace ailego

#endif  //__AILEGO_ENCODING_URI_H__
