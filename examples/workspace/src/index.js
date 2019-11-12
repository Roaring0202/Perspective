/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import perspective from "@finos/perspective";
import "@finos/perspective-workspace";

import "@finos/perspective-viewer-hypergrid";
import "@finos/perspective-viewer-d3fc";

import "./index.less";

const datasource = async () => {
    const req = fetch("./superstore.arrow");
    const resp = await req;
    const buffer = await resp.arrayBuffer();
    const worker = perspective.shared_worker();
    return worker.table(buffer);
};

window.addEventListener("load", async () => {
    const workspace = document.createElement("perspective-workspace");
    document.body.append(workspace);
    workspace.addDatasource("test", datasource());

    const config = {
        detail: {
            main: {
                currentIndex: 0,
                type: "tab-area",
                widgets: [{datasource: "test"}, {datasource: "test"}]
            }
        }
    };
    workspace.restore(config);
});
