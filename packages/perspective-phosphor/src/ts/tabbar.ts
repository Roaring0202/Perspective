/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import { TabBar, Widget} from '@phosphor/widgets';
import {h} from "@phosphor/virtualdom";
import {Signal} from "@phosphor/signaling";
import {Message} from '@phosphor/messaging';
import {ArrayExt} from "@phosphor/algorithm";
import {ElementExt} from "@phosphor/domutils";

enum TabBarActions {
    Maximize = 'maximize'
}

export class PerspectiveTabBarRenderer extends TabBar.Renderer {
    private maximized: boolean = false

    constructor(maximized: boolean) {
        super();
        this.maximized = maximized;
    }

    renderTab(data: any) {
        let title = data.title.caption;
        let key = this.createTabKey(data);
        let style = this.createTabStyle(data);
        let className = this.createTabClass(data);
        let dataset = this.createTabDataset(data);
        
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

    renderMaximizeIcon() {
        let name = "p-TabBar-tabMaximizeIcon";
        return h.div({className: name, id: TabBarActions.Maximize});
    }


    renderLoadingIcon() {
        return h.div(
            {className: "p-TabBar-tabLoadingIcon"},
            h.div({className: "spinner spinner--small"}, h.div({className: "spinner__dot1"}), h.div({className: "spinner__dot2"}), h.div({className: "spinner__dot3"}))
        );
    }
}

export class PerspectiveTabBar extends TabBar<Widget> {
    public tabLinkRequested: Signal<TabBar<Widget>, any>;
    private signals: any;
    private __content_node__: HTMLUListElement

    constructor(options: TabBar.IOptions<Widget> = {}) {
        super(options);
        this.signals = { 
            link: new Signal(this),
            [TabBarActions.Maximize]: new Signal(this)
        }
        this.tabLinkRequested = new Signal(this);
        this.__content_node__
    }

    public get tabMaximizeRequested(){
        return this.signals[TabBarActions.Maximize]
    }

    onUpdateRequest(msg: Message) {
        // NOT INERT!  This is a phosphor bug fix.
        // phosphor/virtualdom keeps a weakmap on contentNode which is later
        // reset - this causes the diff to double some elements.  Memoizing
        // prevent collection from the weakmap.
        this.__content_node__ = this.contentNode;
        super.onUpdateRequest(msg);
    }

    handleEvent(event: any) {
        switch (event.type) {
            case "mousedown":
                if (event.button !== 0) {
                    return;
                }
                let tabs = this.contentNode.children;

                // Find the index of the released tab.
                let index = ArrayExt.findFirstIndex(tabs, tab => {
                    return ElementExt.hitTest(tab, event.clientX, event.clientY);
                });

                if (index < 0) break;

                let title = this.titles[index];
                // Emit the close requested signal if the maximize icon was released.

                const signal = this.signals[event.target.id]
                signal && signal.emit({index, title})
                break;
        }
        super.handleEvent(event);
    }

    onResize(msg: Widget.ResizeMessage) {
        super.onResize(msg);
        this.checkCondensed(msg);
    }

    public link(){
        this.node.classList.add("linked");
    }

    public unlink(){
        this.node.classList.remove("linked");
    }

    checkCondensed(msg: Widget.ResizeMessage) {
        const approx_width = (msg ? msg.width : this.node.offsetWidth) / this.contentNode.children.length;
        if (approx_width < 400) {
            this.node.classList.add("condensed");
        } else {
            this.node.classList.remove("condensed");
        }
    }
}