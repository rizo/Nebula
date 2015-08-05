/**
   \file prelude.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "prelude.hpp"

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

namespace nebula {

namespace logging {

void initialize() { core::get()->set_logging_enabled(false); }

void initialize(const fs::path &output_path, Severity minimum_severity) {
  register_simple_formatter_factory<Severity, char>("Severity");

  // We'll limit ourselves to 100 MB files.
  add_file_log(
      keywords::file_name = output_path.string(),
      keywords::format = "<%Severity%> [%TimeStamp%] * %Channel%: %Message%",
      keywords::filter = trivial::severity >= minimum_severity,
      keywords::rotation_size = 1024u * 1024u * 100u);

  add_common_attributes();
}

}  // namespace logging

}  // namespace nebula
