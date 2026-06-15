// Initialize background WASM worker and SharedArrayBuffer
function initWasmWorker() {
    if (worker) {
        worker.terminate();
    }

    // Check SharedArrayBuffer capability
    if (typeof SharedArrayBuffer !== 'undefined') {
        sharedBuffer = new SharedArrayBuffer(1024);
        sharedInt32 = new Int32Array(sharedBuffer);
        sharedUint8 = new Uint8Array(sharedBuffer, 8); // Data block offset 8
    } else {
        console.error("SharedArrayBuffer is not available. Cross-origin isolation header failed to load.");
        writeToTerminal("[Warning] SharedArrayBuffer is unavailable. Inputs with listen() will not function.", true);
    }

    worker = new Worker('worker.js?v=' + Date.now());

    if (sharedBuffer) {
        worker.postMessage({ type: 'init', sharedBuffer: sharedBuffer });
    }

    worker.onmessage = function (event) {
        const { type, data, files } = event.data;

        if (type === 'ready') {
            stdlibFiles = event.data.stdlib || {};
            setWasmReadyState(true);
            updateVirtualFilesUI();
        }
        else if (type === 'stdout') {
            writeToTerminal(data);
        }
        else if (type === 'stderr') {
            writeToTerminal(data, true);
        }
        else if (type === 'educational_json') {
            if (data) {
                if (data.tokens) {
                    renderTokensTable(data.tokens);
                }
                if (data.ast) {
                    renderAstTree(data.ast);
                    if (data.trace && data.trace.length > 0) {
                        initExecutionStepper(data.trace);
                    } else {
                        stopExecutionStepper();
                    }
                } else if (data.error) {
                    renderAstError(data.error);
                    stopExecutionStepper();
                }
            }
        }
        else if (type === 'request_input') {
            showInputPrompt();
        }
        else if (type === 'done') {
            setRunningState(false);
            if (files) {
                // Merge worker files back into our registry
                for (let filename in files) {
                    virtualFiles[filename] = files[filename];
                }

                updateVirtualFilesUI();

                // Update Monaco value if current file was changed by interpreter run
                if (activeFilename && virtualFiles[activeFilename] !== undefined) {
                    if (editorInstance && editorInstance.getValue() !== virtualFiles[activeFilename]) {
                        editorInstance.setValue(virtualFiles[activeFilename]);
                    }
                    if (eduEditorInstance && eduEditorInstance.getValue() !== virtualFiles[activeFilename]) {
                        eduEditorInstance.setValue(virtualFiles[activeFilename]);
                    }
                }
            }
        }
    };

    worker.postMessage({ type: 'load' });
}

function updateRunButtonState() {
    const runBtn = document.getElementById('editor-btn-run');
    const eduRunBtn = document.getElementById('edu-btn-run');
    const isSad = activeFilename && activeFilename.endsWith('.sad') && !activeFilename.startsWith('stdlib/');

    [runBtn, eduRunBtn].forEach(btn => {
        if (!btn) return;
        if (wasmReady && !isRunning && isSad) {
            btn.disabled = false;
            btn.className = "flex items-center space-x-1.5 btn-run-active px-6 py-2.5 rounded-xl text-xs font-black tracking-wider transition-all cursor-pointer";
        } else {
            btn.disabled = true;
            if (isRunning) {
                btn.className = "flex items-center space-x-1.5 bg-slate-900/60 border border-slate-800/80 text-slate-650 text-slate-600 px-6 py-2.5 rounded-xl text-xs font-bold cursor-not-allowed";
            } else {
                btn.className = "flex items-center space-x-1.5 bg-slate-900/40 border border-slate-800/40 text-slate-660 text-slate-600 px-6 py-2.5 rounded-xl text-xs font-bold cursor-not-allowed transition-all";
            }
        }
    });
}

function setWasmReadyState(isReady) {
    wasmReady = isReady;
    updateRunButtonState();
}

function setRunningState(running) {
    isRunning = running;
    updateRunButtonState();

    const stopBtn = document.getElementById('editor-btn-stop');
    const eduStopBtn = document.getElementById('edu-btn-stop');

    [stopBtn, eduStopBtn].forEach(btn => {
        if (!btn) return;
        if (isRunning) {
            btn.disabled = false;
            btn.className = "flex items-center space-x-1.5 bg-gradient-to-r from-rose-600 to-pink-650 to-pink-600 hover:from-rose-500 hover:to-pink-500 border border-rose-500/30 hover:border-rose-400 text-white px-6 py-2.5 rounded-xl text-xs font-black tracking-wider transition-all shadow-[0_0_15px_rgba(239,68,68,0.25)] hover:scale-[1.03] active:scale-95 cursor-pointer";
        } else {
            btn.disabled = true;
            btn.className = "flex items-center space-x-1.5 bg-rose-950/10 border border-rose-900/10 text-rose-500/30 px-6 py-2.5 rounded-xl text-xs font-bold cursor-not-allowed transition-all";
        }
    });
}

function writeToTerminal(message, isError = false) {
    const consoleEl = document.getElementById('terminal-console');
    const eduConsoleEl = document.getElementById('edu-terminal-console');

    [consoleEl, eduConsoleEl].forEach(el => {
        if (!el) return;
        const newLine = document.createElement('div');

        if (isError) {
            newLine.className = "text-rose-400 font-semibold font-mono";
        } else if (message.startsWith("//") || message.startsWith("[System]")) {
            newLine.className = "text-slate-500 font-mono";
        } else {
            newLine.className = "text-slate-200 font-mono";
        }

        newLine.innerText = message;
        el.appendChild(newLine);
        el.scrollTop = el.scrollHeight;
    });
}

