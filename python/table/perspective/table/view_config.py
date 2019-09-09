# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#


class ViewConfig(object):
    '''Defines the parameters of a View object'''

    def __init__(self, config):
        '''Configuration helper for constructing a view

        Arguments:
            config : dict
                Key-value mapping
        '''
        self._row_pivots = config.get('row-pivots', [])
        self._column_pivots = config.get('column-pivots', [])
        self._aggregates = config.get('aggregates', [])
        self._columns = config.get('columns', [])
        self._sort = config.get('sort', [])
        self._filter = config.get('filter', [])
        self._filter_op = config.get('filter_op', "and")
        self.row_pivot_depth = None  # TODO: implement for 1 and 2-sided views
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
