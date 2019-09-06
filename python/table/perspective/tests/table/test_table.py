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


class TestTable(object):
    def test_table(self):
        x = [{"a": 1, "b": 2}, {"a": 3, "b": 3}]

        p = Table(x)
        print(p.size())
        print(p.schema())
