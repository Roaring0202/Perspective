from perspective.table import Table


class TestToFormat(object):

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
