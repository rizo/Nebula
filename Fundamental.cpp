#include "Fundamental.hpp"

#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace nebula {

namespace logging {

void initialize( bool enable, Severity minimumSeverity ) {
    if ( ! enable ) {
        core::get()->set_logging_enabled( false );
        return;
    }

    logging::register_simple_formatter_factory<Severity, char>( "Severity" );
    
    logging::add_console_log( std::clog,
                              logging::keywords::filter = trivial::severity >= minimumSeverity,
                              logging::keywords::format = "<%Severity%> [%TimeStamp%] * %Channel%: %Message%" );

    logging::add_common_attributes();
}

}

}
