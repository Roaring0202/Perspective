const path = require("path");
const common = require("@finos/perspective/src/config/common.config.js");

module.exports = Object.assign({}, common(), {
    entry: "./dist/esm/workspace.js",
    externals: [/^[a-z0-9@]/],
    output: {
        filename: "perspective-workspace.js",
        library: "perspective-workspace",
        libraryTarget: "commonjs2",
        path: path.resolve(__dirname, "../dist/cjs")
    }
});
