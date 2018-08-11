
const CellEditor = require('fin-hypergrid/src/cellEditors/CellEditor');
import perspective from "@jpmorganchase/perspective";
import {get_text_width} from "@jpmorganchase/perspective-viewer/src/js/utils.js";


/**
 * @constructor
 * @extends CellEditor
 */
export const FilterEditor = CellEditor.extend('FilterEditor', {
    name: "Filter",
    template: '<div class="hypergrid-textfield">' +
                '<div style="display:flex;flex-direction:row;overflow:hidden">' +
                    '<select id="editor-filter-operator" style="padding-right:5px;"></select>' +
                    '<input id="editor-filter-operand" placeholder="Value" lang="{{locale}}" style="{style}}"></input>'+
                '</div>' +
            '</div>',
    
    initialize: function(grid, options) {
        const type = options.columnProperties.type;
        this.initialValue = options.filter ? [options.filter[1],options.filter[2]] : [perspective.FILTER_DEFAULTS[type], ''];
        this.operator = this.el.querySelector('#editor-filter-operator');
        this.operand = this.el.querySelector('#editor-filter-operand');
        this.input = this.operand;
        this.errors = 0;
        
        this.input.onclick = (e) => {
            e.stopPropagation(); // ignore clicks in the text FIELD
        };
        this.operator.addEventListener('change', event => {
            this.operator.style.width = get_text_width(this.operator.value);
        });
        this.input.onfocus = (e) => {
            var target = e.target;
            this.el.style.outline = this.outline = this.outline || window.getComputedStyle(target).outline;
            target.style.outline = 0;
        };
        this.input.onblur = () => {
            this.el.style.outline = 0;
        };

        switch (type) {
            case "float":
            case "integer":
                this.operator.innerHTML = perspective.TYPE_FILTERS.float.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
                break;
            case "boolean":
                this.operator.innerHTML = perspective.TYPE_FILTERS.boolean.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
                break;
            case 'date':
                this.operator.innerHTML = perspective.TYPE_FILTERS.date.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
                break;
            case "string":
                this.operator.innerHTML = perspective.TYPE_FILTERS.string.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
            default:
        }
        
        this.operator.value = options.filter ? options.filter[0].toString() : perspective.FILTER_DEFAULTS[type];
        this.operator.style.width = get_text_width(this.operator.value);
        this.operand.value = options.filter ? options.filter[1].toString(): "";
    },

    setEditorValue: function(value) {
        const operator = value[0];
        const operand = value[1];
        this.operator.value = operator;
        this.operator.style.width = get_text_width(this.operator.value);
        this.input.value = this.localizer.format(operand);
    },
    getEditorValue: function() {
        return [ this.operator.value, this.localizer.parse(this.input.value)];
    },
    

});
