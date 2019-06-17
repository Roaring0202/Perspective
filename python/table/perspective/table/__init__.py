# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#


class _PerspectiveDateValidator(object):
    '''Internal Class for date validation'''
    def __init__(self):
        pass


class _PerspectiveAccessor(object):
    '''Internal class to manage perspective table state'''
    def __init__(self):
        self._data = [{"a": 1, "b": 2}]
        self._date_validator = _PerspectiveDateValidator()

    def data(self): return self._data

    def format(self): return 0

    def date_validator(self): return self._date_validator

    def row_count(self): return len(self._data)

    def marshal(self, cidx, i, type):
        return self._data[i][list(self._data[0].keys())[cidx]]
