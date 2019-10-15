/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import {Widget} from "@phosphor/widgets";
import "./style/index.less";
import {PerspectiveDockPanel, PerspectiveWidget} from "@finos/perspective-phosphor";
import "@finos/perspective-phosphor/src/theme/material/index.less";
import perspective from "@finos/perspective";

window.addEventListener("load", () => {
    const worker = perspective.shared_worker();

    const table = fetch("./superstore.arrow").then(data => data.arrayBuffer().then(table => worker.table(table)));
    table.then(x => {
        let main = new PerspectiveDockPanel("example");
        Widget.attach(main, document.body);

        const widget = new PerspectiveWidget("One");
        const widget2 = new PerspectiveWidget("Two");

        main.addWidget(widget);
        main.addWidget(widget2, {mode: "split-right", ref: widget});

        widget.load(x);
        widget2.load(x);

        window.onresize = () => {
            main.update();
        };
    });
});
