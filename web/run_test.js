const fs = require('fs');
const path = require('path');

if (process.argv.length < 3) {
    console.error("Usage: node run_test.js <test_file.sad> [input_file.in]");
    process.exit(1);
}

const testFile = process.argv[2];
const inputFile = process.argv[3];

const testCode = fs.readFileSync(testFile, 'utf8').replace(/\r/g, '');
let stdinData = '';
if (inputFile && fs.existsSync(inputFile)) {
    stdinData = fs.readFileSync(inputFile, 'utf8');
}

let stdinIndex = 0;

global.Module = {
    print: function(text) {
        console.log(text);
    },
    printErr: function(text) {
        console.error(text);
    },
    preRun: [function() {
        function stdinCallback() {
            if (stdinIndex < stdinData.length) {
                return stdinData.charCodeAt(stdinIndex++);
            }
            return null; // EOF
        }
        if (typeof FS !== 'undefined') {
            FS.init(stdinCallback, null, null);
        } else if (global.Module.FS) {
            global.Module.FS.init(stdinCallback, null, null);
        }
    }],
    onRuntimeInitialized: function() {
        try {
            global.Module.ccall("run_interpreter", null, ["string"], [testCode]);
        } catch (e) {
            console.error("Runtime error:", e);
            process.exit(1);
        }
        process.exit(0);
    }
};

const vm = require('vm');
global.__dirname = __dirname;
global.__filename = __filename;
global.require = require;
const interpreterCode = fs.readFileSync(path.join(__dirname, 'interpreter.js'), 'utf8');
vm.runInThisContext(interpreterCode);


