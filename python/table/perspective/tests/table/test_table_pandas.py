# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#

import pandas as pd
from perspective.table import Table


class TestTableNumpy(object):
    def test_empty_table(self):
        tbl = Table([])
        assert tbl.size() == 0

    def test_table_dataframe(self):
        data = pd.DataFrame([{"a": 1, "b": 2}, {"a": 3, "b": 4}])
        tbl = Table(data)
        assert tbl.size() == 2

    def test_table_series(self):
        data = pd.Series([1, 2, 3], name="a")
        tbl = Table(data)
        assert tbl.size() == 2
