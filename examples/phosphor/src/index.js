/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import perspective from "@finos/perspective";
import {PerspectiveDockPanel, PerspectiveWidget} from "@finos/perspective-phosphor";
import "@finos/perspective-phosphor/src/theme/material/index.less";

import "@finos/perspective-viewer-d3fc";
import "@finos/perspective-viewer-hypergrid";

import {Widget} from "@phosphor/widgets";

import "./style/index.less";

const worker = perspective.shared_worker();
const req = fetch("./superstore.arrow");

window.addEventListener("load", async () => {
    const resp = await req;
    const buffer = await resp.arrayBuffer();
    const table = worker.table(buffer);

    const workspace = new PerspectiveDockPanel("example");
    Widget.attach(workspace, document.body);

    const perspective_viewer_1 = new PerspectiveWidget("One");
    const perspective_viewer_2 = new PerspectiveWidget("Two");

    perspective_viewer_1.dark = true;

    workspace.addWidget(perspective_viewer_1);
    workspace.addWidget(perspective_viewer_2, {mode: "split-right", ref: perspective_viewer_1});

    perspective_viewer_1.load(table);
    perspective_viewer_2.load(table);

    window.onresize = () => {
        workspace.update();
    };
    window.workspace = workspace;
});
