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
            const fs_api = global.Module.FS || (typeof FS !== 'undefined' ? FS : null);
            if (fs_api) {
                const repoRoot = path.join(__dirname, '..');
                
                // Recursively recreate the host tests directory structure in MEMFS
                function recreateDirs(hostDirPath) {
                    const relativePath = path.relative(repoRoot, hostDirPath).replace(/\\/g, '/');
                    if (relativePath && relativePath !== '.') {
                        try {
                            fs_api.mkdir('/' + relativePath);
                        } catch (err) {
                            // Directory may already exist
                        }
                    }
                    const items = fs.readdirSync(hostDirPath);
                    for (const item of items) {
                        const fullPath = path.join(hostDirPath, item);
                        if (fs.statSync(fullPath).isDirectory()) {
                            recreateDirs(fullPath);
                        }
                    }
                }
                const testsDir = path.join(repoRoot, 'tests');
                if (fs.existsSync(testsDir)) {
                    recreateDirs(testsDir);
                }

                // Copy sibling files from host filesystem to MEMFS
                const hostDir = path.dirname(testFile);
                if (fs.existsSync(hostDir)) {
                    const siblings = fs.readdirSync(hostDir);
                    const relativeTestPath = path.relative(repoRoot, testFile);
                    const dirName = path.dirname(relativeTestPath).replace(/\\/g, '/');
                    for (const sibling of siblings) {
                        const hostFilePath = path.join(hostDir, sibling);
                        const stat = fs.statSync(hostFilePath);
                        if (stat.isFile() && sibling !== path.basename(testFile)) {
                            const fileData = fs.readFileSync(hostFilePath);
                            const memfsPath = (dirName && dirName !== '.') ? `${dirName}/${sibling}` : sibling;
                            try {
                                fs_api.writeFile(memfsPath, fileData);
                            } catch (err) {
                                // Ignore write errors
                            }
                        }
                    }
                }
            }
            
            const relativeTestPath = path.relative(path.join(__dirname, '..'), testFile).replace(/\\/g, '/');
            global.Module.ccall("run_interpreter", null, ["string", "string", "number", "number"], [testCode, relativeTestPath, 0, 0]);
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


