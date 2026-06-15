// Select and display file contents in the main editor
function selectFile(filename) {
    // Save current editor content to active file before switching, but ONLY if the file still exists in our registry and is not stdlib!
    if (activeFilename && !activeFilename.startsWith('stdlib/') && virtualFiles[activeFilename] !== undefined) {
        if (editorInstance) {
            const val = editorInstance.getValue();
            if (virtualFiles[activeFilename] !== val) {
                virtualFiles[activeFilename] = val;
                if (typeof virtualFileLastEdited !== 'undefined') {
                    virtualFileLastEdited[activeFilename] = Date.now();
                }
            }
        }
        if (eduEditorInstance) {
            const val = eduEditorInstance.getValue();
            if (virtualFiles[activeFilename] !== val) {
                virtualFiles[activeFilename] = val;
                if (typeof virtualFileLastEdited !== 'undefined') {
                    virtualFileLastEdited[activeFilename] = Date.now();
                }
            }
        }
    }

    activeFilename = filename;
    if (filename) {
        if (typeof virtualFileLastOpened !== 'undefined') {
            virtualFileLastOpened[filename] = Date.now();
        }
    }
    updateVirtualFilesUI();
    updateRunButtonState(); // Enable/disable Run Code button based on file extension

    const emptyState = document.getElementById('editor-empty-state');
    const eduEmptyState = document.getElementById('edu-editor-empty-state');
    const activeSpan = document.getElementById('editor-active-filename');
    const eduActiveSpan = document.getElementById('edu-active-filename');

    if (!filename) {
        // Show empty state overlay, hide editor contents
        if (emptyState) emptyState.classList.remove('hidden');
        if (eduEmptyState) eduEmptyState.classList.remove('hidden');
        if (activeSpan) {
            activeSpan.innerText = "No file open";
            activeSpan.className = "bg-slate-900/40 border border-slate-800/80 rounded-lg px-2.5 py-1 text-[11px] font-mono text-slate-500 select-none";
        }
        if (eduActiveSpan) {
            eduActiveSpan.innerText = "No file open";
            eduActiveSpan.className = "bg-slate-900/40 border border-slate-800/80 rounded-lg px-2.5 py-1 text-[11px] font-mono text-slate-500 select-none";
        }
        return;
    }

    // Show file contents
    if (emptyState) emptyState.classList.add('hidden');
    if (eduEmptyState) eduEmptyState.classList.add('hidden');
    if (activeSpan) {
        activeSpan.innerText = filename;
        activeSpan.className = "bg-gradient-to-r from-indigo-500/20 to-purple-500/10 border border-indigo-500/35 px-3 py-1 rounded-lg text-xs font-mono text-indigo-300 font-bold tracking-wide select-all shadow-[0_0_10px_rgba(99,102,241,0.1)]";
    }
    if (eduActiveSpan) {
        eduActiveSpan.innerText = filename;
        eduActiveSpan.className = "bg-gradient-to-r from-indigo-500/20 to-purple-500/10 border border-indigo-500/35 px-3 py-1 rounded-lg text-xs font-mono text-indigo-300 font-bold tracking-wide select-all shadow-[0_0_10px_rgba(99,102,241,0.1)]";
    }

    let fileContent = "";
    let isReadOnly = false;
    if (filename.startsWith('stdlib/')) {
        const libName = filename.substring(7);
        fileContent = stdlibFiles[libName] || "";
        isReadOnly = true;
    } else {
        fileContent = virtualFiles[filename] || "";
    }

    if (editorInstance) {
        editorInstance.setValue(fileContent);
        editorInstance.updateOptions({ readOnly: isReadOnly });
        // Set language syntax highlighting based on extension
        const isSad = filename.endsWith('.sad');
        const model = editorInstance.getModel();
        if (model) {
            monaco.editor.setModelLanguage(model, isSad ? 'sardine' : 'plaintext');
        }
        setTimeout(() => editorInstance.layout(), 50);
    }
    if (eduEditorInstance) {
        eduEditorInstance.setValue(fileContent);
        eduEditorInstance.updateOptions({ readOnly: isReadOnly });
        // Set language syntax highlighting based on extension
        const isSad = filename.endsWith('.sad');
        const model = eduEditorInstance.getModel();
        if (model) {
            monaco.editor.setModelLanguage(model, isSad ? 'sardine' : 'plaintext');
        }
        setTimeout(() => eduEditorInstance.layout(), 50);
    }
}

// Override the global stub
window.selectFile = selectFile;

