/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import {h} from "@phosphor/virtualdom";
import {TabBar} from "@phosphor/widgets";

export enum TabBarActions {
    Maximize = "maximize"
}

export class PerspectiveTabBarRenderer extends TabBar.Renderer {
    private maximized = false;

    constructor(maximized: boolean) {
        super();
        this.maximized = maximized;
    }

    public renderTab(data: any) {
        const title = data.title.caption;
        const key = this.createTabKey(data);
        const style = this.createTabStyle(data);
        let className = this.createTabClass(data);
        const dataset = this.createTabDataset(data);

        // move these to tabbar as well
        if (!this.maximized) {
            className += ` p-mod-maximize`;
        } else {
            className += ` p-mod-minimize`;
        }
        if (data.title.updating) {
            className += " perspective_updating";
        }

        return h.li(
            {key, className, title, style, dataset},
            this.renderLoadingIcon(),
            this.renderLabel(data),
            h.div({className: "p-TabBar-tabLinkIcon"}),
            this.renderMaximizeIcon(),
            this.renderCloseIcon(data),
            h.div({className: "divider"}),
            h.div({className: "shadow"})
        );
    }

    public renderMaximizeIcon() {
        const name = "p-TabBar-tabMaximizeIcon";
        return h.div({className: name, id: TabBarActions.Maximize});
    }

    public renderLoadingIcon() {
        return h.div({className: "p-TabBar-tabLoadingIcon"}, h.div({className: "p-TabBar-tabLoadingAnimation"}));
    }
}
