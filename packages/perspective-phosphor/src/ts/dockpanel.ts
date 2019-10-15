/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import {find} from "@phosphor/algorithm";
import {DockPanel, TabBar, Widget} from "@phosphor/widgets";
import {ContextMenu} from "@phosphor/widgets";
import "../less/layout.less";
import {createCommands} from "./contextmenu";
import {PerspectiveTabBar} from "./tabbar";
import {PerspectiveTabBarRenderer} from "./tabbarrenderer";
import {PerspectiveWidget} from "./widget";

// TODO:
//  - link/broadcast functionality
//  - don't save all attributes
//  - dark theme
//  - better names for duplicates
//  - add docs

class PerspectiveDockPanelRenderer extends DockPanel.Renderer {
    public dock: PerspectiveDockPanel;

    public createTabBar(): TabBar<Widget> {
        const tabbar = new PerspectiveTabBar({renderer: new PerspectiveTabBarRenderer(this.dock.maximized)});
        tabbar.addClass("p-DockPanel-tabBar");
        tabbar.tabMaximizeRequested.connect(this.dock.onTabMaximizeRequested, this);
        return tabbar;
    }
}

// tslint:disable-next-line: max-classes-per-file
export class PerspectiveDockPanel extends DockPanel {
    public id = "main";

    // this shouldn't be public
    public maximized: boolean;

    private commands: any;
    private minimizedLayout: any;

    constructor(name: string, options?: DockPanel.IOptions) {
        super({renderer: new PerspectiveDockPanelRenderer(), spacing: 14, ...options});

        // this isn't quite right
        (this as any)._renderer.dock = this;

        this.commands = createCommands(this);
        this.addTabbarEventListeners();
    }

    public duplicate(widget: PerspectiveWidget): void {
        const newWidget = new PerspectiveWidget(widget.name);
        newWidget.load(widget.table);
        newWidget.restore(widget.save()).then(() => {
            this.addWidget(newWidget, {mode: "split-right", ref: widget});
        });
    }

    public addWidget(widget: PerspectiveWidget, options: DockPanel.IAddOptions = {}): void {
        this.addWidgetEventListeners(widget);
        super.addWidget(widget, options);
    }

    public deserialize(layout: DockPanel.ILayoutConfig): void {
        const newLayout = this.mapWidgets(this.createWidget, layout);
        this.restoreLayout(newLayout);
    }

    public serialize() {
        const layout = this.saveLayout();
        return this.mapWidgets((widget: PerspectiveWidget) => widget.save(), layout);
    }

    /**
     * Handle the `tabMaximizeRequested` signal from a tab bar.
     */
    // rethink maximize
    public onTabMaximizeRequested = (sender: any, args: any) => {
        this.maximized = !this.maximized;
        if (this.maximized) {
            this.minimizedLayout = this.saveLayout();

            this.restoreLayout({
                main: {
                    currentIndex: 0,
                    type: "tab-area",
                    widgets: [args.title.owner]
                }
            });
        } else {
            this.restoreLayout(this.minimizedLayout);
            this.minimizedLayout = null;
        }
    };

    protected findTabbar(widget: PerspectiveWidget): PerspectiveTabBar {
        return find(this.tabBars(), bar => bar.titles[0].owner === widget) as PerspectiveTabBar;
    }

    private createMenu(widget: any) {
        const contextMenu = new ContextMenu({commands: this.commands});
        contextMenu.addItem({command: "perspective:duplicate", selector: ".p-Widget", args: {widget}});

        // could move these 3 to perspective widget
        contextMenu.addItem({command: "perspective:export", selector: ".p-Widget", args: {widget}});
        contextMenu.addItem({command: "perspective:copy", selector: ".p-Widget", args: {widget}});
        contextMenu.addItem({command: "perspective:reset", selector: ".p-Widget", args: {widget}});
        return contextMenu;
    }

    private showMenu(widget: PerspectiveWidget, event: MouseEvent) {
        // create menu in add widget instead??
        const menu = this.createMenu(widget);
        const menuOpened = menu.open(event);
        if (menuOpened) {
            event.preventDefault();
        }

        event.stopPropagation();
    }

    private addTabbarEventListeners() {
        this.node.addEventListener("contextmenu", event => {
            const tabBar = find(this.tabBars(), bar => {
                return bar.node.contains(event.target as Node);
            });
            this.showMenu(tabBar.titles[0].owner as PerspectiveWidget, event);
            event.preventDefault();
        });
    }

    private addWidgetEventListeners(widget: PerspectiveWidget) {
        widget.node.addEventListener("contextmenu", (event: MouseEvent) => this.showMenu(widget, event));
        widget.viewer.addEventListener("perspective-toggle-settings", (event: CustomEvent) => {
            widget.title.className = event.detail && "settings_open";
        });
    }

    private createWidget = (config: any) => {
        // TODO I should be able to instantiate perspective with the config in one call
        const widget = new PerspectiveWidget(config.name);
        widget.restore(config);
        this.addWidgetEventListeners(widget);
        return widget;
    };

    private mapWidgets(widgetFunc: any, layout: any) {
        if (layout.main) {
            layout.main = this.mapWidgets(widgetFunc, layout.main);
        } else if (layout.children) {
            layout.children = layout.children.map((x: any) => this.mapWidgets(widgetFunc, x));
        } else if (layout.widgets) {
            layout.widgets = layout.widgets.map((x: any) => widgetFunc(x) || x);
        }
        return layout;
    }
}
