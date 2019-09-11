# *****************************************************************************
#
# Copyright (c) 2019, the Perspective Authors.
#
# This file is part of the Perspective library, distributed under the terms of
# the Apache License 2.0.  The full license can be found in the LICENSE file.
#
from dateutil.parser import parse


class _PerspectiveDateValidator(object):
    '''Internal Class for date validation'''

    def is_valid(self, str):
        try:
            parse(str)
            return True
        except ValueError:
            return False
