# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#
from ._date_validator import _PerspectiveDateValidator


def _type_to_format(data_or_schema):
    if isinstance(data_or_schema, list):
        return 0
    elif isinstance(data_or_schema, dict):
        for v in data_or_schema.values():
            if isinstance(v, type):
                return 2
            elif isinstance(v, list) or iter(v):
                return 1
            else:
                raise NotImplementedError("Dict values must be list or type!")
        raise NotImplementedError("Dict values must be list or type!")
    else:
        raise NotImplementedError("Must be dict or list!")


class _PerspectiveAccessor(object):
    '''Internal class to manage perspective table state'''

    def __init__(self, data_or_schema):
        self._data_or_schema = data_or_schema
        self._format = _type_to_format(data_or_schema)
        self._date_validator = _PerspectiveDateValidator()
        self._row_count = \
            len(data_or_schema) if self._format == 0 else \
            len(max(data_or_schema.values(), key=len)) if self._format == 1 else \
            0
        if isinstance(data_or_schema, list):
            self._names = list(data_or_schema[0].keys()) if len(data_or_schema) > 0 else []
        elif isinstance(data_or_schema, dict):
            self._names = list(data_or_schema.keys())
        self._types = []

    def data(self):
        return self._data_or_schema

    def format(self):
        return self._format

    def names(self):
        return self._names

    def types(self):
        return self._types

    def date_validator(self):
        return self._date_validator

    def row_count(self):
        return self._row_count

    def marshal(self, cidx, i, type):
        if self._format == 0:
            return self._data_or_schema[i][list(self._data_or_schema[0].keys())[cidx]]
        elif self._format == 1:
            return self._data_or_schema[list(self._data_or_schema.keys())[cidx]][i]
        else:
            raise NotImplementedError()
