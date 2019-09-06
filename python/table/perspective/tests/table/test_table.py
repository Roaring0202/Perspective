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

class TestTable(object):
    def test_table_construct(self):
        tbl = Table(data)
        assert tbl.size() == 2 

    def test_table_columns(self):
        tbl = Table(data)
        assert tbl.columns() == ["a", "b"]
