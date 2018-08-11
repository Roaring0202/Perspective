import {FilterSubGrid} from "./hypergrid-filter-subgrid.js";
import {FilterEditor} from "./hypergrid-filter-editor.js";


var CLASS_NAME = 'Filter';

var prototypeFilterRenderer = {
    /**
     * @summary The filter paint function.
     * @desc This is the heart of the cell renderer.
     *
     * If you feel the urge to override this, you are on the wrong path! Write a new cell renderer instead.
     * @implements paintFunction
     * @default
     * @memberOf FilterRenderer#
     */
    paint: paintFilterCell,
}

function installPlugin(grid, options) {
    options = options || {};

    var CellRenderer = options.CellRenderer || 'SimpleCell';
    if (typeof CellRenderer === 'string') {
        CellRenderer = grid.cellRenderers.get(CellRenderer).constructor;
    }

    if (typeof CellRenderer !== 'function') {
        throw new grid.HypergridError('Expected `options.CellRenderer` to be a constructor or a registered cell redenderer.');
    }

    var FilterRenderer = CellRenderer.extend(CLASS_NAME, prototypeFilterRenderer);
    grid.cellRenderers.add(FilterRenderer);

    grid.cellEditors.add( "Filter", FilterEditor);
    grid.addProperties({'subgrids':['HeaderSubgrid', FilterSubGrid, 'data']});
}

function paintFilterCell(gc, config) {
    var paint = this.super.paint,
        columnIndex = config.gridCell.x,
        rect = config.bounds,
        prevVisCol = config.grid.renderer.visibleColumns.find(function (visCol) {
            return visCol.columnIndex === columnIndex - 1;
        });

    if (prevVisCol && prevVisCol.top) {
        prevVisCol.top = prevVisCol.top + rect.height;
    }

    paint.call(this, gc, config);

}

// `install` makes this a Hypergrid plug-in
exports.install = function(grid, options) {
    installPlugin(grid, options);
};