// Load Monaco files via loader.js
require.config({ paths: { vs: 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.39.0/min/vs' } });
require(['vs/editor/editor.main'], function () {

    // Define custom editor color theme (Catppuccin Mocha)
    monaco.editor.defineTheme('sardine-dark', {
        base: 'vs-dark',
        inherit: true,
        rules: [
            { token: 'keyword', foreground: 'cba6f7', fontStyle: 'bold' }, // Mauve
            { token: 'type', foreground: 'f9e2af', fontStyle: 'italic' },  // Yellow
            { token: 'operator', foreground: '89dceb' },                  // Sky
            { token: 'identifier', foreground: 'cdd6f4' },                // Text
            { token: 'number', foreground: 'fab387' },                    // Peach
            { token: 'string', foreground: 'a6e3a1' },                    // Green
            { token: 'comment', foreground: '6c7086', fontStyle: 'italic' }, // Overlay1
            { token: 'delimiter', foreground: '94e2d5' }                  // Teal
        ],
        colors: {
            'editor.background': '#1e1e2e',                 // Mocha Base
            'editor.foreground': '#cdd6f4',                 // Mocha Text
            'editorLineNumber.foreground': '#585b70',       // Mocha Overlay0
            'editorLineNumber.activeForeground': '#cba6f7',  // Mocha Mauve
            'editor.lineHighlightBackground': '#313244',     // Mocha Surface0
            'editor.lineHighlightBorder': '#31324400',
            'editor.selectionBackground': '#585b7050',       // Selected text background
            'editor.inactiveSelectionBackground': '#45475a50',
            'editorGutter.background': '#1e1e2e',
            'scrollbarSlider.background': '#31324480',
            'scrollbarSlider.hoverBackground': '#585b70a0',
            'scrollbarSlider.activeBackground': '#cba6f780',
            'editorCursor.foreground': '#f5e0dc',            // Rosewater
            'editorWidget.background': '#181825',           // Mantle
            'editorWidget.border': '#313244',
        }
    });

    // Register Sardine syntax highlight profile
    monaco.languages.register({ id: 'sardine' });

    // Set language configuration for comments so Ctrl+/ toggles comments
    monaco.languages.setLanguageConfiguration('sardine', {
        comments: {
            lineComment: '#',
            blockComment: ['#*', '*#']
        }
    });

    monaco.languages.setMonarchTokensProvider('sardine', {
        keywords: [
            'model', 'init', 'method', 'attr', 'open', 'guarded', 'secret',
            'proceed', 'escape', 'yield', 'menu', 'choice', 'fallback',
            'risk', 'trap', 'clean', 'during', 'cycle', 'trace', 'summon',
            'from', 'as', 'when', 'orwhen', 'otherwise', 'and', 'or', 'not'
        ],
        typeKeywords: [
            'Integer', 'Float', 'String', 'List', 'Dictionary', 'RunTimeError', 'Error'
        ],
        operators: [
            '=', '+=', '-=', '*=', '/=', '%=', '//=', '**=', '&=', '^=', '|=',
            '<<=', '>>=', '+', '-', '*', '/', '%', '//', '**', '&', '|', '^',
            '~', '<<', '>>', '==', '!=', '<', '>', '<=', '>=', '->', '<-', '?', ':'
        ],
        symbols: /[=><!~?:&|+\-*\/\^%]+/,
        escapes: /\\(?:[abfnrtv\\"']|x[0-9A-Fa-f]{1,4}|u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})/,
        tokenizer: {
            root: [
                [/[a-zA-Z_]\w*/, {
                    cases: {
                        '@keywords': 'keyword',
                        '@typeKeywords': 'type',
                        '@default': 'identifier'
                    }
                }],
                { include: '@whitespace' },
                [/"([^"\\]|\\.)*$/, 'string.invalid'],
                [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],
                [/\d*\.\d+([eE][\-+]?\d+)?/, 'number.float'],
                [/\d+/, 'number'],
                [/[{}()\[\]]/, '@brackets'],
                [/@symbols/, {
                    cases: {
                        '@operators': 'operator',
                        '@default': ''
                    }
                }]
            ],
            string: [
                [/[^\\"]+/, 'string'],
                [/@escapes/, 'string.escape'],
                [/\\./, 'string.escape.invalid'],
                [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
            ],
            whitespace: [
                [/[ \t\r\n]+/, 'white'],
                [/#\*.*\*#/, 'comment'], // multiline comment
                [/#.*$/, 'comment'],     // singleline comment
            ]
        }
    });

    // Configure language configurations (braces, matching brackets)
    monaco.languages.setLanguageConfiguration('sardine', {
        brackets: [
            ['{', '}'],
            ['[', ']'],
            ['(', ')']
        ],
        autoClosingPairs: [
            { open: '{', close: '}' },
            { open: '[', close: ']' },
            { open: '(', close: ')' },
            { open: '"', close: '"' }
        ],
        surroundingPairs: [
            { open: '{', close: '}' },
            { open: '[', close: ']' },
            { open: '(', close: ')' },
            { open: '"', close: '"' }
        ]
    });

    // Create Monaco regular editor instance
    editorInstance = monaco.editor.create(document.getElementById('monaco-editor-container'), {
        value: "",
        language: 'sardine',
        theme: 'sardine-dark',
        fontSize: 14,
        lineHeight: 22,
        fontFamily: "'JetBrains Mono', Consolas, monospace",
        minimap: { enabled: true },
        automaticLayout: true,
        tabSize: 4
    });

    // Create Monaco educational editor instance
    eduEditorInstance = monaco.editor.create(document.getElementById('edu-monaco-editor-container'), {
        value: "",
        language: 'sardine',
        theme: 'sardine-dark',
        fontSize: 14,
        lineHeight: 22,
        fontFamily: "'JetBrains Mono', Consolas, monospace",
        minimap: { enabled: true },
        automaticLayout: true,
        tabSize: 4
    });

    // Auto-save changes back to our virtual files registry
    editorInstance.onDidChangeModelContent(() => {
        if (activeFilename && !activeFilename.startsWith('stdlib/')) {
            const val = editorInstance.getValue();
            if (virtualFiles[activeFilename] !== val) {
                virtualFiles[activeFilename] = val;
                if (typeof virtualFileLastEdited !== 'undefined') {
                    virtualFileLastEdited[activeFilename] = Date.now();
                }
            }
        }
    });

    eduEditorInstance.onDidChangeModelContent(() => {
        if (activeFilename && !activeFilename.startsWith('stdlib/')) {
            const val = eduEditorInstance.getValue();
            if (virtualFiles[activeFilename] !== val) {
                virtualFiles[activeFilename] = val;
                if (typeof virtualFileLastEdited !== 'undefined') {
                    virtualFileLastEdited[activeFilename] = Date.now();
                }
            }
        }
    });

    let cursorChangeTimeout = null;
    eduEditorInstance.onDidChangeCursorPosition(event => {
        if (window.isCursorSourcedFocus) return;
        if (cursorChangeTimeout) clearTimeout(cursorChangeTimeout);
        cursorChangeTimeout = setTimeout(() => {
            if (window.isCursorSourcedFocus) return;
            const pos = event.position;
            const targetLine = pos.lineNumber - 1;
            const targetCol = pos.column - 1;

            let foundNode = null;
            for (let i = dfsNodesQueue.length - 1; i >= 0; i--) {
                const node = dfsNodesQueue[i];
                if (node.pos_start && node.pos_end) {
                    const startsBefore = node.pos_start.line < targetLine || (node.pos_start.line === targetLine && node.pos_start.col <= targetCol);
                    const endsAfter = node.pos_end.line > targetLine || (node.pos_end.line === targetLine && node.pos_end.col >= targetCol);
                    if (startsBefore && endsAfter) {
                        foundNode = node;
                        break;
                    }
                }
            }

            if (foundNode) {
                const index = dfsNodesQueue.findIndex(n => n.id === foundNode.id);
                if (index !== -1 && index !== currentTraversalIndex) {
                    currentTraversalIndex = index;
                    highlightCurrentNode(true, 'editor');
                }
            }
        }, 200);
    });

    // Select main.sad by default
    selectFile('main.sad');

    // Let layout adjust itself initially
    for (let delay of [100, 300, 800, 1500, 3000]) {
        setTimeout(() => {
            if (typeof monaco !== 'undefined' && monaco.editor && monaco.editor.remeasureFonts) {
                monaco.editor.remeasureFonts();
            }
            if (editorInstance) editorInstance.layout();
            if (eduEditorInstance) eduEditorInstance.layout();
        }, delay);
    }

    // Redraw Monaco once fonts are fully loaded to prevent cursor alignment offset bugs
    if (document.fonts) {
        document.fonts.ready.then(function () {
            if (typeof monaco !== 'undefined' && monaco.editor && monaco.editor.remeasureFonts) {
                monaco.editor.remeasureFonts();
            }
            if (editorInstance) {
                editorInstance.layout();
            }
            if (eduEditorInstance) {
                eduEditorInstance.layout();
            }
        });
    }

    // Hide loading animation screen since environment is ready
    hideLoadingOverlay();
});
