# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#
from perspective.table.libbinding import make_table, make_view_zero, make_view_one, make_view_two, t_op, string_vector, string_vector_vector, t_val_vector_vector


class _PerspectiveDateValidator(object):
    '''Internal Class for date validation'''
    def __init__(self):
        pass


class _PerspectiveAccessor(object):
    '''Internal class to manage perspective table state'''
    def __init__(self, data_or_schema):
        self._data_or_schema = data_or_schema
        self._format = \
            0 if isinstance(data_or_schema, list) else \
            1 if isinstance(data_or_schema, dict) and isinstance(next(data_or_schema.values), list) else \
            2

        self._date_validator = _PerspectiveDateValidator()
        self._row_count = \
            len(data_or_schema) if self._format == 0 else \
            max(data_or_schema.values(), key=len) if self._format == 1 else \
            0

    def data(self): return self._data_or_schema

    def format(self): return self._format

    def date_validator(self): return self._date_validator

    def row_count(self): return self._row_count

    def marshal(self, cidx, i, type):
        if self._format == 0:
            return self._data_or_schema[i][list(self._data_or_schema[0].keys())[cidx]]
        elif self._format == 1:
            return self._data_or_schema[list(self._data_or_schema[0].keys())[cidx]][i]
        else:
            raise NotImplementedError()


class ViewConfig(object):
    def __init__(self, config):
        self._row_pivots = string_vector(config.get('row-pivots', []))
        self._column_pivots = string_vector(config.get('column-pivots', []))
        self._columns = string_vector(config.get('columns', []))
        self._sort = string_vector_vector(config.get('sort', []))
        self._filter_op = "|"
        self._aggregates = string_vector_vector(config.get('aggregates', []))
        self._filter = t_val_vector_vector(config.get('filter', []))
        self.row_pivot_depth = None
        self.column_pivot_depth = None

    def get_row_pivots(self):
        return self._row_pivots

    def get_column_pivots(self):
        return self._column_pivots

    def get_columns(self):
        return self._columns

    def get_sort(self):
        return self._sort

    def get_filter_op(self):
        return self._filter_op

    def get_aggregates(self):
        return self._aggregates

    def get_filter(self):
        return self._filter


class Perspective(object):
    def __init__(self):
        pass

    def load(self, data_or_schema, limit=4294967295, index=""):
        self._accessor = _PerspectiveAccessor(data_or_schema)
        self._table = make_table(None, self._accessor, None, limit, index, t_op.OP_INSERT, False, False)
        config = ViewConfig({})
        self._view = make_view_zero(self._table, "test", "", config, self._accessor._date_validator)
        self._view1 = make_view_one(self._table, "test", "", config, self._accessor._date_validator)
        self._view2 = make_view_two(self._table, "test", "", config, self._accessor._date_validator)

    def size(self):
        return self._table.size()

    def schema(self):
        return self._table.get_schema()

    def view(self):
        return self._view
