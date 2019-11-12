export class Datasources {
    constructor(workspace) {
        this._workspace = workspace;
        this._datasources = new Map();
    }

    addDatasource(name, datasource) {
        if (!(datasource instanceof Promise)) {
            throw "datasource must be of type Promise<Table>";
        }
        if (this._datasources.has(name)) {
            console.warn(`Overriding existing '${name}' datasource`);
        }
        this._datasources.set(name, datasource);
        this._workspace.widgets.map(widget => {
            if (widget.datasource === name) {
                widget.subscribe();
            }
        });
    }

    getDatasource(name) {
        return this._datasources.get(name);
    }

    getDatasources() {
        return {...this._datasources};
    }
}
