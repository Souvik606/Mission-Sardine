const Module = require('./interpreter.js');
Module.onRuntimeInitialized = function() {
    console.log("WASM loaded successfully!");
    try {
        Module.ccall("run_interpreter", null, ["string"], ["show(\"Hello from Node WASM test!\")\n"]);
        console.log("WASM run_interpreter completed.");
    } catch(e) {
        console.error("WASM run failed:", e);
    }
};
