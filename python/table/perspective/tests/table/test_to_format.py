import numpy as np
from perspective.table import Table


class TestToFormat(object):

    # to_dict

    def test_to_dict_int(self):
        data = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_dict() == data

    def test_to_dict_float(self):
        data = [{"a": 1.5, "b": 2.5}, {"a": 3.5, "b": 4.5}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_dict() == data

    def test_to_dict_string(self):
        data = [{"a": "string1", "b": "string2"}, {"a": "string3", "b": "string4"}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_dict() == data

    def test_to_dict_none(self):
        data = [{"a": None, "b": 1}, {"a": None, "b": 2}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_dict() == data

    def test_to_dict_one(self):
        data = [{"a": 1, "b": "string1"}, {"a": 1, "b": "string2"}]
        tbl = Table(data)
        view = tbl.view({
            "row-pivots": ["a"]
        })
        assert view.to_dict() == [
            {"__ROW_PATH__": [], "a": 2, "b": 2}, {"__ROW_PATH__": ["1"], "a": 2, "b": 2}
        ]

    def test_to_dict_two(self):
        data = [{"a": 1, "b": "string1"}, {"a": 1, "b": "string2"}]
        tbl = Table(data)
        view = tbl.view({
            "row-pivots": ["a"],
            "column-pivots": ["b"]
        })
        assert view.to_dict() == [
            {"__ROW_PATH__": [], "string1|a": 1, "string1|b": 1, "string2|a": 1, "string2|b": 1},
            {"__ROW_PATH__": ["1"], "string1|a": 1, "string1|b": 1, "string2|a": 1, "string2|b": 1},
        ]

    def test_to_dict_column_only(self):
        data = [{"a": 1, "b": "string1"}, {"a": 1, "b": "string2"}]
        tbl = Table(data)
        view = tbl.view({
            "column-pivots": ["b"]
        })
        assert view.to_dict() == [
            {"string1|a": 1, "string1|b": "string1", "string2|a": None, "string2|b": None},
            {"string1|a": None, "string1|b": None, "string2|a": 1, "string2|b": "string2"},
        ]

    # to_columns

    def test_to_columns_int(self):
        data = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_columns() == {
            "a": [1, 3],
            "b": [2, 4]
        }

    def test_to_columns_float(self):
        data = [{"a": 1.5, "b": 2.5}, {"a": 3.5, "b": 4.5}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_columns() == {
            "a": [1.5, 3.5],
            "b": [2.5, 4.5]
        }

    def test_to_columns_bool(self):
        data = [{"a": True, "b": False}, {"a": True, "b": False}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_columns() == {
            "a": [True, True],
            "b": [False, False]
        }

    def test_to_columns_string(self):
        data = [{"a": "string1", "b": "string2"}, {"a": "string3", "b": "string4"}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_columns() == {
            "a": ["string1", "string3"],
            "b": ["string2", "string4"]
        }

    def test_to_columns_none(self):
        data = [{"a": None, "b": None}, {"a": None, "b": None}]
        tbl = Table(data)
        view = tbl.view()
        assert view.to_columns() == {
            "a": [None, None],
            "b": [None, None]
        }

    def test_to_columns_one(self):
        data = [{"a": 1, "b": 2}, {"a": 1, "b": 2}]
        tbl = Table(data)
        view = tbl.view({
            "row-pivots": ["a"]
        })
        assert view.to_columns() == {
            "__ROW_PATH__": [[], ["1"]],
            "a": [2, 2],
            "b": [4, 4]
        }

    def test_to_columns_two(self):
        data = [{"a": 1, "b": 2}, {"a": 1, "b": 2}]
        tbl = Table(data)
        view = tbl.view({
            "row-pivots": ["a"],
            "column-pivots": ["b"]
        })
        assert view.to_columns() == {
            "__ROW_PATH__": [[], ["1"]],
            "2|a": [2, 2],
            "2|b": [4, 4]
        }

    def test_to_columns_column_only(self):
        data = [{"a": 1, "b": 2}, {"a": 1, "b": 2}]
        tbl = Table(data)
        view = tbl.view({
            "column-pivots": ["b"]
        })
        assert view.to_columns() == {
            "2|a": [1, 1],
            "2|b": [2, 2],
        }

    # to_numpy

    def test_to_numpy_int(self):
        data = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        tbl = Table(data)
        view = tbl.view()
        v = view.to_numpy()
        assert np.array_equal(v["a"], np.array([1, 3]))
        assert np.array_equal(v["b"], np.array([2, 4]))

    def test_to_numpy_float(self):
        data = [{"a": 1.5, "b": 2.5}, {"a": 3.5, "b": 4.5}]
        tbl = Table(data)
        view = tbl.view()
        v = view.to_numpy()
        assert np.array_equal(v["a"], np.array([1.5, 3.5]))
        assert np.array_equal(v["b"], np.array([2.5, 4.5]))

    def test_to_numpy_bool(self):
        data = [{"a": True, "b": False}, {"a": True, "b": False}]
        tbl = Table(data)
        view = tbl.view()
        v = view.to_numpy()
        assert np.array_equal(v["a"], np.array([True, True]))
        assert np.array_equal(v["b"], np.array([False, False]))

    def test_to_numpy_string(self):
        data = [{"a": "string1", "b": "string2"}, {"a": "string3", "b": "string4"}]
        tbl = Table(data)
        view = tbl.view()
        v = view.to_numpy()
        assert np.array_equal(v["a"], np.array(["string1", "string3"]))
        assert np.array_equal(v["b"], np.array(["string2", "string4"]))

    def test_to_numpy_none(self):
        data = [{"a": None, "b": None}, {"a": None, "b": None}]
        tbl = Table(data)
        view = tbl.view()
        v = view.to_numpy()
        assert np.array_equal(v["a"], np.array([None, None]))
        assert np.array_equal(v["b"], np.array([None, None]))

    def test_to_numpy_one(self):
        data = [{"a": 1, "b": 2}, {"a": 1, "b": 2}]
        tbl = Table(data)
        view = tbl.view({
            "row-pivots": ["a"]
        })
        v = view.to_numpy()
        assert np.array_equal(v["__ROW_PATH__"], [[], ["1"]])
        assert np.array_equal(v["a"], np.array([2, 2]))
        assert np.array_equal(v["b"], np.array([4, 4]))

    def test_to_numpy_two(self):
        data = [{"a": 1, "b": 2}, {"a": 1, "b": 2}]
        tbl = Table(data)
        view = tbl.view({
            "row-pivots": ["a"],
            "column-pivots": ["b"]
        })
        v = view.to_numpy()
        assert np.array_equal(v["__ROW_PATH__"], [[], ["1"]])
        assert np.array_equal(v["2|a"], np.array([2, 2]))
        assert np.array_equal(v["2|b"], np.array([4, 4]))

    def test_to_numpy_column_only(self):
        data = [{"a": 1, "b": 2}, {"a": 1, "b": 2}]
        tbl = Table(data)
        view = tbl.view({
            "column-pivots": ["b"]
        })
        v = view.to_numpy()
        assert np.array_equal(v["2|a"], np.array([1, 1]))
        assert np.array_equal(v["2|b"], np.array([2, 2]))
