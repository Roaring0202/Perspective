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
#include <perspective/python/accessor.h>
#include <perspective/python/base.h>
#include <perspective/python/utils.h>

namespace perspective {
namespace binding {

/******************************************************************************
 *
 * Data accessor API
 */

std::vector<std::string>
get_column_names(t_val data, std::int32_t format) {
    std::vector<std::string> names;
    if (format == 0) {
        // record
        py::list data_list = data.cast<py::list>();
        std::int32_t max_check = 50;

        if(data_list.size()){
            for(auto tup: data_list[0].cast<py::dict>()){
                names.push_back(tup.first.cast<std::string>());
            }
        }

        std::int32_t check_index = std::min(max_check, int32_t(data_list.size()));

        for (auto ix = 0; ix < check_index; ix++) {
            py::dict next_dict = data_list[ix].cast<py::dict>();
            auto old_size = names.size();

            for (auto tup: next_dict) {
                if (std::find(names.begin(), names.end(), tup.first.cast<std::string>()) == names.end()) {
                    names.push_back(tup.first.cast<std::string>());
                }
            }
            if (old_size != names.size()){
                if (max_check == 50) {
                    WARN("Data parse warning: Array data has inconsistent rows");
                }
                WARN("Extended from %d to %d",  old_size, names.size());
                max_check *= 2;
            }
        }
    } else if (format == 1 || format == 2) {
        py::dict data_dict = data.cast<py::dict>();
        for(auto tup: data_dict){
            names.push_back(tup.first.cast<std::string>());
        }
    }
    return names;
}

t_dtype
infer_type(t_val x, t_val date_validator) {
    std::string jstype = py::str(x.get_type());
    t_dtype t = t_dtype::DTYPE_STR;

    if (x.is_none()) {
        t = t_dtype::DTYPE_NONE;
    } else if (py::isinstance<py::int_>(x)) {
        double x_float64 = x.cast<double>();
        if ((std::fmod(x_float64, 1.0) == 0.0) && (x_float64 < 10000.0)
            && (x_float64 != 0.0)) {
            t = t_dtype::DTYPE_INT32;
        } else {
            t = t_dtype::DTYPE_FLOAT64;
        }
    } else if (py::isinstance<py::float_>(x)) {
        t = t_dtype::DTYPE_FLOAT64;
    } else if (py::isinstance<py::bool_>(x) || jstype == "bool") {
        t = t_dtype::DTYPE_BOOL;
    } else if (jstype == "datetime.datetime") {
        // TODO allow derived types
        t = t_dtype::DTYPE_TIME;
    } else if (jstype == "datetime.date") {
        // TODO allow derived types
        t = t_dtype::DTYPE_DATE;
    } else if (py::isinstance<py::str>(x) || jstype == "string") {
        if (date_validator.attr("check")(x).cast<bool>()) {
            t = t_dtype::DTYPE_TIME;
        } else {
            std::string lower = x.attr("lower")().cast<std::string>();
            if (lower == "true" || lower == "false") {
                t = t_dtype::DTYPE_BOOL;
            } else {
                t = t_dtype::DTYPE_STR;
            }
        }
    }
    return t;
}

t_dtype
get_data_type(
    t_val data, std::int32_t format, py::str name, t_val date_validator) {
    std::int32_t i = 0;
    boost::optional<t_dtype> inferredType;

    if (format == 0) {
        py::list data_list = data.cast<py::list>();

        // loop parameters differ slightly so rewrite the loop
        while (!inferredType.is_initialized() && i < 100
            && i < data_list.size()) {
            if (!data_list.is_none()) {
                if (!data_list[i].cast<py::dict>()[name].is_none()) {
                    inferredType = infer_type(data_list[i].cast<py::dict>()[name].cast<t_val>(), date_validator);
                }
            }
            i++;
        }
    } else if (format == 1) {
        py::dict data_dict = data.cast<py::dict>();

        while (!inferredType.is_initialized() && i < 100
            && i < data_dict[name].cast<py::list>().size()) {
            if (!data_dict[name].cast<py::list>()[i].is_none()) {
                inferredType = infer_type(data_dict[name].cast<py::list>()[i].cast<t_val>(), date_validator);
            }
            i++;
        }
    }

    if (!inferredType.is_initialized()) {
        return t_dtype::DTYPE_STR;
    } else {
        return inferredType.get();
    }
}

std::vector<t_dtype>
get_data_types(t_val data, std::int32_t format, std::vector<std::string> names,
    t_val date_validator) {
    std::vector<t_dtype> types;

    if (names.size() == 0) {
        WARN("Cannot determine data types without column names!");
        return types;
    }


    if (format == 2) {
        py::dict data_dict = data.cast<py::dict>();

        for (auto tup : data_dict) {
            auto name = tup.first.cast<std::string>();
            auto value = py::str(tup.second.cast<py::object>().attr("__name__")).cast<std::string>();
            t_dtype type;

            if (name == "__INDEX__") {
                WARN("Warning: __INDEX__ column should not be in the Table schema.");
                continue;
            }

            // TODO consider refactor
            if (value == "int") {
                // Python int
                type = t_dtype::DTYPE_INT64;
            } else if (value == "int8") {
                // Numpy int8
                type = t_dtype::DTYPE_INT8;
            } else if (value == "int16") {
                // Numpy int16
                type = t_dtype::DTYPE_INT16;
            } else if (value == "int32") {
                // Numpy int32
                type = t_dtype::DTYPE_INT32;
            } else if (value == "int64") {
                // Numpy int64
                type = t_dtype::DTYPE_INT64;
            } else if (value == "float") {
                // Python float
                type = t_dtype::DTYPE_FLOAT64;
            } else if (value == "float16") {
                // TODO
                // Numpy float16
                // type = t_dtype::DTYPE_FLOAT16;
            } else if (value == "float32") {
                // Numpy float32
                type = t_dtype::DTYPE_FLOAT32;
            } else if (value == "float64") {
                // Numpy float64
                type = t_dtype::DTYPE_FLOAT64;
            } else if (value == "float128") {
                // TODO
                // Numpy float128
                // type = t_dtype::DTYPE_FLOAT128;
            } else if (value == "str") {
                // Python unicode str
                type = t_dtype::DTYPE_STR;
            } else if (value == "bool") {
                // Python bool
                type = t_dtype::DTYPE_BOOL;
            } else if (value == "bool8") {
                // Numpy bool8
                type = t_dtype::DTYPE_BOOL;
            } else if (value == "datetime") {
                // Python datetime
                // TODO inheritance
                type = t_dtype::DTYPE_TIME;
            } else if (value == "datetime64") {
                // Numpy datetime64
                type = t_dtype::DTYPE_TIME;
            } else if (value == "Timestamp") {
                // Pandas timestamp
                type = t_dtype::DTYPE_TIME;
            } else if (value == "date") {
                // Python date
                // TODO inheritance
                type = t_dtype::DTYPE_DATE;
            } else {
                CRITICAL("Unknown type '%s' for key '%s'", value, name);
            }
            types.push_back(type);
        }

        return types;
    } else {
        for (auto name : names) {
            // infer type for each column
            t_dtype type = get_data_type(data, format, py::str(name), date_validator);
            types.push_back(type);
        }
    }

    return types;
}


/******************************************************************************
 *
 * Data serialization
 */

template <>
t_val
get_column_data(std::shared_ptr<t_data_table> table, const std::string& colname) {
    py::array arr = py::array();
    // TODO
    // auto col = table->get_column(colname);
    // for (auto idx = 0; idx < col->size(); ++idx) {
    //     arr[idx] = py::cast(col->get_scalar(idx));
    // }
    return arr;
}

template <typename CTX_T>
std::shared_ptr<t_data_slice<CTX_T>>
get_data_slice(std::shared_ptr<View<CTX_T>> view, std::uint32_t start_row,
    std::uint32_t end_row, std::uint32_t start_col, std::uint32_t end_col) {
    auto data_slice = view->get_data(start_row, end_row, start_col, end_col);
    return data_slice;
}

std::shared_ptr<t_data_slice<t_ctx0>>
get_data_slice_ctx0(std::shared_ptr<View<t_ctx0>> view, std::uint32_t start_row,
    std::uint32_t end_row, std::uint32_t start_col, std::uint32_t end_col) {
    return get_data_slice<t_ctx0>(view, start_row, end_row, start_col, end_col);
}

std::shared_ptr<t_data_slice<t_ctx1>>
get_data_slice_ctx1(std::shared_ptr<View<t_ctx1>> view, std::uint32_t start_row,
    std::uint32_t end_row, std::uint32_t start_col, std::uint32_t end_col) {
    return get_data_slice<t_ctx1>(view, start_row, end_row, start_col, end_col);
}

std::shared_ptr<t_data_slice<t_ctx2>>
get_data_slice_ctx2(std::shared_ptr<View<t_ctx2>> view, std::uint32_t start_row,
    std::uint32_t end_row, std::uint32_t start_col, std::uint32_t end_col) {
    return get_data_slice<t_ctx2>(view, start_row, end_row, start_col, end_col);
}

template <typename CTX_T>
t_val
get_from_data_slice(
    std::shared_ptr<t_data_slice<CTX_T>> data_slice, t_uindex ridx, t_uindex cidx) {
    auto d = data_slice->get(ridx, cidx);
    return py::cast(d);
}

t_val
get_from_data_slice_ctx0(
    std::shared_ptr<t_data_slice<t_ctx0>> data_slice, t_uindex ridx, t_uindex cidx) {
    return get_from_data_slice<t_ctx0>(data_slice, ridx, cidx);
}


t_val
get_from_data_slice_ctx1(
    std::shared_ptr<t_data_slice<t_ctx1>> data_slice, t_uindex ridx, t_uindex cidx) {
    return get_from_data_slice<t_ctx1>(data_slice, ridx, cidx);
}

t_val
get_from_data_slice_ctx2(
    std::shared_ptr<t_data_slice<t_ctx2>> data_slice, t_uindex ridx, t_uindex cidx) {
    return get_from_data_slice<t_ctx2>(data_slice, ridx, cidx);
}

} //namespace binding
} //namespace perspective

#endif