function clearTerminal() {
    const consoleEl = document.getElementById('terminal-console');
    const eduConsoleEl = document.getElementById('edu-terminal-console');
    if (consoleEl) consoleEl.innerHTML = "";
    if (eduConsoleEl) eduConsoleEl.innerHTML = "";

    // Reset AST visualizer state
    latestAst = null;
    dfsNodesQueue = [];
    currentTraversalIndex = -1;

    // Clear Monaco range highlights
    if (eduEditorInstance && window.eduEditorDecorations) {
        window.eduEditorDecorations = eduEditorInstance.deltaDecorations(window.eduEditorDecorations, []);
    }
    if (typeof stopExecutionStepper === 'function') {
        stopExecutionStepper();
    }

    const tokensTbody = document.getElementById('edu-tokens-table-body');
    const astContainer = document.getElementById('edu-ast-tree-container');
    if (tokensTbody) {
        tokensTbody.innerHTML = '<tr><td colspan="2" class="p-4 text-center text-slate-500 italic">// Run code in Educational Mode to populate.</td></tr>';
    }
    if (astContainer) {
        astContainer.innerHTML = '<div class="text-slate-500 italic p-4 text-center">// Run code in Educational Mode to populate.</div>';
    }
}

function runCode(educationalMode = false) {
    const currentEditor = educationalMode ? eduEditorInstance : editorInstance;
    if (!currentEditor) return;

    // Save active editor contents to virtualFiles registry
    if (activeFilename && !activeFilename.startsWith('stdlib/')) {
        virtualFiles[activeFilename] = currentEditor.getValue();
    }

    // Clear terminal automatically if in educationalMode, else check toggle
    if (educationalMode) {
        clearTerminal();
        if (typeof stopExecutionStepper === 'function') {
            stopExecutionStepper();
        }
        if (typeof collapseSidebar === 'function') collapseSidebar(true);
        if (typeof collapseTerminal === 'function') collapseTerminal(true);

        const tokensTbody = document.getElementById('edu-tokens-table-body');
        const astContainer = document.getElementById('edu-ast-tree-container');
        if (tokensTbody) {
            tokensTbody.innerHTML = '<tr><td colspan="2" class="p-4 text-center text-slate-500 italic animate-pulse">Waiting for tokens...</td></tr>';
        }
        if (astContainer) {
            astContainer.innerHTML = '<div class="text-slate-550 italic p-4 text-center animate-pulse text-slate-550">// Waiting for AST...</div>';
        }
    } else {
        if (document.getElementById('editor-toggle-clear-terminal').checked) {
            clearTerminal();
        }
    }

    // Run active file if it is a .sad file, otherwise fall back to main.sad
    let code = "";
    if (activeFilename && activeFilename.endsWith('.sad')) {
        code = virtualFiles[activeFilename];
    } else if (virtualFiles['main.sad'] !== undefined) {
        code = virtualFiles['main.sad'];
        writeToTerminal("// Running main.sad...");
    } else {
        writeToTerminal("[Error] No runnable .sad file available.", true);
        setRunningState(false);
        return;
    }

    const unbounded = educationalMode ? false : document.getElementById('editor-toggle-unbounded').checked;

    writeToTerminal("// ------ Execution Start ------");
    setRunningState(true);

    if (worker) {
        worker.postMessage({
            type: 'run',
            code: code,
            unbounded: unbounded,
            educational: educationalMode,
            files: virtualFiles // Pass all files to the worker MEMFS
        });
    } else {
        writeToTerminal("[Error] Wasm background thread unavailable.", true);
        setRunningState(false);
    }
}

function terminateExecution() {
    writeToTerminal("// ------ Execution Interrupted by User ------", true);
    initWasmWorker(); // Spawns a clean background thread
    setRunningState(false);

    // Hide terminal input prompts if active
    const inputRow = document.getElementById('terminal-input-row');
    const eduInputRow = document.getElementById('edu-terminal-input-row');
    if (inputRow) inputRow.classList.add('hidden');
    if (eduInputRow) eduInputRow.classList.add('hidden');
}

// Interactive terminal input handlers
function showInputPrompt() {
    const inputRow = document.getElementById('terminal-input-row');
    const eduInputRow = document.getElementById('edu-terminal-input-row');
    if (inputRow) inputRow.classList.remove('hidden');
    if (eduInputRow) eduInputRow.classList.remove('hidden');

    const inputEl = document.getElementById('terminal-stdin');
    const eduInputEl = document.getElementById('edu-terminal-stdin');
    if (inputEl) {
        inputEl.value = "";
        inputEl.focus();
    }
    if (eduInputEl) {
        eduInputEl.value = "";
        eduInputEl.focus();
    }
}

function submitTerminalStdin(isEdu = false) {
    const inputEl = document.getElementById(isEdu ? 'edu-terminal-stdin' : 'terminal-stdin');
    const val = inputEl.value;

    // Print the prompt symbol and user response directly into the terminal
    writeToTerminal("> " + val);

    const encoder = new TextEncoder();
    const encoded = encoder.encode(val + "\n");
    sharedUint8.set(encoded);

    sharedInt32[1] = encoded.length;

    Atomics.store(sharedInt32, 0, 1);
    Atomics.notify(sharedInt32, 0, 1);

    const inputRow = document.getElementById('terminal-input-row');
    const eduInputRow = document.getElementById('edu-terminal-input-row');
    if (inputRow) inputRow.classList.add('hidden');
    if (eduInputRow) eduInputRow.classList.add('hidden');
}
