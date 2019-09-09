# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#

import os
import os.path
import numpy as np
import pandas as pd
from perspective.table import Table


data = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]


class TestView(object):

    def test_view_zero(self):
        tbl = Table(data)
        view = tbl.view()
        assert view.num_rows() == 2
        assert view.num_columns() == 2
        assert view.schema() == {
            "a": "integer",
            "b": "integer"
        }

    def test_view_one(self):
        tbl = Table(data)
        view = tbl.view({
            "row_pivots": ["a"]
        })
        assert view.num_rows() == 2
        assert view.num_columns() == 2
        assert view.schema() == {
            "a": "integer",
            "b": "integer"
        }

    def test_view_two(self):
        tbl = Table(data)
        view = tbl.view({
            "row_pivots": ["a"],
            "column_pivots": ["b"]
        })
        assert view.num_rows() == 2
        assert view.num_columns() == 2
        assert view.schema() == {
            "a": "integer",
            "b": "integer"
        }

    # schema correctness

    def test_zero_view_schema(self):
        tbl = Table(data)
        view = tbl.view()
        assert view.schema() == {
            "a": "integer",
            "b": "integer"
        }

    def test_one_view_schema(self):
        tbl = Table({"a": ["string1", "string2", "string3"]})
        view = tbl.view({
            "row-pivots": ["a"],
            "aggregates": [["a", "distinct count"]]
        })
        assert view.schema() == {
            "a": "integer"  # distinct count returns integer counts
        }

    def test_two_view_schema(self):
        tbl = Table({"a": ["string1", "string2", "string3"], "b": ["string1", "string2", "string3"]})
        view = tbl.view({
            "row-pivots": ["a"],
            "column-pivots": ["b"],
            "aggregates": [["a", "distinct count"], ["b", "unique"]]
        })
        assert view.schema() == {
            "a": "integer",  # distinct count returns integer counts
            "b": "string"  # unique returns string
        }
