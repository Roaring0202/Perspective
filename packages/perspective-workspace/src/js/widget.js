import {PerspectiveWidget, PSP_CONTAINER_CLASS, PSP_CONTAINER_CLASS_DARK, PSP_CLASS} from "@finos/perspective-phosphor";
// import {Widget} from "@phosphor/widgets";

let ID_COUNTER = 0;

export class PerspectiveWorkspaceWidget extends PerspectiveWidget {
    constructor(options) {
        const {name, root} = options;
        super(name, {createNode: PerspectiveWorkspaceWidget.createNode, bindto: root});
        this._workspace = root;
        this._loaded = true;
    }

    get datasource() {
        return this._datasource;
    }

    set datasource(name) {
        this._datasource = name;
        this.subscribe();
    }

    subscribe() {
        this._loaded = false;
        if (this.isAttached) {
            this._subscribe();
        }
    }

    set dark(dark) {
        this._dark = dark;
        if (this._dark) {
            this.viewer.classList.add(PSP_CONTAINER_CLASS_DARK);
            this.viewer.classList.remove(PSP_CONTAINER_CLASS);
        } else {
            this.viewer.classList.add(PSP_CONTAINER_CLASS);
            this.viewer.classList.remove(PSP_CONTAINER_CLASS_DARK);
        }
        if (this.isAttached) {
            this.viewer.restyleElement();
        }
    }

    _subscribe() {
        if (this.datasource && this._loaded === false) {
            const datasource = this._workspace.getDatasource(this.datasource);
            datasource && datasource.then(table => this.viewer.load(table));
            this._loaded = true;
        }
    }

    _restore(config) {
        const {datasource, theme} = config;
        this.datasource = datasource;
        this.theme = theme;
        return this.viewer.restore(config);
    }

    restore(config) {
        if (this.isAttached) {
            this._restore(config);
        } else {
            this._config = config;
        }
    }

    save() {
        return {
            ...this.viewer.save(),
            datasource: this.datasource,
            theme: this.theme
        };
    }

    duplicate() {
        const widget = new PerspectiveWorkspaceWidget({name: "duplicate", root: this._workspace});
        widget.restore(this.save());
        return widget;
    }

    onAfterAttach(msg) {
        super.onAfterAttach(msg);
        this._config && this.restore(this._config);
        delete this._config;
    }

    static createNode(root) {
        const slot = document.createElement("slot");
        const name = `AUTO_ID_${ID_COUNTER++}`;
        slot.setAttribute("name", name);

        const node = document.createElement("div");
        node.classList.add("p-Widget");
        node.classList.add(PSP_CONTAINER_CLASS);
        node.appendChild(slot);

        const viewer = document.createElement("perspective-viewer");
        viewer.classList.add(PSP_CLASS);
        root.appendChild(viewer);
        viewer.setAttribute("slot", name);

        return {node, viewer};
    }
}
