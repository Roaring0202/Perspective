
from perspective.table.libbinding import get_from_data_slice_zero, get_from_data_slice_one, get_from_data_slice_two
from ._constants import COLUMN_SEPARATOR_STRING


class _PerspectiveDataFormatter(object):
    '''Helper for exporting formatted data from Perspective.'''

    @staticmethod
    def to_format(options, view, column_names, data_slice, data):
        data_type = type(data).__name__
        for ridx in range(options["start_row"], options["end_row"]):
            row_path = data_slice.get_row_path(ridx) if options["has_row_path"] else []
            if options["leaves_only"] and len(row_path) < len(view._config.get_row_pivots()):
                continue

            if (data_type == "list"):
                data.append({})

            for cidx in range(options["start_col"], options["end_col"]):
                name = COLUMN_SEPARATOR_STRING.join([n.to_string(False) for n in column_names[cidx]])

                if (data_type == "dict"):
                    if name not in data:
                        data[name] = []

                if (cidx - (1 if view._sides > 0 else 0)) % (len(view._config.get_columns()) + view._num_hidden_cols()) >= len(view._config.get_columns()):
                    # don't emit columns used for hidden sort
                    continue
                elif cidx == options["start_col"] and view._sides > 0:
                    if options["has_row_path"]:
                        paths = [path.to_string(False) for path in row_path]
                        if data_type == "list":
                            data[ridx]["__ROW_PATH__"] = paths
                        else:
                            data["__ROW_PATH__"].append([path.to_string(False) for path in row_path])
                else:
                    if view._sides == 0:
                        value = get_from_data_slice_zero(data_slice, ridx, cidx)
                    elif view._sides == 1:
                        value = get_from_data_slice_one(data_slice, ridx, cidx)
                    else:
                        value = get_from_data_slice_two(data_slice, ridx, cidx)

                    if (data_type == "list"):
                        data[ridx][name] = value
                    else:
                        data[name].append(value)

        if data_type == "dict" and (not options["has_row_path"] and ("__ROW_PATH__" in data)):
            del data["__ROW_PATH__"]

        return data
