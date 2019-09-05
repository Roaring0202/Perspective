# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#
from random import random
from perspective.table.libbinding import make_table, make_view_zero, make_view_one, make_view_two, t_op, string_vector, string_vector_vector, t_val_vector_vector

DEFAULT_SEPARATOR_STRING = "|" # TODO: move to a constants.py

class _PerspectiveDateValidator(object):
    '''Internal Class for date validation'''
    def __init__(self):
        pass # TODO: implement


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
    '''Defines the parameters of a View object'''
    def __init__(self, config):
        self._row_pivots = string_vector(config.get('row-pivots', []))
        self._column_pivots = string_vector(config.get('column-pivots', []))
        self._aggregates = string_vector_vector(config.get('aggregates', []))
        self._columns = string_vector(config.get('columns', []))
        self._sort = string_vector_vector(config.get('sort', []))
        self._filter = t_val_vector_vector(config.get('filter', []))
        self._filter_op = config.get('filter_op', "and")
        self.row_pivot_depth = None # TODO: implement for 1 and 2-sided views
        self.column_pivot_depth = None

    def get_row_pivots(self):
        return self._row_pivots

    def get_column_pivots(self):
        return self._column_pivots

    def get_aggregates(self):
        return self._aggregates

    def get_columns(self):
        return self._columns

    def get_sort(self):
        return self._sort

    def get_filter(self):
        return self._filter

    def get_filter_op(self):
        return self._filter_op


class Table(object):
    def __init__(self, data_or_schema, config = {}):
        self._accessor = _PerspectiveAccessor(data_or_schema)
        self._limit = config.get("limit", 4294967295)
        self._index = config.get("index", "")
        # FIXME: views and tables created lose reference to the View/Table in C++
        self._table = make_table(None, self._accessor, None, self._limit, self._index, t_op.OP_INSERT, False, False)

    def size(self):
        return self._table.size()

    def schema(self):
        return self._table.get_schema()

    def view(self, config = {}):
        return View(self, config)

class View(object):
    def __init__(self, Table, config = {}):
        """Private constructor for a View object - use the Table.view() method to create Views.
        
        A View object represents a specific transform (configuration or pivot,
        filter, sort, etc) configuration on an underlying Table. A View
        receives all updates from the Table from which it is derived, and
        can be serialized to JSON or trigger a callback when it is updated.

        View objects are immutable, and will remain in memory and actively process
        updates until its delete() method is called.
        """
        self._table = Table
        self._config = ViewConfig(config)
        self._sides = self.sides()
        
        if self._sides == 0:
            # FIXME: weird date validator passing
            self._view = make_view_zero(self._table._table, str(random()), DEFAULT_SEPARATOR_STRING, self._config, self._table._accessor._date_validator)
        elif self._sides == 1:
            self._view = make_view_one(self._table._table, str(random()), DEFAULT_SEPARATOR_STRING, self._config, self._table._accessor._date_validator)
        else:
            self._view = make_view_two(self._table._table, str(random()), DEFAULT_SEPARATOR_STRING, self._config, self._table._accessor._date_validator)

    def sides(self):
        """How many pivoted sides does this View have?"""
        if len(self._config.get_row_pivots()) > 0 or len(self._config.get_column_pivots()) > 0:
            if len(self._config.get_column_pivots()) > 0:
                return 2
            else:
                return 1
        else:
            return 0

    def num_rows(self):
        """The number of aggregated rows in the View. This is affected by the `row-pivots` that are applied to the View."""
        return self._view.num_rows()

    def num_columns(self):
        """The number of aggregated columns in the View. This is affected by the `column-pivots` that are applied to the View."""
        return self._view.num_columns()

