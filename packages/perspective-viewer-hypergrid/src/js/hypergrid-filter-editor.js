
const CellEditor = require('fin-hypergrid/src/cellEditors/CellEditor');
import perspective from "@jpmorganchase/perspective";
import {get_text_width} from "@jpmorganchase/perspective-viewer/src/js/utils.js";
import "./editor-filter-element.js";

/**
 * @constructor
 * @extends CellEditor
 */
export const FilterEditor = CellEditor.extend('FilterEditor', {
    name: "Filter",
    template: '<div class="editor-filter">' +
                '<div id="editor-filter-container" lang="{{locale}}" style="{style}}">' + 
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
            this._value = [{operator: perspective.FILTER_DEFAULTS[this.type], operand: ''}];
        }
        this.initialValue = this.value;
        this.input = this;
        this.errors = 0;
       
    },

    setEditorValue: function(value) {
        const type = this.type;
        const container = this.el.querySelector( '#editor-filter-container');
        value.forEach(filter => {
            let input = document.createElement("hypergrid-filter-input");
            input.setAttribute('value', JSON.stringify( filter ) );
            input.setAttribute('type', type);
            input.addEventListener("filter-selected", ()=>{
                console.log( 'filter selected');
            });
            container.appendChild(input);
        });
    },
    getEditorValue: function() {
        const inputElements = this.el.querySelectorAll( 'hypergrid-filter-input');
        var value = [];
        inputElements.forEach(element=>{
            const filter = JSON.parse(element.value);
            value.push([filter.operator, filter.operand]);
        });
        return value;
    },
    focus: function(){
        const inputElements = this.el.querySelector( 'hypergrid-filter-input');
        inputElements.focus();
    }
    

});
