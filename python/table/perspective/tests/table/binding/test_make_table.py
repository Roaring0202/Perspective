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
from perspective.table.libbinding import make_table, t_op
from perspective.table import _PerspectiveAccessor


class TestMakeTable(object):
    def test_make_table(self):
        p = _PerspectiveAccessor([{"a": 1, "b": 2}, {"a": 3, "b": 3}])
        x = make_table(None, p, None, 4294967295, '', t_op.OP_INSERT, False, False)
        print(x.size())
        print(x.get_schema())
