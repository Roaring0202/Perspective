/******************************************************************************
 *
 * Copyright (c) 2017, the Perspective Authors.
 *
 * This file is part of the Perspective library, distributed under the terms of
 * the Apache License 2.0.  The full license can be found in the LICENSE file.
 *
 */

const {getarg, execute, docker} = require("./script_utils.js");

let flags = " -DPSP_WASM_BUILD=OFF -DPSP_CPP_BUILD=ON -DPSP_CPP_BUILD_TESTS=ON -DPSP_CPP_BUILD_STRICT=OFF";

try {
    execute`mkdir -p cpp/perspective/cppbuild`;

    let cmd;

    if (getarg("--ci")) {
        // Install dependencies
        execute`
            export CUR_DIR=\`pwd\`;
            wget "https://github.com/google/flatbuffers/archive/v1.11.0.tar.gz" -O /tmp/flatbuffers.tar.gz;
            cd /tmp;
            tar xfa /tmp/flatbuffers.tar.gz;
            cd flatbuffers-1.11.0;
            mkdir build;
            cd build;
            cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DFLATBUFFERS_BUILD_SHAREDLIB=ON;
            sudo make install;
            sudo cp flatc /usr/bin;
            sudo rm -rf /tmp/flatbuffers.tar.gz /tmp/flatbuffers-1.11.0;
            wget "https://cmake.org/files/v3.15/cmake-3.15.5-Linux-x86_64.tar.gz" -O /tmp/cmake.tar.gz;
            cd /tmp;
            tar xfz cmake.tar.gz;
            sudo cp -r cmake-3.15.5-Linux-x86_64/* /usr;
            sudo rm -rf /usr/local/cmake-3.12.4 /tmp/cmake.tar.gz;
            /tmp/cmake-3.15.5-Linux-x86_64;
            cd $CUR_DIR;
            export CXX="g++-4.9" CC="gcc-4.9";
        `;
    }

    if (process.env.PSP_DOCKER) {
        cmd = " ";
    } else {
        cmd = "cd cpp/perspective/cppbuild && ";
    }

    cmd += ` cmake ../ ${flags}`;

    if (process.env.PSP_DEBUG) {
        cmd += ` -DCMAKE_BUILD_TYPE=debug`;
    }

    if (process.env.PSP_DOCKER) {
        execute(docker("python") + cmd);
        execute(docker("python") + " make -j${PSP_CPU_COUNT-8}");
    } else {
        execute(cmd);
        execute("cd cpp/perspective/cppbuild && make -j${PSP_CPU_COUNT-8}");
    }
} catch (e) {
    console.log(e.message);
    process.exit(1);
}
