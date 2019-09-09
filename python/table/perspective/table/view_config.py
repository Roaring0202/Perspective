# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#
from perspective.table.libbinding import string_vector, string_vector_vector, t_val_vector, t_val_vector_vector

# FIXME: how to make user-friendly by returning python data structures


class ViewConfig(object):
    '''Defines the configuration for a View object.'''

    def __init__(self, config):
        '''Receives a user-provided config dict and converts its contents into C++ vectors for use in the core engine.

        Arguments:
            config : dict
                Key-value mapping of configuration items to their values
        '''
        self._row_pivots = string_vector(config.get('row-pivots', []))
        self._column_pivots = string_vector(config.get('column-pivots', []))
        self._aggregates = string_vector_vector([string_vector(agg) for agg in config.get('aggregates', [])])
        self._columns = string_vector(config.get('columns', []))
        self._sort = string_vector_vector([string_vector(sort) for sort in config.get('sort', [])])
        self._filter = t_val_vector_vector([t_val_vector(filt) for filt in config.get('filter', [])])
        self._filter_op = config.get('filter_op', "and")
        self.row_pivot_depth = None  # TODO: implement for 1 and 2-sided views
        self.column_pivot_depth = None

    def get_row_pivots(self):
        '''The columns used as [row pivots](https://en.wikipedia.org/wiki/Pivot_table#Row_labels)

        Returns:
            self._row_pivots : a vector of strings
        '''
        return self._row_pivots

    def get_column_pivots(self):
        '''The columns used as [column pivots](https://en.wikipedia.org/wiki/Pivot_table#Column_labels)

        Returns:
            self._column_pivots : a vector of strings
        '''
        return self._column_pivots

    def get_aggregates(self):
        '''Defines the grouping of data within columns.

        FIXME: defined as vector of vectors, not a map as in JS

        Returns:
            self._aggregates : a vector of string vectors in which the first value is the column name, and the second value is the string representation of an aggregate
        '''
        return self._aggregates

    def get_columns(self):
        '''The columns that will be shown to the user in the view. If left empty, the view shows all columns in the dataset by default.

        Returns:
            self._columns : a vector of strings
        '''
        return self._columns

    def get_sort(self):
        '''The columns that should be sorted, and the direction to sort.

        A sort configuration is a list of two elements: a string column name, and a string sort direction, which are:
        "none", "asc", "desc", "col asc", "col desc", "asc abs", "desc abs", "col asc abs", and "col desc abs".

        Returns:
            self._sort : a vector of string vectors
        '''
        return self._sort

    def get_filter(self):
        '''The columns that should be filtered.

        A filter configuration is a list of three elements: a string column name, a filter comparison string (i.e. "===", ">"), and a value to compare.

        Returns:
            self._filter : a vector of t_val vectors
        '''
        return self._filter

    def get_filter_op(self):
        '''When multiple filters are applied, filter_op defines how data should be returned.

        Defaults to "and" if not set by the user, meaning that data returned with multiple filters will satisfy all filters.

        If "or" is provided, returned data will satsify any one of the filters applied.

        Returns
            self._filter_op : string
        '''
        return self._filter_op
