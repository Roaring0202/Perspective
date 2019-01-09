/******************************************************************************
 *
 * Copyright (c) 2019, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

#include <perspective/first.h>
#include <perspective/date_parser.h>
#include <memory>
#include <boost/date_time/local_time/local_time.hpp>

namespace perspective {
t_date_parser::t_date_parser(std::string format) {
    // TODO: add unit tests
    auto input_facet = std::make_shared<boost::local_time::local_time_input_facet>();
    input_facet->format(format.c_str());
    // TODO: imbue input facet
    ss.imbue(std::locale(ss.getloc(), input_facet.get()));
}

bool 
t_date_parser::operator()(std::string const& datestring) {
    ss.clear();
    ss.str(datestring);
    ss >> pt;
    return !pt.is_not_a_date_time();
}
} // end namespace perspective