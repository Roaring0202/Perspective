/******************************************************************************
 *
 * Copyright (c) 2017, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import _ from "underscore";

import {bindTemplate, get_text_width} from "@jpmorganchase/perspective-viewer/src/js/utils.js";;

import perspective from "@jpmorganchase/perspective";
import template from "../html/editor-filter-element.html";

import "../less/hypergrid.less";


@bindTemplate(template)
class FilterInput extends HTMLElement {

    get value(){
        return JSON.parse(this.getAttribute('value'));
    }

    set value(f) {
        const filter_dropdown = this.querySelector('#editor-filter-operator');
        const filter = JSON.parse(this.getAttribute('value'));
        const operator = filter.operator || perspective.FILTER_DEFAULTS[this.getAttribute('type')];
        if (filter_dropdown.value !== filter.operator) {
            filter_dropdown.value = operator;
            filter_dropdown.setAttribute('value', operator);
        }
        filter_dropdown.style.width = get_text_width(operator);
        const filter_input = this.querySelector('#editor-filter-operand');
        const operand = (filter.operand != null && filter.operand != undefined) ? filter.operand.toString() : "";
        if (!this._initialized) {
            filter_input.value = operand;
        }
        filter_input.style.minWidth = get_text_width(operand, 30);
    }

    set type(t) {
        let type = this.getAttribute('type');
        if (!type) return;
        let filter_dropdown = this.querySelector('#editor-filter-operator');
        switch (type) {
            case "float":
            case "integer":
                filter_dropdown.innerHTML = perspective.TYPE_FILTERS.float.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
                break;
            case "boolean":
                filter_dropdown.innerHTML = perspective.TYPE_FILTERS.boolean.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
                break;
            case 'date':
                filter_dropdown.innerHTML = perspective.TYPE_FILTERS.date.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
                break;
            case "string":
                filter_dropdown.innerHTML = perspective.TYPE_FILTERS.string.map(agg => 
                    `<option value="${agg}">${agg}</option>`
                ).join('');
            default:
        }
        const filter = JSON.parse(this.getAttribute('value'));
        const operator = filter.operator || perspective.FILTER_DEFAULTS[this.getAttribute('type')];
        filter_dropdown.value = operator;
        filter_dropdown.setAttribute('value', operator);
        let filter_operand = this.querySelector('#editor-filter-operand');
        this._callback = event => this._update_filter(event);
        filter_operand.addEventListener('keyup', event => {
            if (filter_operand.value !== 'in') {
                this._callback(event);
            }
        });
    }

    _update_filter(event) {
        let filter_operand = this.querySelector('#editor-filter-operand');
        let filter_operator = this.querySelector('#editor-filter-operator');
        let val = filter_operand.value;
        let type = this.getAttribute('type');
        switch (type) {
            case "float":
                val = parseFloat(val);
                break;
            case "integer":
                val = parseInt(val);
                break;
            case "boolean":
                val = val.toLowerCase().indexOf('true') > -1;
                break;
            case "string":
            default:
        }
        if (filter_operator.value === "in") {
            val = val.split(',').map(x => x.trim());
        }
        this.setAttribute('value', JSON.stringify({operator: filter_operator.value, operand: val}));
        this.dispatchEvent(new CustomEvent('filter-selected', {detail: event}));   
    }

    connectedCallback() {
        let operand = this.querySelector('#editor-filter-operand');
        let operator = this.querySelector('#editor-filter-operator');
        let debounced_filter = _.debounce(event => this._update_filter(event), 50);
        this.value = this.getAttribute('value');
        operator.addEventListener('change', () => {
            operator.setAttribute( 'value', operator.value);
            operator.style.width = get_text_width(operator.value);
            const filter_input = this.querySelector('#editor-filter-operand');
            filter_input.style.minWidth = get_text_width("" + operand.value, 30);    
            debounced_filter();
        });
        
    }

    focus(){
        let operand = this.querySelector('#editor-filter-operand');
        operand.focus();
    }
};


