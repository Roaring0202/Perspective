/******************************************************************************
 *
 * Copyright (c) 2019, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

#pragma once
#include <perspective/first.h>
#include <perspective/exports.h>
#include <iostream>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace perspective {
struct PERSPECTIVE_EXPORT t_date_parser {
    t_date_parser(std::string format);

    bool operator()(std::string const& datestring);

    private:
        boost::posix_time::ptime pt;
        std::stringstream ss;
};
} // end namespace perspective