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
import {Menu} from "@phosphor/widgets";
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

    private listeners: WeakMap<PerspectiveWidget, Function>;

    constructor(name: string, options?: DockPanel.IOptions) {
        super({renderer: new PerspectiveDockPanelRenderer(), spacing: 14, ...options});

        // this isn't quite right
        (this as any)._renderer.dock = this;

        this.commands = createCommands(this);
        this.addTabbarEventListeners();
        this.listeners = new WeakMap();
    }

    public duplicate(widget: PerspectiveWidget): void {
        const newWidget = new PerspectiveWidget(widget.name);
        newWidget.restore(widget.save()).then(() => {
            this.addWidget(newWidget, {mode: "split-right", ref: widget});
            newWidget.load(widget.table);
        });
    }

    public addWidget(widget: PerspectiveWidget, options: DockPanel.IAddOptions = {}): void {
        this.addWidgetEventListeners(widget);
        super.addWidget(widget, options);
    }

    public deserialize(layout: any): void {
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
        const contextMenu = new Menu({commands: this.commands});
        contextMenu.addItem({command: "perspective:duplicate", args: {widget}});

        // could move these 3 to perspective widget
        contextMenu.addItem({command: "perspective:export", args: {widget}});
        contextMenu.addItem({command: "perspective:copy", args: {widget}});
        contextMenu.addItem({command: "perspective:reset", args: {widget}});
        return contextMenu;
    }

    private showMenu(widget: PerspectiveWidget, event: MouseEvent) {
        // create menu in add widget instead??
        const menu = this.createMenu(widget);
        menu.open(event.clientX, event.clientY);
        event.preventDefault();
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
        if (this.listeners.has(widget)) {
            this.listeners.get(widget)();
        }
        const contextmenu = this.showMenu.bind(this, widget);
        const settings = (event: CustomEvent) => {
            widget.title.className = event.detail && "settings_open";
        };
        widget.viewer.addEventListener("contextmenu", contextmenu);
        widget.viewer.addEventListener("perspective-toggle-settings", settings);
        this.listeners.set(widget, () => {
            widget.viewer.removeEventListener("contextmenu", contextmenu);
            widget.viewer.removeEventListener("perspective-toggle-settings", settings);
        });
    }

    public createWidget = (config: any) => {
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
