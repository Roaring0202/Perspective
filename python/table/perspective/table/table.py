# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#
from perspective.table.libbinding import make_table, t_op
from .view import View
from ._accessor import _PerspectiveAccessor
from ._utils import _dtype_to_str


class Table(object):
    def __init__(self, data_or_schema, config=None):
        '''Perspective Table object

        Arguments:
            data_or_schema : dict/list/dataframe
            config : dict
        '''
        config = config or {}
        self._accessor = _PerspectiveAccessor(data_or_schema)
        self._limit = config.get("limit", 4294967295)
        self._index = config.get("index", "")
        self._table = make_table(None, self._accessor, None, self._limit, self._index, t_op.OP_INSERT, False, False)

    def load(self, data_or_schema):
        '''Configuration helper for constructing a view

        Arguments:
            data_or_schema : dict/list/dataframe
        '''
        pass

    def size(self):
        '''Get size of the perspective table

        Returns:
            int : the size of the table
        '''
        return self._table.size()

    def schema(self):
        '''Get perspective schema

        Returns:
           schema : dict
                A key-value mapping of column names to data types.
        '''
        # we should be returning native python data structures - if we ingest python data structures and return weird c++ ones, it doesn't make sense
        s = self._table.get_schema()
        columns = s.columns()
        types = s.types()
        schema = {}
        for i in range(0, len(columns)):
            if (columns[i] != "psp_okey"):
                schema[columns[i]] = _dtype_to_str(types[i])
        return schema

    def columns(self, computed=False):
        return list(self.schema().keys())

    def update(self, data):
        pass

    def remove(self, data):
        pass

    def view(self, config=None):
        config = config or {}
        if len(config.get("columns", [])) == 0:
            config["columns"] = self.columns()
        return View(self, config)
