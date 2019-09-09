COLUMN_SEPARATOR_STRING = "|"

NUMBER_AGGREGATES = [
    "any",
    "avg",
    "count",
    "distinct count",
    "dominant",
    "first by index",
    "last by index",
    "last",
    "high",
    "low",
    "mean",
    "mean by count",
    "median",
    "pct sum parent",
    "pct sum grand total",
    "sum",
    "sum abs",
    "sum not null",
    "unique"
]

STRING_AGGREGATES = ["any", "count", "distinct count", "distinct leaf", "dominant", "first by index", "last by index", "last", "mean by count", "unique"]

BOOLEAN_AGGREGATES = ["any", "count", "distinct count", "distinct leaf", "dominant", "first by index", "last by index", "last", "mean by count", "unique", "and", "or"]

SORT_ORDERS = ["none", "asc", "desc", "col asc", "col desc", "asc abs", "desc abs", "col asc abs", "col desc abs"]

SORT_ORDER_IDS = [2, 0, 1, 0, 1, 3, 4, 3, 4]

TYPE_AGGREGATES = {
    "string": STRING_AGGREGATES,
    "float": NUMBER_AGGREGATES,
    "integer": NUMBER_AGGREGATES,
    "boolean": BOOLEAN_AGGREGATES,
    "datetime": STRING_AGGREGATES,
    "date": STRING_AGGREGATES
}

FILTER_OPERATORS = {
    "lessThan": "<",
    "greaterThan": ">",
    "equals": "==",
    "lessThanOrEquals": "<=",
    "greaterThanOrEquals": ">=",
    "doesNotEqual": "!=",
    "isNull": "is null",
    "isNotNull": "is not null",
    "isIn": "in",
    "isNotIn": "not in",
    "contains": "contains",
    "bitwiseAnd": "&",
    "bitwiseOr": "|",
    "and": "and",
    "or": "or",
    "beginsWith": "begins with",
    "endsWith": "ends with"
}

BOOLEAN_FILTERS = [
    FILTER_OPERATORS["bitwiseAnd"],
    FILTER_OPERATORS["bitwiseOr"],
    FILTER_OPERATORS["equals"],
    FILTER_OPERATORS["doesNotEqual"],
    FILTER_OPERATORS["or"],
    FILTER_OPERATORS["and"],
    FILTER_OPERATORS["isNull"],
    FILTER_OPERATORS["isNotNull"]
]

NUMBER_FILTERS = [
    FILTER_OPERATORS["lessThan"],
    FILTER_OPERATORS["greaterThan"],
    FILTER_OPERATORS["equals"],
    FILTER_OPERATORS["lessThanOrEquals"],
    FILTER_OPERATORS["greaterThanOrEquals"],
    FILTER_OPERATORS["doesNotEqual"],
    FILTER_OPERATORS["isNull"],
    FILTER_OPERATORS["isNotNull"]
]

STRING_FILTERS = [
    FILTER_OPERATORS["equals"],
    FILTER_OPERATORS["contains"],
    FILTER_OPERATORS["doesNotEqual"],
    FILTER_OPERATORS["isIn"],
    FILTER_OPERATORS["isNotIn"],
    FILTER_OPERATORS["beginsWith"],
    FILTER_OPERATORS["endsWith"],
    FILTER_OPERATORS["isNull"],
    FILTER_OPERATORS["isNotNull"]
]

DATETIME_FILTERS = [
    FILTER_OPERATORS["lessThan"],
    FILTER_OPERATORS["greaterThan"],
    FILTER_OPERATORS["equals"],
    FILTER_OPERATORS["lessThanOrEquals"],
    FILTER_OPERATORS["greaterThanOrEquals"],
    FILTER_OPERATORS["doesNotEqual"],
    FILTER_OPERATORS["isNull"],
    FILTER_OPERATORS["isNotNull"]
]

TYPE_FILTERS = {
    "string": STRING_FILTERS,
    "float": NUMBER_FILTERS,
    "integer": NUMBER_FILTERS,
    "boolean": BOOLEAN_FILTERS,
    "datetime": DATETIME_FILTERS,
    "date": DATETIME_FILTERS
}
