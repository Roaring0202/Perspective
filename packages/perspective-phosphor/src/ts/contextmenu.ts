/******************************************************************************
 *
 * Copyright (c) 2018, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

import {    CommandRegistry  } from '@phosphor/commands';
  
export const createCommands = (dock: any) => {
  const commands = new CommandRegistry();
  commands.addCommand('perspective:duplicate', {
    label: 'Duplicate',
    mnemonic: 0,
    iconClass: 'p-MenuItem-duplicate',
    execute: ({widget}) => dock.duplicate(widget)
  });

  commands.addCommand("perspective:export", {
    label: "Export CSV",
    mnemonic: 0,
    iconClass: "p-MenuItem-export",
    execute: (args: any) => args.widget.pspNode.download()
  });

  commands.addCommand("perspective:copy", {
      label: "Copy To Clipboard",
      mnemonic: 0,
      iconClass: "p-MenuItem-copy",
      execute: (args: any) => args.widget.pspNode.copy()
  });

  commands.addCommand("perspective:reset", {
      label: "Reset",
      mnemonic: 0,
      iconClass: "p-MenuItem-reset",
      execute: (args: any) => args.widget.pspNode.reset()
  });

  return commands
}