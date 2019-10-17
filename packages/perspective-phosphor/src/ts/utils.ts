/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

/* defines */
export const MIME_TYPE = "application/psp+json";

export const PSP_CLASS = "PSPViewer";

export const PSP_CONTAINER_CLASS = "PSPContainer";

export const PSP_CONTAINER_CLASS_DARK = "PSPContainer-dark";

export const mapWidgets = (widgetFunc: any, layout: any) => {
    if (layout.main) {
        layout.main = mapWidgets(widgetFunc, layout.main);
    } else if (layout.children) {
        layout.children = layout.children.map((x: any) => mapWidgets(widgetFunc, x));
    } else if (layout.widgets) {
        layout.widgets = layout.widgets.map((x: any) => widgetFunc(x) || x);
    }
    return layout;
}
