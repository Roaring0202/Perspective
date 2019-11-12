const path = require("path");
const common = require("@finos/perspective/src/config/common.config.js");

const commonConfig = common();
commonConfig.module.rules.push({
    test: /\.js$/,
    use: [{loader: "babel-loader"}]
});

module.exports = Object.assign({}, commonConfig, {
    entry: "./src/js/workspace.js",
    externals: [/^[a-z0-9@]/],

    output: {
        filename: "perspective-workspace.js",
        library: "perspective-workspace",
        libraryTarget: "commonjs2",
        path: path.resolve(__dirname, "../dist/cjs")
    }
});
