# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#

from perspective.table import Table
from datetime import date, datetime


class TestTable(object):
    # table constructors

    def test_empty_table(self):
        tbl = Table([])
        assert tbl.size() == 0

    def test_table_int(self):
        data = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        tbl = Table(data)
        assert tbl.size() == 2
        assert tbl.schema() == {
            "a": "integer",
            "b": "integer"
        }

    def test_table_nones(self):
        none_data = [{"a": 1, "b": None}, {"a": None, "b": 2}]
        tbl = Table(none_data)
        assert tbl.size() == 2
        assert tbl.schema() == {
            "a": "integer",
            "b": "integer"
        }

    def test_table_bool(self):
        bool_data = [{"a": True, "b": False}, {"a": True, "b": True}]
        tbl = Table(bool_data)
        assert tbl.size() == 2
        # TODO: booleans cast as floats and ints
        assert tbl.schema() == {
            "a": "boolean",
            "b": "boolean"
        }

    def test_table_float(self):
        float_data = [{"a": 1.5, "b": 2.5}, {"a": 3.2, "b": 3.1}]
        tbl = Table(float_data)
        assert tbl.size() == 2
        assert tbl.schema() == {
            "a": "float",
            "b": "float"
        }

    def test_table_str(self):
        str_data = [{"a": "b", "b": "b"}, {"a": "3", "b": "3"}]
        tbl = Table(str_data)
        assert tbl.size() == 2
        assert tbl.schema() == {
            "a": "string",
            "b": "string"
        }

    def test_table_date(self):
        str_data = [{"a": date.today(), "b": date.today()}]
        tbl = Table(str_data)
        assert tbl.size() == 1

    def test_table_datetime(self):
        str_data = [{"a": datetime.now(), "b": datetime.now()}]
        tbl = Table(str_data)
        assert tbl.size() == 1

    def test_table_columns(self):
        data = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        tbl = Table(data)
        assert tbl.columns() == ["a", "b"]
        assert tbl.schema() == {
            "a": "integer",
            "b": "integer"
        }

    def test_table_columnar(self):
        data = {"a": [1, 2, 3], "b": [4, 5, 6]}
        tbl = Table(data)
        assert tbl.columns() == ["a", "b"]
        assert tbl.size() == 3
        assert tbl.schema() == {
            "a": "integer",
            "b": "integer"
        }

    def test_table_schema(self):
        data = {"a": int,
                "b": float,
                "c": str,
                "d": bool,
                "e": date,
                "f": datetime}

        tbl = Table(data)

        assert tbl.schema() == {
            "a": "integer",
            "b": "float",
            "c": "string",
            "d": "boolean",
            "e": "date",
            "f": "datetime"
        }

    # infer data types correctly

    def test_table_infer_int(self):
        data = {"a": [None, None, None, None, 1, 0, 1, 1, 1]}
        tbl = Table(data)
        assert tbl.schema() == {"a": "integer"}

    def test_table_infer_bool(self):
        data = {"a": [None, None, None, None, True, True, True]}
        tbl = Table(data)
        assert tbl.schema() == {"a": "boolean"}

    def test_table_infer_str(self):
        data = {"a": [None, None, None, None, None, None, "abc"]}
        tbl = Table(data)
        assert tbl.schema() == {"a": "string"}

    # index

    def test_table_index(self):
        data = [{"a": 1, "b": 2}, {"a": 1, "b": 4}]
        tbl = Table(data, {"index": "a"})
        assert tbl.size() == 1
        assert tbl.view().to_dict() == [
            {"a": 1, "b": 4}
        ]

    # limit
    
    def test_table_limit(self):
        data = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        tbl = Table(data, {"limit": 1})
        assert tbl.size() == 1
        assert tbl.view().to_dict() == [
            {"a": 3, "b": 4}
        ]