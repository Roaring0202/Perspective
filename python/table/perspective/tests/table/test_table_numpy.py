# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#

import numpy as np
from perspective.table import Table


class TestTableNumpy(object):
    def test_empty_table(self):
        tbl = Table([])
        assert tbl.size() == 0

    def test_table_int(self):
        data = {"a": np.array([1, 2, 3]), "b": np.array([4, 5, 6])}
        tbl = Table(data)
        assert tbl.size() == 2
