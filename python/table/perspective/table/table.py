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
        # FIXME: views and tables created lose reference to the View/Table in C++
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
            t_schema : the Perspective schema object
        '''
        return self._table.get_schema()

    def columns(self, computed=False):
        return [column for column in self.schema().columns() if column != "psp_okey"]

    def update(self, data):
        pass

    def remove(self, data):
        pass

    def view(self, config=None):
        config = config or {}
        if len(config.get("columns", [])) == 0:
            config["columns"] = self.columns()
        return View(self, config)
