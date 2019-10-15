/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import {CommandRegistry} from "@phosphor/commands";

export const createCommands = (dock: any) => {
    const commands = new CommandRegistry();
    commands.addCommand("perspective:duplicate", {
        execute: ({widget}) => dock.duplicate(widget),
        iconClass: "p-MenuItem-duplicate",
        label: "Duplicate",
        mnemonic: 0
    });

    commands.addCommand("perspective:export", {
        execute: (args: any) => args.widget.pspNode.download(),
        iconClass: "p-MenuItem-export",
        label: "Export CSV",
        mnemonic: 0
    });

    commands.addCommand("perspective:copy", {
        execute: (args: any) => args.widget.pspNode.copy(),
        iconClass: "p-MenuItem-copy",
        label: "Copy To Clipboard",
        mnemonic: 0
    });

    commands.addCommand("perspective:reset", {
        execute: (args: any) => args.widget.pspNode.reset(),
        iconClass: "p-MenuItem-reset",
        label: "Reset",
        mnemonic: 0
    });

    return commands;
};
