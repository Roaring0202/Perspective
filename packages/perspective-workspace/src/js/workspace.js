/******************************************************************************
 *
 * Copyright (c) 2017, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import style from "../less/workspace.less";
import {PerspectiveWorkspaceWidget} from "./widget";
import {Datasources} from "./datasources";
import template from "../html/workspace.html";
import {bindTemplate} from "@finos/perspective-viewer/dist/esm/utils";
import {PerspectiveWorkspace as PerspectivePhosphorWorkspace, PerspectiveDockPanel} from "@finos/perspective-phosphor";
import {MessageLoop} from "@phosphor/messaging";
import {Widget} from "@phosphor/widgets";
import {toArray} from "@phosphor/algorithm";
import "!!style-loader!css-loader!less-loader!../less/index.less";

const DEFAULT_WORKSPACE_SIZE = [1, 3];

@bindTemplate(template, style) // eslint-disable-next-line no-unused-vars
class PerspectiveWorkspace extends HTMLElement {
    save() {
        const master = {
            widgets: this.masterpanel.widgets.map(widget => widget.save()),
            sizes: [...this.masterpanel.relativeSizes()]
        };

        return {
            sizes: [...this.relativeSizes()],
            detail: this.dockpanel.save(),
            master
        };
    }

    clearLayout() {
        this.widgets.forEach(widget => widget.close());
        this.workspace.dockpanel.close();

        if (this.workspace.masterpanel.isAttached) {
            this.workspace.masterpanel.close();
        }
    }

    restore(layout) {
        this.clearLayout();
        if (layout.master && layout.master.widgets.length > 0) {
            this.workspace.addWidget(this.masterpanel);
            this.workspace.addWidget(this.workspace.dockpanel);
            this.workspace.setRelativeSizes(layout.sizes || DEFAULT_WORKSPACE_SIZE);
        } else {
            this.workspace.addWidget(this.workspace.dockpanel);
        }

        if (layout.master) {
            layout.master.widgets.forEach(widgetConfig => {
                const widget = new PerspectiveWorkspaceWidget({title: "test", root: this});
                widget.viewer.addEventListener("perspective-click", this.onMasterPerspectiveClick);
                widget.restore(widgetConfig);
                this.workspace.masterpanel.addWidget(widget);
            });
            layout.master.sizes && this.workspace.masterpanel.setRelativeSizes(layout.master.sizes);
        }

        const detailLayout = PerspectiveDockPanel.mapWidgets(widgetConfig => {
            // const slot = document.createElement("slot");
            const widget = new PerspectiveWorkspaceWidget({title: "test", root: this});
            widget.restore(widgetConfig);
            return widget;
        }, layout.detail);
        this.workspace.dockpanel.restoreLayout(detailLayout);
    }

    get widgets() {
        return [...this.workspace.masterpanel.widgets, ...toArray(this.workspace.dockpanel.widgets())];
    }

    addDatasource(name, datasource) {
        this._datasources.addDatasource(name, datasource);
    }
    getDatasource(name) {
        return this._datasources.getDatasource(name);
    }

    connectedCallback() {
        const container = this.shadowRoot.querySelector("#container");
        this.workspace = new PerspectivePhosphorWorkspace();

        MessageLoop.sendMessage(this.workspace, Widget.Msg.BeforeAttach);
        container.insertBefore(this.workspace.node, null);
        MessageLoop.sendMessage(this.workspace, Widget.Msg.AfterAttach);

        this._datasources = new Datasources(this);
        window.onresize = () => {
            this.workspace.update();
        };
    }
}
