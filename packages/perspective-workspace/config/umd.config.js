const path = require("path");
const common = require("@finos/perspective/src/config/common.config.js");

module.exports = Object.assign({}, common(), {
    entry: "./dist/esm/workspace.js",
    output: {
        filename: "perspective-workspace.js",
        library: "perspective-workspace",
        libraryTarget: "umd",
        path: path.resolve(__dirname, "../dist/umd")
    }
});
