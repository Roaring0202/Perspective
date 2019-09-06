# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#

from perspective.table import Table


class TestTable(object):
    def test_empty_table(self):
        x = []
        p = Table(x)
        print(p.size())
        print(p.schema())

    def test_table_int(self):
        x = [{"a": 1, "b": 2}, {"a": 3, "b": 3}]

        p = Table(x)
        print(p.size())
        print(p.schema())

    def test_table_nones(self):
        x = [{"a": 1, "b": None}, {"a": None, "b": 2}]

        p = Table(x)
        print(p.size())
        print(p.schema())

    def test_table_bool(self):
        x = [{"a": True, "b": False}, {"a": True, "b": True}]

        p = Table(x)
        print(p.size())
        print(p.schema())

    def test_table_float(self):
        x = [{"a": 1.5, "b": 2.5}, {"a": 3.2, "b": 3.1}]

        p = Table(x)
        print(p.size())
        print(p.schema())

    def test_table_str(self):
        x = [{"a": "b", "b": "b"}, {"a": "3", "b": "3"}]

        p = Table(x)
        print(p.size())
        print(p.schema())
