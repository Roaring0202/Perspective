/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import {ArrayExt} from "@phosphor/algorithm";
import {ElementExt} from "@phosphor/domutils";
import {Message} from "@phosphor/messaging";
import {Signal} from "@phosphor/signaling";
import {TabBar, Widget} from "@phosphor/widgets";
import {TabBarActions} from "./tabbarrenderer";

export class PerspectiveTabBar extends TabBar<Widget> {
    public tabLinkRequested: Signal<TabBar<Widget>, any>;
    private signals: any;
    private __content_node__: HTMLUListElement;

    constructor(options: TabBar.IOptions<Widget> = {}) {
        super(options);
        this.signals = {
            link: new Signal(this),
            [TabBarActions.Maximize]: new Signal(this)
        };
        this.tabLinkRequested = new Signal(this);
        this.__content_node__;
    }

    public get tabMaximizeRequested() {
        return this.signals[TabBarActions.Maximize];
    }

    public onUpdateRequest(msg: Message) {
        // NOT INERT!  This is a phosphor bug fix.
        // phosphor/virtualdom keeps a weakmap on contentNode which is later
        // reset - this causes the diff to double some elements.  Memoizing
        // prevent collection from the weakmap.
        this.__content_node__ = this.contentNode;
        super.onUpdateRequest(msg);
    }

    public handleEvent(event: any) {
        switch (event.type) {
            case "mousedown":
                if (event.button !== 0) {
                    return;
                }
                const tabs = this.contentNode.children;

                // Find the index of the released tab.
                const index = ArrayExt.findFirstIndex(tabs, tab => {
                    return ElementExt.hitTest(tab, event.clientX, event.clientY);
                });

                if (index < 0) {
                    break;
                }

                const title = this.titles[index];
                // Emit the close requested signal if the maximize icon was released.

                const signal = this.signals[event.target.id];
                signal && signal.emit({index, title});
                break;
        }
        super.handleEvent(event);
    }

    public onResize(msg: Widget.ResizeMessage) {
        super.onResize(msg);
        this.checkCondensed(msg);
    }

    public link() {
        this.node.classList.add("linked");
    }

    public unlink() {
        this.node.classList.remove("linked");
    }

    public checkCondensed(msg: Widget.ResizeMessage) {
        const approx_width = (msg ? msg.width : this.node.offsetWidth) / this.contentNode.children.length;
        if (approx_width < 400) {
            this.node.classList.add("condensed");
        } else {
            this.node.classList.remove("condensed");
        }
    }
}
