# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#
from random import random
from perspective.table.libbinding import make_view_zero, make_view_one, make_view_two
from .view_config import ViewConfig

DEFAULT_SEPARATOR_STRING = "|"  # TODO: move to a constants.py


class View(object):
    def __init__(self, Table, config=None):
        """Private constructor for a View object - use the Table.view() method to create Views.

        A View object represents a specific transform (configuration or pivot,
        filter, sort, etc) configuration on an underlying Table. A View
        receives all updates from the Table from which it is derived, and
        can be serialized to JSON or trigger a callback when it is updated.

        View objects are immutable, and will remain in memory and actively process
        updates until its delete() method is called.
        """
        self._table = Table
        self._config = ViewConfig(config or {})
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
        """The number of aggregated rows in the View. This is affected by the `row-pivots` that are applied to the View.

        Returns:
            int : number of rows
        """
        return self._view.num_rows()

    def num_columns(self):
        """The number of aggregated columns in the View. This is affected by the `column-pivots` that are applied to the View.
        Returns:
            int : number of columns
        """
        return self._view.num_columns()

    def schema(self):
        return self._view.schema()
