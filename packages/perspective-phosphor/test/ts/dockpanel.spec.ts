import {PerspectiveDockPanel} from "../../src/ts/dockpanel";
import {PerspectiveWidget} from "../../src/ts/widget";

describe("dockpanel", () => {
    test("serialise returns a correct widget state", () => {
        const dockpanel = new PerspectiveDockPanel("name");

        const widget = new PerspectiveWidget("One", {plugin_config: {columns: ["A"]}});
        dockpanel.addWidget(widget);

        const expectedConfig = { 
            main: { 
                type: 'tab-area', 
                widgets: [ {columns: ["A"]} ], 
                currentIndex: 0 
            } 
        }
        expect(dockpanel.serialize()).toStrictEqual(expectedConfig)
    });

    test("deserialise restore correct dockpanel state", () => {
        const dockpanel = new PerspectiveDockPanel("name");
        const config = { 
            main: { 
                type: 'tab-area', 
                widgets: [ {columns: ["A"]} ], 
                currentIndex: 0 
            } 
        }

        const expectedConfig = JSON.parse(JSON.stringify(config))

        dockpanel.deserialize(config);
        expect(dockpanel.serialize()).toStrictEqual(expectedConfig)
    });
});
