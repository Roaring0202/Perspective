
const CellEditor = require('fin-hypergrid/src/cellEditors/CellEditor');
import perspective from "@jpmorganchase/perspective";
import "./editor-filter-element.js";

/**
 * @constructor
 * @extends CellEditor
 */
export const FilterEditor = CellEditor.extend('FilterEditor', {
    name: "Filter",
    template: '<div class="editor-filter">' +
                '<div id="editor-filter-container" lang="{{locale}}" style="{style}}">' + 
                '<span id="editor-filter-add">+</span>' +
                '</div>' + 
              '</div>',

    set value(val){
        this._value = val;
    },

    get value(){
        return this._value;
    },

    initialize: function(grid, options) {
        this.type = options.columnProperties.type;
        
        if (options.filter && options.filter.length > 0){
            const value = [];
            options.filter.forEach(element => {
                value.push({operator: element[1], operand: element[2]});
            });
            this._value = value;
        }
        else{
            this._value = [this._getDefaultFilter()];
        }
        this.initialValue = this.value;
        this.input = this;
        this.errors = 0;
       
        const add = this.el.querySelector( '#editor-filter-add');
        add.addEventListener( 'click', () => {
            this._addFilterRow( this._getDefaultFilter() );
        });
    },

    _getDefaultFilter: function(){
        return {operator: perspective.FILTER_DEFAULTS[this.type], operand: ''};
    },

    _addFilterRow: function(filter){
        const container = this.el.querySelector( '#editor-filter-container');
        const addFilter = this.el.querySelector( '#editor-filter-add');
        let input = document.createElement("hypergrid-filter-input");
        input.setAttribute('type', this.type);
        input.setAttribute('value', JSON.stringify( filter ) );

        input.addEventListener("filter-selected", ()=>{
            console.log( 'filter selected');
        });
        container.insertBefore(input, addFilter);
    },
    setEditorValue: function(value) {
        const inputs = this.el.querySelectorAll( 'hypergrid-filter-input');
        const container = this.el.querySelector( '#editor-filter-container');

        inputs.forEach( input =>{
            container.removeChild(input);
        });
        value.forEach(filter => {
            this._addFilterRow(filter);
        });
    },
    getEditorValue: function() {
        const inputElements = this.el.querySelectorAll( 'hypergrid-filter-input');
        var value = [];
        inputElements.forEach(element=>{
            const filter = element.value;
            value.push([filter.operator, filter.operand]);
        });
        return value;
    },
    focus: function(){
        const inputElements = this.el.querySelector( 'hypergrid-filter-input');
        inputElements.focus();
    }
    

});
