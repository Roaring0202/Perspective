/******************************************************************************
 *
 * Copyright (c) 2017, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import perspective from "@jpmorganchase/perspective";

export const FilterSubGrid =  require('datasaur-local').extend('FilterSubGrid',{
    initialize: function(datasaur, options){
        this.grid = options.grid;
        this.filters = {};
    },
    type: 'filter',
    format: 'string',
    name: 'filter',
    setFilters: function(filters){
        this.filters = filters.length && filters.length > 0 ? 
                filters.reduce((map, filter) => { map[filter[0]] = filter; return map; }, {}) : {};
    },
    getRowCount: function(){
        return this.grid.properties.showFilterRow ? 1:0;
    },
    _getColumnHeader: function(x){
        const column = this.grid.behavior.getActiveColumn(x);
        var header = column.header.split(column.dataModel.groupHeaderDelimiter);
        header = header[header.length-1];
        return header;
    },
    getValue: function(x,y){
        const header = this._getColumnHeader(x);
        const filter = this.filters[header];
        return filter != undefined ? filter[1] + ' ' + filter[2] : '';
    },
    setValue: function(x,y,value){
        const header = this._getColumnHeader(x);

        const operator = value[0];
        const operand = value[1];

        if (!value || !operand){
            delete this.filters[header];
            return;
        }

        const column = this.grid.behavior.getActiveColumn(x);
        let type = column.type;
        let val = undefined;
        switch (type) {
            case "float":
                val = parseFloat(operand);
                break;
            case "integer":
                val = parseInt(operand);
                break;
            case "boolean":
                val = operand.toLowerCase().indexOf('true') > -1;
                break;
            case "string":
            default:
                val = operand;
        }
        
        const filter = [ header, operator, val ];
        this.filters[header] = filter;
    },
    getRow: function(y){
        const row = [];
        for(let x=0;x<this.grid.behavior.columns.length; x++){
            row.push(this.getValue(x,y));
        }
        return row;
    },
    // Return the cell renderer
    getCell: function(config, rendererName) {
        
        return config.grid.cellRenderers.get("Filter");
    },

    // Return the cell editor for a given (x,y) cell coordinate
    getCellEditorAt: function(x, y, declaredEditorName, cellEvent) {
        cellEvent.format = "";
        cellEvent.filter = this.filters[this._getColumnHeader(x)];
        return cellEvent.grid.cellEditors.create( 'Filter', cellEvent );
    }

});
