/******************************************************************************
 *
 * Copyright (c) 2019, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */
#ifdef PSP_ENABLE_PYTHON

#include <perspective/base.h>
#include <perspective/binding.h>
#include <perspective/python/base.h>
#include <perspective/python/utils.h>

namespace perspective {
namespace binding {

/******************************************************************************
 *
 * Date Parsing
 */
t_date
pythondate_to_t_date(t_val date) {
    return t_date(date.attr("year").cast<std::int32_t>(),
        date.attr("month").cast<std::int32_t>(),
        date.attr("day").cast<std::int32_t>());
}

} //namespace binding
} //namespace perspective

#endif