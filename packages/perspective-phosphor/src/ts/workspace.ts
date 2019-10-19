import {SplitPanel, Widget, DockLayout} from "@phosphor/widgets";
import {PerspectiveDockPanel, ContextMenuArgs} from "./dockpanel";
import {PerspectiveWidget} from "./widget";
import {mapWidgets} from "./utils";
import {uniqBy} from "lodash";
import {Menu} from "@phosphor/widgets";
import {createCommands} from "./contextmenu";
import {CommandRegistry} from "@phosphor/commands";

export interface PerspectiveWorkspaceOptions {
    container?: HTMLElement;
}

export class PerspectiveWorkspace {
    private dockpanel: PerspectiveDockPanel;
    private driver: SplitPanel;
    private main: SplitPanel;
    private commands: CommandRegistry;

    constructor({container}: PerspectiveWorkspaceOptions = {}) {
        this.dockpanel = new PerspectiveDockPanel("main", {enableContextMenu: false});
        this.driver = new SplitPanel({orientation: "vertical"});
        this.main = new SplitPanel({orientation: "horizontal"});

        this.driver.addClass("p-Master");
        this.main.addWidget(this.dockpanel);
        this.commands = this.createCommands();

        this.dockpanel.onContextMenu.connect(this.showContextMenu.bind(this));

        Widget.attach(this.main, container || document.body);
    }

    addWidget(widget: PerspectiveWidget, options: DockLayout.IAddOptions): void {
        this.dockpanel.addWidget(widget, options);
    }

    update(): void {
        this.main.update();
    }

    private createContextMenu(widget: any): Menu {
        const contextMenu = new Menu({commands: this.commands});

        contextMenu.addItem({command: "workspace:broadcast", args: {widget}});
        if (widget.parent === this.dockpanel) {
            contextMenu.addItem({command: "perspective:duplicate", args: {widget}});
        }

        contextMenu.addItem({command: "perspective:export", args: {widget}});
        contextMenu.addItem({command: "perspective:copy", args: {widget}});
        contextMenu.addItem({command: "perspective:reset", args: {widget}});
        return contextMenu;
    }

    private showContextMenu(sender: PerspectiveDockPanel, args: ContextMenuArgs): void {
        const {widget, event} = args;
        const menu = this.createContextMenu(widget);
        menu.open(event.clientX, event.clientY);
        event.preventDefault();
        event.stopPropagation();
    }

    private filterWidget(filters: string[][]): void {
        mapWidgets(async (widget: PerspectiveWidget): Promise<void> => {
            const availableColumns = Object.keys(await (widget.table as any).schema());
            const currentFilters = widget.save().filters || [];
            const columnAvailable = (filter: string[]): boolean => filter[0] && availableColumns.includes(filter[0]);
            const validFilters = filters.filter(columnAvailable);

            validFilters.push(...currentFilters);
            const newFilters = uniqBy(validFilters, (item: string[]) => item[0]);
            widget.restore({filters: newFilters});
        }, this.dockpanel.saveLayout());
    }

    private makeMaster(widget: PerspectiveWidget): void {
        const table = widget.table;
        widget.close();

        const newWidget = this.dockpanel.createWidget(widget.title.label, {dark: true, ...widget.save()});
        newWidget.title.closable = true;

        if (this.driver.widgets.length === 0) {
            this.dockpanel.close();
            this.main.addWidget(this.driver);
            this.main.addWidget(this.dockpanel);
            this.main.setRelativeSizes([1, 3]);
        }

        this.driver.addWidget(newWidget);
        newWidget.load(table);
        newWidget.node.addEventListener("perspective-click", (event: CustomEvent) => {
            this.filterWidget([...event.detail.config.filters]);
        });
    }

    private makeDetail(widget: PerspectiveWidget): void {
        const table = widget.table;
        widget.close();

        const newWidget = new PerspectiveWidget(widget.title.label, widget.save());
        newWidget.title.closable = true;

        this.dockpanel.addWidget(newWidget, {mode: "split-right"});
        newWidget.load(table);

        if (this.driver.widgets.length === 0) {
            this.dockpanel.close();
            this.driver.close();
            this.main.addWidget(this.dockpanel);
        }
    }

    private toggleMasterDetail(widget: PerspectiveWidget): void {
        if (widget.parent === this.dockpanel) {
            this.makeMaster(widget);
        } else {
            this.makeDetail(widget);
        }
    }

    private createCommands(): CommandRegistry {
        const commands = createCommands(this.dockpanel) as CommandRegistry;
        commands.addCommand("workspace:broadcast", {
            execute: args => this.toggleMasterDetail((args as any).widget),
            iconClass: args => ((args as any).widget.parent === this.dockpanel ? "p-MenuItem-master" : "p-MenuItem-detail"),
            label: args => ((args as any).widget.parent === this.dockpanel ? "Master" : "Detail"),
            mnemonic: 0
        });
        return commands;
    }
}
