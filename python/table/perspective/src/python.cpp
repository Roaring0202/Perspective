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
#include <perspective/python.h>
#include "Python.h"
#include <chrono>
#include <ctime>

// https://github.com/pybind/pybind11/issues/1598
// no global py::objects
#define WARN py::module::import("logging").attr("warning")
#define CRITICAL py::module::import("logging").attr("critical")

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

t_val
scalar_to_py(const t_tscalar& scalar, bool cast_double, bool cast_string) {
    if (!scalar.is_valid()) {
        return py::none();
    }
    
    switch (scalar.get_dtype()) {
        case DTYPE_BOOL: {
            if (scalar) {
                return py::cast(true);
            } else {
                return py::cast(false);
            }
        }
        case DTYPE_TIME: {
            if (cast_double) {
                auto x = scalar.to_uint64();
                double y = *reinterpret_cast<double*>(&x);
                return py::cast(y);
            } else if (cast_string) {
                double ms = scalar.to_double();
                return py::cast(ms);
                //t_val date = t_val::global("Date").new_(ms);
                //return date.call<t_val>("toLocaleString");
            } else {
                // TODO: should return python datetime
                return py::cast(scalar.to_double());
            }
        }
        case DTYPE_FLOAT64:
        case DTYPE_FLOAT32: {
            if (cast_double) {
                auto x = scalar.to_uint64();
                double y = *reinterpret_cast<double*>(&x);
                return py::cast(y);
            } else {
                return py::cast(scalar.to_double());
            }
        }
        case DTYPE_DATE: {
            CRITICAL("date return is not implemented"); // TODO: implement python datetime return
        }
        case DTYPE_UINT8:
        case DTYPE_UINT16:
        case DTYPE_UINT32:
        case DTYPE_INT8:
        case DTYPE_INT16:
        case DTYPE_INT32: {
            return py::cast(scalar.to_int64());
        }
        case DTYPE_UINT64:
        case DTYPE_INT64: {
            // This could potentially lose precision
            return py::cast(scalar.to_int64());
        }
        case DTYPE_NONE: {
            return py::none();
        }
        case DTYPE_STR:
        default: {
            std::wstring_convert<utf8convert_type, wchar_t> converter("", L"<Invalid>");
            return py::cast(scalar.to_string());
        }
    }
}

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
    std::string pytype = py::str(x.get_type());
    t_dtype t = t_dtype::DTYPE_STR;

    if (x.is_none()) {
        t = t_dtype::DTYPE_NONE;
    } else if (py::isinstance<py::bool_>(x) || pytype == "bool") { 
        t = t_dtype::DTYPE_BOOL;
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
    } else if (pytype == "datetime.datetime") {
        // TODO allow derived types
        t = t_dtype::DTYPE_TIME;
    } else if (pytype == "datetime.date") {
        // TODO allow derived types
        t = t_dtype::DTYPE_DATE;
    } else if (py::isinstance<py::str>(x) || pytype == "string") {
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
 * Fill columns with data
 */

// void
// _fill_col_time(t_data_accessor accessor, std::shared_ptr<t_column> col, const std::string& name,
//     std::int32_t cidx, t_dtype type, bool is_arrow, bool is_update) {
//     t_uindex nrows = col->size();

//     if (is_arrow) {
//         t_val data = accessor["values"];
//         // arrow packs 64 bit into two 32 bit ints
//         arrow::vecFromTypedArray(data, col->get_nth<t_time>(0), nrows * 2);

//         std::int8_t unit = accessor["type"]["unit"].as<std::int8_t>();
//         if (unit != /* Arrow.enum_.TimeUnit.MILLISECOND */ 1) {
//             // Slow path - need to convert each value
//             std::int64_t factor = 1;
//             if (unit == /* Arrow.enum_.TimeUnit.NANOSECOND */ 3) {
//                 factor = 1e6;
//             } else if (unit == /* Arrow.enum_.TimeUnit.MICROSECOND */ 2) {
//                 factor = 1e3;
//             }
//             for (auto i = 0; i < nrows; ++i) {
//                 col->set_nth<std::int64_t>(i, *(col->get_nth<std::int64_t>(i)) / factor);
//             }
//         }
//     } else {
//         for (auto i = 0; i < nrows; ++i) {
//             t_val item = accessor.call<t_val>("marshal", cidx, i, type);

//             if (item.isUndefined())
//                 continue;

//             if (item.isNull()) {
//                 if (is_update) {
//                     col->unset(i);
//                 } else {
//                     col->clear(i);
//                 }
//                 continue;
//             }

//             auto elem = static_cast<std::int64_t>(
//                 item.call<t_val>("getTime").as<double>()); // dcol[i].as<T>();
//             col->set_nth(i, elem);
//         }
//     }
// }

// void
// _fill_col_date(t_data_accessor accessor, std::shared_ptr<t_column> col, const std::string& name,
//     std::int32_t cidx, t_dtype type, bool is_arrow, bool is_update) {
//     t_uindex nrows = col->size();

//     if (is_arrow) {
//         // t_val data = dcol["values"];
//         // // arrow packs 64 bit into two 32 bit ints
//         // arrow::vecFromTypedArray(data, col->get_nth<t_time>(0), nrows * 2);

//         // std::int8_t unit = dcol["type"]["unit"].as<std::int8_t>();
//         // if (unit != /* Arrow.enum_.TimeUnit.MILLISECOND */ 1) {
//         //     // Slow path - need to convert each value
//         //     std::int64_t factor = 1;
//         //     if (unit == /* Arrow.enum_.TimeUnit.NANOSECOND */ 3) {
//         //         factor = 1e6;
//         //     } else if (unit == /* Arrow.enum_.TimeUnit.MICROSECOND */ 2) {
//         //         factor = 1e3;
//         //     }
//         //     for (auto i = 0; i < nrows; ++i) {
//         //         col->set_nth<std::int32_t>(i, *(col->get_nth<std::int32_t>(i)) / factor);
//         //     }
//         // }
//     } else {
//         for (auto i = 0; i < nrows; ++i) {
//             t_val item = accessor.call<t_val>("marshal", cidx, i, type);

//             if (item.isUndefined())
//                 continue;

//             if (item.isNull()) {
//                 if (is_update) {
//                     col->unset(i);
//                 } else {
//                     col->clear(i);
//                 }
//                 continue;
//             }

//             col->set_nth(i, pythondate_to_t_date(item));
//         }
//     }
// }

void
_fill_col_bool(t_data_accessor accessor, std::shared_ptr<t_column> col, const std::string& name,
    std::int32_t cidx, t_dtype type, bool is_arrow, bool is_update) {
    t_uindex nrows = col->size();

    if (is_arrow) {
        // TODO
        // bools are stored using a bit mask
        // t_val data = accessor["values"];
        // for (auto i = 0; i < nrows; ++i) {
        //     t_val item = data[i / 8];

        //     if (item.isUndefined()) {
        //         continue;
        //     }

        //     if (item.isNull()) {
        //         if (is_update) {
        //             col->unset(i);
        //         } else {
        //             col->clear(i);
        //         }
        //         continue;
        //     }

        //     std::uint8_t elem = item.as<std::uint8_t>();
        //     bool v = elem & (1 << (i % 8));
        //     col->set_nth(i, v);
        // }
    } else {
        for (auto i = 0; i < nrows; ++i) {
            t_val item = accessor.attr("marshal")(cidx, i, type);

            // TODO
            // if (item.isUndefined())
            //     continue;

            if (item.is_none()) {
                if (is_update) {
                    col->unset(i);
                } else {
                    col->clear(i);
                }
                continue;
            }

            auto elem = item.cast<bool>();
            col->set_nth(i, elem);
        }
    }
}

void
_fill_col_string(t_data_accessor accessor, std::shared_ptr<t_column> col, const std::string& name,
    std::int32_t cidx, t_dtype type, bool is_arrow, bool is_update) {

    t_uindex nrows = col->size();

    if (is_arrow) {
        // TODO
        // if (accessor["constructor"]["name"].as<std::string>() == "DictionaryVector") {

        //     t_val dictvec = accessor["dictionary"];
        //     arrow::fill_col_dict(dictvec, col);

        //     // Now process index into dictionary

        //     // Perspective stores string indices in a 32bit unsigned array
        //     // Javascript's typed arrays handle copying from various bitwidth arrays
        //     // properly
        //     t_val vkeys = accessor["indices"]["values"];
        //     arrow::vecFromTypedArray(
        //         vkeys, col->get_nth<t_uindex>(0), nrows, "Uint32Array");

        // } else if (accessor["constructor"]["name"].as<std::string>() == "Utf8Vector"
        //     || accessor["constructor"]["name"].as<std::string>() == "BinaryVector") {

        //     t_val vdata = accessor["values"];
        //     std::int32_t vsize = vdata["length"].as<std::int32_t>();
        //     std::vector<std::uint8_t> data;
        //     data.reserve(vsize);
        //     data.resize(vsize);
        //     arrow::vecFromTypedArray(vdata, data.data(), vsize);

        //     t_val voffsets = accessor["valueOffsets"];
        //     std::int32_t osize = voffsets["length"].as<std::int32_t>();
        //     std::vector<std::int32_t> offsets;
        //     offsets.reserve(osize);
        //     offsets.resize(osize);
        //     arrow::vecFromTypedArray(voffsets, offsets.data(), osize);

        //     std::string elem;

        //     for (std::int32_t i = 0; i < nrows; ++i) {
        //         std::int32_t bidx = offsets[i];
        //         std::size_t es = offsets[i + 1] - bidx;
        //         elem.assign(reinterpret_cast<char*>(data.data()) + bidx, es);
        //         col->set_nth(i, elem);
        //     }
        // }
    } else {
        for (auto i = 0; i < nrows; ++i) {
            t_val item = accessor.attr("marshal")(cidx, i, type);

            // TODO
            // if (item.isUndefined())
            //     continue;

            if (item.is_none()) {
                if (is_update) {
                    col->unset(i);
                } else {
                    col->clear(i);
                }
                continue;
            }

            std::wstring welem = item.cast<std::wstring>();
            std::wstring_convert<utf16convert_type, wchar_t> converter;
            std::string elem = converter.to_bytes(welem);
            col->set_nth(i, elem);
        }
    }
}

void
_fill_col_int64(t_data_accessor accessor, t_data_table& tbl, std::shared_ptr<t_column> col, const std::string& name,
    std::int32_t cidx, t_dtype type, bool is_arrow, bool is_update) {
    t_uindex nrows = col->size();

    if (is_arrow) {
        // TODO
        // t_val data = accessor["values"];
        // // arrow packs 64 bit into two 32 bit ints
        // arrow::vecFromTypedArray(data, col->get_nth<std::int64_t>(0), nrows * 2);
    } else {
        t_uindex nrows = col->size();
        for (auto i = 0; i < nrows; ++i) {
            t_val item = accessor.attr("marshal")(cidx, i, type);

            // TODO
            // if (item.isUndefined())
            //     continue;

            if (item.is_none()) {
                if (is_update) {
                    col->unset(i);
                } else {
                    col->clear(i);
                }
                continue;
            }

            double fval = item.cast<double>();
            if (isnan(fval)) {
                WARN("Promoting to string");
                tbl.promote_column(name, DTYPE_STR, i, false);
                col = tbl.get_column(name);
                _fill_col_string(
                    accessor, col, name, cidx, DTYPE_STR, is_arrow, is_update);
                return;
            } else {
                col->set_nth(i, static_cast<std::int64_t>(fval));
            }
        }
    }
}


template <>
void
set_column_nth(t_column* col, t_uindex idx, t_val value) {

    // Check if the value is a javascript null
    if (value.is_none()) {
        col->unset(idx);
        return;
    }

    switch (col->get_dtype()) {
        case DTYPE_BOOL: {
            col->set_nth<bool>(idx, value.cast<bool>(), STATUS_VALID);
            break;
        }
        case DTYPE_FLOAT64: {
            col->set_nth<double>(idx, value.cast<double>(), STATUS_VALID);
            break;
        }
        case DTYPE_FLOAT32: {
            col->set_nth<float>(idx, value.cast<float>(), STATUS_VALID);
            break;
        }
        case DTYPE_UINT32: {
            col->set_nth<std::uint32_t>(idx, value.cast<std::uint32_t>(), STATUS_VALID);
            break;
        }
        case DTYPE_UINT64: {
            col->set_nth<std::uint64_t>(idx, value.cast<std::uint64_t>(), STATUS_VALID);
            break;
        }
        case DTYPE_INT32: {
            col->set_nth<std::int32_t>(idx, value.cast<std::int32_t>(), STATUS_VALID);
            break;
        }
        case DTYPE_INT64: {
            col->set_nth<std::int64_t>(idx, value.cast<std::int64_t>(), STATUS_VALID);
            break;
        }
        case DTYPE_STR: {
            std::wstring welem = value.cast<std::wstring>();

            std::wstring_convert<utf16convert_type, wchar_t> converter;
            std::string elem = converter.to_bytes(welem);
            col->set_nth(idx, elem, STATUS_VALID);
            break;
        }
        case DTYPE_DATE: {
            // col->set_nth<t_date>(idx, pythondate_to_t_date(value), STATUS_VALID);
            break;
        }
        case DTYPE_TIME: {
            col->set_nth<std::int64_t>(
                idx, static_cast<std::int64_t>(value.cast<double>()), STATUS_VALID);
            break;
        }
        case DTYPE_UINT8:
        case DTYPE_UINT16:
        case DTYPE_INT8:
        case DTYPE_INT16:
        default: {
            // Other types not implemented
        }
    }
}

// TODO
// template <>
// void
// table_add_computed_column(t_data_table& table, t_val computed_defs) {
//     auto vcomputed_defs = vecFromArray<t_val, t_val>(computed_defs);
//     for (auto i = 0; i < vcomputed_defs.size(); ++i) {
//         t_val coldef = vcomputed_defs[i];
//         std::string name = coldef["column"].cast<std::string>();
//         t_val inputs = coldef["inputs"];
//         t_val func = coldef["func"];
//         t_val type = coldef["type"];

//         std::string stype;

//         if (type.isUndefined()) {
//             stype = "string";
//         } else {
//             stype = type.cast<std::string>();
//         }

//         t_dtype dtype;
//         if (stype == "integer") {
//             dtype = DTYPE_INT32;
//         } else if (stype == "float") {
//             dtype = DTYPE_FLOAT64;
//         } else if (stype == "boolean") {
//             dtype = DTYPE_BOOL;
//         } else if (stype == "date") {
//             dtype = DTYPE_DATE;
//         } else if (stype == "datetime") {
//             dtype = DTYPE_TIME;
//         } else {
//             dtype = DTYPE_STR;
//         }

//         // Get list of input column names
//         auto icol_names = vecFromArray<t_val, std::string>(inputs);

//         // Get t_column* for all input columns
//         std::vector<const t_column*> icols;
//         for (const auto& cc : icol_names) {
//             icols.push_back(table._get_column(cc));
//         }

//         int arity = icols.size();

//         // Add new column
//         t_column* out = table.add_column(name, dtype, true);

//         t_val i1 = t_val::undefined(), i2 = t_val::undefined(), i3 = t_val::undefined(),
//               i4 = t_val::undefined();

//         t_uindex size = table.size();
//         for (t_uindex ridx = 0; ridx < size; ++ridx) {
//             t_val value = t_val::undefined();

//             switch (arity) {
//                 case 0: {
//                     value = func();
//                     break;
//                 }
//                 case 1: {
//                     i1 = scalar_to_val(icols[0]->get_scalar(ridx));
//                     if (!i1.isNull()) {
//                         value = func(i1);
//                     }
//                     break;
//                 }
//                 case 2: {
//                     i1 = scalar_to_val(icols[0]->get_scalar(ridx));
//                     i2 = scalar_to_val(icols[1]->get_scalar(ridx));
//                     if (!i1.isNull() && !i2.isNull()) {
//                         value = func(i1, i2);
//                     }
//                     break;
//                 }
//                 case 3: {
//                     i1 = scalar_to_val(icols[0]->get_scalar(ridx));
//                     i2 = scalar_to_val(icols[1]->get_scalar(ridx));
//                     i3 = scalar_to_val(icols[2]->get_scalar(ridx));
//                     if (!i1.isNull() && !i2.isNull() && !i3.isNull()) {
//                         value = func(i1, i2, i3);
//                     }
//                     break;
//                 }
//                 case 4: {
//                     i1 = scalar_to_val(icols[0]->get_scalar(ridx));
//                     i2 = scalar_to_val(icols[1]->get_scalar(ridx));
//                     i3 = scalar_to_val(icols[2]->get_scalar(ridx));
//                     i4 = scalar_to_val(icols[3]->get_scalar(ridx));
//                     if (!i1.isNull() && !i2.isNull() && !i3.isNull() && !i4.isNull()) {
//                         value = func(i1, i2, i3, i4);
//                     }
//                     break;
//                 }
//                 default: {
//                     // Don't handle other arity values
//                     break;
//                 }
//             }

//             if (!value.isUndefined()) {
//                 set_column_nth(out, ridx, value);
//             }
//         }
//     }
// }

void
_fill_col_numeric(t_data_accessor accessor, t_data_table& tbl,
    std::shared_ptr<t_column> col, const std::string& name, std::int32_t cidx, t_dtype type,
    bool is_arrow, bool is_update) {
    t_uindex nrows = col->size();

    if (is_arrow) {
        // TODO
        // t_val data = accessor["values"];

        // switch (type) {
        //     case DTYPE_INT8: {
        //         arrow::vecFromTypedArray(data, col->get_nth<std::int8_t>(0), nrows);
        //     } break;
        //     case DTYPE_INT16: {
        //         arrow::vecFromTypedArray(data, col->get_nth<std::int16_t>(0), nrows);
        //     } break;
        //     case DTYPE_INT32: {
        //         arrow::vecFromTypedArray(data, col->get_nth<std::int32_t>(0), nrows);
        //     } break;
        //     case DTYPE_FLOAT32: {
        //         arrow::vecFromTypedArray(data, col->get_nth<float>(0), nrows);
        //     } break;
        //     case DTYPE_FLOAT64: {
        //         arrow::vecFromTypedArray(data, col->get_nth<double>(0), nrows);
        //     } break;
        //     default:
        //         break;
        // }
    } else {
        for (auto i = 0; i < nrows; ++i) {
            t_val item = accessor.attr("marshal")(cidx, i, type);

            // TODO
            // if (item.isUndefined())
            //     continue;

            if (item.is_none()) {
                if (is_update) {
                    col->unset(i);
                } else {
                    col->clear(i);
                }
                continue;
            }

            switch (type) {
                case DTYPE_INT8: {
                    col->set_nth(i, item.cast<std::int8_t>());
                } break;
                case DTYPE_INT16: {
                    col->set_nth(i, item.cast<std::int16_t>());
                } break;
                case DTYPE_INT32: {
                    // This handles cases where a long sequence of e.g. 0 precedes a clearly
                    // float value in an inferred column. Would not be needed if the type
                    // inference checked the entire column/we could reset parsing.
                    double fval = item.cast<double>();
                    if (fval > 2147483647 || fval < -2147483648) {
                        WARN("Promoting to float");
                        tbl.promote_column(name, DTYPE_FLOAT64, i, true);
                        col = tbl.get_column(name);
                        type = DTYPE_FLOAT64;
                        col->set_nth(i, fval);
                    } else if (isnan(fval)) {
                        WARN("Promoting to string");
                        tbl.promote_column(name, DTYPE_STR, i, false);
                        col = tbl.get_column(name);
                        _fill_col_string(
                            accessor, col, name, cidx, DTYPE_STR, is_arrow, is_update);
                        return;
                    } else {
                        col->set_nth(i, static_cast<std::int32_t>(fval));
                    }
                } break;
                case DTYPE_FLOAT32: {
                    col->set_nth(i, item.cast<float>());
                } break;
                case DTYPE_FLOAT64: {
                    col->set_nth(i, item.cast<double>());
                } break;
                default:
                    break;
            }
        }
    }
}

void
_fill_data_helper(t_data_accessor accessor, t_data_table& tbl,
    std::shared_ptr<t_column> col, const std::string& name, std::int32_t cidx, t_dtype type,
    bool is_arrow, bool is_update) {
    switch (type) {
        case DTYPE_INT64: {
            _fill_col_int64(accessor, tbl, col, name, cidx, type, is_arrow, is_update);
        } break;
        case DTYPE_BOOL: {
            _fill_col_bool(accessor, col, name, cidx, type, is_arrow, is_update);
        } break;
        case DTYPE_DATE: {
            // _fill_col_date(accessor, col, name, cidx, type, is_arrow, is_update);
        } break;
        case DTYPE_TIME: {
            // _fill_col_time(accessor, col, name, cidx, type, is_arrow, is_update);
        } break;
        case DTYPE_STR: {
            _fill_col_string(accessor, col, name, cidx, type, is_arrow, is_update);
        } break;
        case DTYPE_NONE: {
            break;
        }
        default:
            _fill_col_numeric(
                accessor, tbl, col, name, cidx, type, is_arrow, is_update);
    }
}

/******************************************************************************
 *
 * Fill tables with data
 */

void
_fill_data(t_data_table& tbl, t_data_accessor accessor, const t_schema& input_schema,
           const std::string& index, std::uint32_t offset, std::uint32_t limit,
           bool is_arrow, bool is_update) {
    bool implicit_index = false;
    std::vector<std::string> col_names(input_schema.columns());
    std::vector<t_dtype> data_types(input_schema.types());

    for (auto cidx = 0; cidx < col_names.size(); ++cidx) {
        auto name = col_names[cidx];
        auto type = data_types[cidx];

        t_val dcol = py::none();

        if (is_arrow) {
            //TODO
            // dcol = accessor["cdata"][cidx];
        } else {
            dcol = accessor;
        }
        if (name == "__INDEX__") {
            implicit_index = true;
            std::shared_ptr<t_column> pkey_col_sptr = tbl.add_column_sptr("psp_pkey", type, true);
            _fill_data_helper(dcol, tbl, pkey_col_sptr, "psp_pkey", cidx, type, is_arrow, is_update);
            tbl.clone_column("psp_pkey", "psp_okey");
            continue;
         }

        auto col = tbl.get_column(name);
        _fill_data_helper(dcol, tbl, col, name, cidx, type, is_arrow, is_update);

        if (is_arrow) {
            // TODO
            // // Fill validity bitmap
            // std::uint32_t null_count = dcol["nullCount"].cast<std::uint32_t>();

            // if (null_count == 0) {
            //     col->valid_raw_fill();
            // } else {
            //     t_val validity = dcol["nullBitmap"];
            //     arrow::fill_col_valid(validity, col);
            // }
        }
    }
    // Fill index column - recreated every time a `t_data_table` is created.
    if (!implicit_index) {
        if (index == "") {
            // Use row number as index if not explicitly provided or provided with `__INDEX__`
            auto key_col = tbl.add_column("psp_pkey", DTYPE_INT32, true);
            auto okey_col = tbl.add_column("psp_okey", DTYPE_INT32, true);

            for (std::uint32_t ridx = 0; ridx < tbl.size(); ++ridx) {
                key_col->set_nth<std::int32_t>(ridx, (ridx + offset) % limit);
                okey_col->set_nth<std::int32_t>(ridx, (ridx + offset) % limit);
            }
        } else {
            tbl.clone_column(index, "psp_pkey");
            tbl.clone_column(index, "psp_okey");
        }
    }
}
/******************************************************************************
 *
 * Table API
 */

std::shared_ptr<Table> make_table_py(t_val table, t_data_accessor accessor, t_val computed,
        std::uint32_t limit, py::str index, t_op op, bool is_update, bool is_arrow) {
    std::vector<std::string> column_names;
    std::vector<t_dtype> data_types;

    // Determine metadata
    bool is_delete = op == OP_DELETE;
    if (is_arrow || (is_update || is_delete)) {
        column_names = accessor.attr("column_names")().cast<std::vector<std::string>>();
        data_types = accessor.attr("column_types")().cast<std::vector<t_dtype>>();
    } else {
        // Infer names and types
        t_val data = accessor.attr("data")();
        std::int32_t format = accessor.attr("format")().cast<std::int32_t>();
        column_names = get_column_names(data, format);
        data_types = get_data_types(data, format, column_names, accessor.attr("date_validator")().cast<t_val>());
    }

    // Check if index is valid after getting column names
    bool table_initialized = !table.is_none();
    std::shared_ptr<t_pool> pool;
    std::shared_ptr<Table> tbl;
    std::uint32_t offset;

    // If the Table has already been created, use it
    if (table_initialized) {
        // Get a reference to the Table, and update its metadata
        tbl = table.cast<std::shared_ptr<Table>>();
        pool = tbl->get_pool();
        tbl->set_column_names(column_names);
        tbl->set_data_types(data_types);
        offset = tbl->get_offset();

        auto current_gnode = tbl->get_gnode();

        // use gnode metadata to help decide if we need to update
        is_update = (is_update || current_gnode->mapping_size() > 0);

        // if performing an arrow schema update, promote columns
        auto current_data_table = current_gnode->get_table();

        if (is_arrow && is_update && current_data_table->size() == 0) {
            auto current_schema = current_data_table->get_schema();
            for (auto idx = 0; idx < current_schema.m_types.size(); ++idx) {
                if (data_types[idx] == DTYPE_INT64) {
                    WARN("Promoting int64 '" + column_names[idx] + "'");
                    current_gnode->promote_column(column_names[idx], DTYPE_INT64);
                }
            }
        }
    } else {
        pool = std::make_shared<t_pool>();
        tbl = std::make_shared<Table>(pool, column_names, data_types, limit, index);
        offset = 0;
    }

    // Create input schema - an input schema contains all columns to be displayed AND index + operation columns
    t_schema input_schema(column_names, data_types);

    // strip implicit index, if present
    auto implicit_index_it = std::find(column_names.begin(), column_names.end(), "__INDEX__");
    if (implicit_index_it != column_names.end()) {
        auto idx = std::distance(column_names.begin(), implicit_index_it);
        // position of the column is at the same index in both vectors
        column_names.erase(column_names.begin() + idx);
        data_types.erase(data_types.begin() + idx);
    }

    // Create output schema - contains only columns to be displayed to the user
    t_schema output_schema(column_names, data_types); // names + types might have been mutated at this point after implicit index removal

    std::uint32_t row_count = accessor.attr("row_count")().cast<std::int32_t>();
    t_data_table data_table(output_schema);
    data_table.init();
    data_table.extend(row_count);

    // write data at the correct row
    _fill_data(data_table, accessor, input_schema, index, offset, limit, is_arrow, is_update);

     if (!computed.is_none()) {
        // TODO
        // re-add computed columns after update, delete, etc.
        // table_add_computed_column(data_table, computed);
     }

    // calculate offset, limit, and set the gnode
    tbl->init(data_table, row_count, op);

    // FIXME: replicate JS _clear_process etc.
    pool->_process();
    return tbl;
}

std::shared_ptr<Table>
make_computed_table_py(std::shared_ptr<Table> table, t_val computed) {
    // TODO
    return table;
}

/******************************************************************************
 *
 * View API
 */

template <>
bool
is_valid_filter(t_dtype column_type, t_val date_parser, t_filter_op filter_operator, t_val filter_term) {
    if (filter_operator == t_filter_op::FILTER_OP_IS_NULL
        || filter_operator == t_filter_op::FILTER_OP_IS_NOT_NULL) {
        return true;
    } else if (column_type == DTYPE_DATE || column_type == DTYPE_TIME) {
        t_val parsed_date = date_parser.attr("parse")(filter_term);
        return !parsed_date.is_none();
    } else {
        return !filter_term.is_none();
    }
};

template <>
std::tuple<std::string, std::string, std::vector<t_tscalar>>
make_filter_term(t_dtype column_type, t_val date_parser, const std::string& column_name, const std::string& filter_op_str, t_val filter_term) {
    t_filter_op filter_op = str_to_filter_op(filter_op_str);
    std::vector<t_tscalar> terms;

    switch (filter_op) {
        case FILTER_OP_NOT_IN:
        case FILTER_OP_IN: {
            std::vector<std::string> filter_terms
                = filter_term.cast<std::vector<std::string>>();
            for (auto term : filter_terms) {
                terms.push_back(mktscalar(get_interned_cstr(term.c_str())));
            }
        } break;
        case FILTER_OP_IS_NULL:
        case FILTER_OP_IS_NOT_NULL: {
            terms.push_back(mktscalar(0));
        } break;
        default: {
            switch (column_type) {
                case DTYPE_INT32: {
                    terms.push_back(mktscalar(filter_term.cast<std::int32_t>()));
                } break;
                case DTYPE_INT64:
                case DTYPE_FLOAT64: {
                    terms.push_back(mktscalar(filter_term.cast<double>()));
                } break;
                case DTYPE_BOOL: {
                    terms.push_back(mktscalar(filter_term.cast<bool>()));
                } break;
                case DTYPE_DATE: {
                    t_val parsed_date = date_parser.attr("parse")(filter_term);
                    terms.push_back(mktscalar(pythondate_to_t_date(parsed_date)));
                } break;
                case DTYPE_TIME: {
                    t_val parsed_date = date_parser.attr("parse")(filter_term);
                    terms.push_back(mktscalar(t_time(static_cast<std::int64_t>(
                    parsed_date.attr("timestamp")().cast<double>()))));
                } break;
                default: {
                    terms.push_back(
                        mktscalar(get_interned_cstr(filter_term.cast<std::string>().c_str())));
                }
            }
        }
    }

    return std::make_tuple(column_name, filter_op_str, terms);
}
template <>
t_view_config
make_view_config(const t_schema& schema, t_val date_parser, t_val config) {
    auto row_pivots = config.attr("get_row_pivots")().cast<std::vector<std::string>>();
    auto column_pivots = config.attr("get_column_pivots")().cast<std::vector<std::string>>();
    auto columns = config.attr("get_columns")().cast<std::vector<std::string>>();
    auto sort = config.attr("get_sort")().cast<std::vector<std::vector<std::string>>>();
    auto filter_op = config.attr("get_filter_op")().cast<std::string>();

    // aggregates require manual parsing - std::maps read from JS are empty
    auto p_aggregates= config.attr("get_aggregates")().cast<std::vector<std::vector<std::string>>>();
    tsl::ordered_map<std::string, std::string> aggregates;

    for (const auto& vec : p_aggregates) {
        aggregates[vec[0]] = vec[1];
    };

    bool column_only = false;

    // make sure that primary keys are created for column-only views
    if (row_pivots.size() == 0 && column_pivots.size() > 0) {
        row_pivots.push_back("psp_okey");
        column_only = true;
    }

    // construct filters with filter terms, and fill the vector of tuples
    auto p_filter = config.attr("get_filter")().cast<std::vector<std::vector<t_val>>>();
    std::vector<std::tuple<std::string, std::string, std::vector<t_tscalar>>> filter;

    for (auto f : p_filter) {
        std::string column_name = f.at(0).cast<std::string>();
        std::string filter_op_str = f.at(1).cast<std::string>();
        t_dtype column_type = schema.get_dtype(column_name);
        t_filter_op filter_operator = str_to_filter_op(filter_op_str);
        
        // validate the filter before it goes into the core engine
        t_val filter_term = py::none();
        if (f.size() > 2) {
            filter_term = f.at(2);
        }
        if (is_valid_filter(column_type, date_parser, filter_operator, filter_term)) {
            filter.push_back(make_filter_term(column_type, date_parser, column_name, filter_op_str, filter_term));
        }

        
    }

    // create the `t_view_config`
    t_view_config view_config(row_pivots, column_pivots, aggregates, columns, filter, sort,
        filter_op, column_only);

    // transform primitive values into abstractions that the engine can use
    view_config.init(schema);

    // set pivot depths if provided
    if (! config.attr("row_pivot_depth").is_none()) {
        view_config.set_row_pivot_depth(config.attr("row_pivot_depth").cast<std::int32_t>());
    }

    if (! config.attr("column_pivot_depth").is_none()) {
        view_config.set_column_pivot_depth(config.attr("column_pivot_depth").cast<std::int32_t>());
    }

    return view_config;
}

template <typename CTX_T>
std::shared_ptr<View<CTX_T>>
make_view(std::shared_ptr<Table> table, const std::string& name, const std::string& separator,
    t_val view_config, t_val date_parser) {
    auto schema = table->get_schema();

    t_view_config config = make_view_config<t_val>(schema, date_parser, view_config);

    auto ctx = make_context<CTX_T>(table, schema, config, name);

    auto view_ptr = std::make_shared<View<CTX_T>>(table, ctx, name, separator, config);

    return view_ptr;
}

std::shared_ptr<View<t_ctx0>>
make_view_ctx0(std::shared_ptr<Table> table, const std::string& name, const std::string& separator,
    t_val view_config, t_val date_parser) {
    return make_view<t_ctx0>(table, name, separator, view_config, date_parser);
}

std::shared_ptr<View<t_ctx1>>
make_view_ctx1(std::shared_ptr<Table> table, const std::string& name, const std::string& separator,
    t_val view_config, t_val date_parser) {
    return make_view<t_ctx1>(table, name, separator, view_config, date_parser);
}

std::shared_ptr<View<t_ctx2>>
make_view_ctx2(std::shared_ptr<Table> table, const std::string& name, const std::string& separator,
    t_val view_config, t_val date_parser) {
    return make_view<t_ctx2>(table, name, separator, view_config, date_parser);
}

/******************************************************************************
 *
 * Context API
 */

template <>
std::shared_ptr<t_ctx0>
make_context(std::shared_ptr<Table> table, const t_schema& schema,
    const t_view_config& view_config, const std::string& name) {
    auto columns = view_config.get_columns();
    auto filter_op = view_config.get_filter_op();
    auto fterm = view_config.get_fterm();
    auto sortspec = view_config.get_sortspec();

    auto cfg = t_config(columns, filter_op, fterm);
    auto ctx0 = std::make_shared<t_ctx0>(schema, cfg);
    ctx0->init();
    ctx0->sort_by(sortspec);

    auto pool = table->get_pool();
    auto gnode = table->get_gnode();
    pool->register_context(gnode->get_id(), name, ZERO_SIDED_CONTEXT,
        reinterpret_cast<std::uintptr_t>(ctx0.get()));

    return ctx0;
}

template <>
std::shared_ptr<t_ctx1>
make_context(std::shared_ptr<Table> table, const t_schema& schema,
    const t_view_config& view_config, const std::string& name) {
    auto row_pivots = view_config.get_row_pivots();
    auto aggspecs = view_config.get_aggspecs();
    auto filter_op = view_config.get_filter_op();
    auto fterm = view_config.get_fterm();
    auto sortspec = view_config.get_sortspec();
    auto row_pivot_depth = view_config.get_row_pivot_depth();

    auto cfg = t_config(row_pivots, aggspecs, filter_op, fterm);
    auto ctx1 = std::make_shared<t_ctx1>(schema, cfg);

    ctx1->init();
    ctx1->sort_by(sortspec);

    auto pool = table->get_pool();
    auto gnode = table->get_gnode();
    pool->register_context(gnode->get_id(), name, ONE_SIDED_CONTEXT,
        reinterpret_cast<std::uintptr_t>(ctx1.get()));

    if (row_pivot_depth > -1) {
        ctx1->set_depth(row_pivot_depth - 1);
    } else {
        ctx1->set_depth(row_pivots.size());
    }

    return ctx1;
}

template <>
std::shared_ptr<t_ctx2>
make_context(std::shared_ptr<Table> table, const t_schema& schema,
    const t_view_config& view_config, const std::string& name) {
    bool column_only = view_config.is_column_only();
    auto row_pivots = view_config.get_row_pivots();
    auto column_pivots = view_config.get_column_pivots();
    auto aggspecs = view_config.get_aggspecs();
    auto filter_op = view_config.get_filter_op();
    auto fterm = view_config.get_fterm();
    auto sortspec = view_config.get_sortspec();
    auto col_sortspec = view_config.get_col_sortspec();
    auto row_pivot_depth = view_config.get_row_pivot_depth();
    auto column_pivot_depth = view_config.get_column_pivot_depth();

    t_totals total = sortspec.size() > 0 ? TOTALS_BEFORE : TOTALS_HIDDEN;

    auto cfg = t_config(
        row_pivots, column_pivots, aggspecs, total, filter_op, fterm, column_only);
    auto ctx2 = std::make_shared<t_ctx2>(schema, cfg);

    ctx2->init();

    auto pool = table->get_pool();
    auto gnode = table->get_gnode();
    pool->register_context(gnode->get_id(), name, TWO_SIDED_CONTEXT,
        reinterpret_cast<std::uintptr_t>(ctx2.get()));

    if (row_pivot_depth > -1) {
        ctx2->set_depth(t_header::HEADER_ROW, row_pivot_depth - 1);
    } else {
        ctx2->set_depth(t_header::HEADER_ROW, row_pivots.size());
    }

    if (column_pivot_depth > -1) {
        ctx2->set_depth(t_header::HEADER_COLUMN, column_pivot_depth - 1);
    } else {
        ctx2->set_depth(t_header::HEADER_COLUMN, column_pivots.size());
    }

    if (sortspec.size() > 0) {
        ctx2->sort_by(sortspec);
    }

    if (col_sortspec.size() > 0) {
        ctx2->column_sort_by(col_sortspec);
    }

    return ctx2;
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
    t_tscalar d = data_slice->get(ridx, cidx);
    return scalar_to_py(d);
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