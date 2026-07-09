// Visual Stepper state and helper functions
window.executionTrace = [];
window.currentExecutionStep = -1;
window.stepperInterval = null;
window.stepperSpeed = 600;
window.previousStepVars = {};

function initExecutionStepper(trace) {
    // Stop any running play interval
    if (window.stepperInterval) {
        clearInterval(window.stepperInterval);
        window.stepperInterval = null;
    }
    
    window.executionTrace = trace;
    window.currentExecutionStep = -1; // Start at -1 (inactive state) to keep AST clean initially
    window.previousStepVars = {};
    
    // Switch to AST tab to visualize
    switchEduTab('ast');
    
    // Show stepper panel next to editor and expand sidebar width
    const sidebar = document.getElementById('edu-info-sidebar');
    const stepperPanel = document.getElementById('edu-stepper-panel');
    if (sidebar && stepperPanel) {
        if (stepperPanel.classList.contains('hidden')) {
            const currentWidth = sidebar.offsetWidth || 384;
            sidebar.style.width = `${currentWidth + 350}px`;
            stepperPanel.classList.remove('hidden');
        }
        if (eduEditorInstance) {
            setTimeout(() => eduEditorInstance.layout(), 50);
        }
    }
    
    // Reset play button state
    const playIcon = document.getElementById('btn-step-play-icon');
    if (playIcon) {
        playIcon.setAttribute('data-lucide', 'play');
    }
    const playBtn = document.getElementById('btn-step-play');
    if (playBtn) {
        playBtn.className = "p-1.5 rounded bg-indigo-600 hover:bg-indigo-500 text-white transition-all cursor-pointer flex items-center justify-center shadow-[0_0_10px_rgba(99,102,241,0.25)] active:scale-95";
    }
    lucide.createIcons();
    
    // Reset status counter label
    const statusLabel = document.getElementById('edu-stepper-status');
    if (statusLabel) {
        statusLabel.innerText = `Step 0 / ${window.executionTrace.length}`;
    }
    
    // Reset active node label
    const activeNodeLabel = document.getElementById('edu-stepper-active-node');
    if (activeNodeLabel) {
        activeNodeLabel.innerText = "None";
    }
    
    // Clear scopes panel
    const scopesContainer = document.getElementById('edu-stepper-scopes');
    if (scopesContainer) {
        scopesContainer.innerHTML = '<div class="text-slate-500 italic text-[11px] text-center py-4">// Press Step or Play to start code execution trace.</div>';
    }
    
    // Clear Monaco execution decorations
    if (eduEditorInstance && window.eduExecutionDecorations) {
        window.eduExecutionDecorations = eduEditorInstance.deltaDecorations(window.eduExecutionDecorations, []);
    }
    
    // Re-apply standard layout / reset traversal highlights (Exploration Mode)
    resetTraversal();
}

function stopExecutionStepper() {
    if (window.stepperInterval) {
        clearInterval(window.stepperInterval);
        window.stepperInterval = null;
    }
    window.executionTrace = [];
    window.currentExecutionStep = -1;
    window.previousStepVars = {};
    
    const sidebar = document.getElementById('edu-info-sidebar');
    const stepperPanel = document.getElementById('edu-stepper-panel');
    if (sidebar && stepperPanel) {
        if (!stepperPanel.classList.contains('hidden')) {
            const currentWidth = sidebar.offsetWidth || 704;
            const newWidth = Math.max(200, currentWidth - 350);
            sidebar.style.width = `${newWidth}px`;
            stepperPanel.classList.add('hidden');
        }
        if (eduEditorInstance) {
            setTimeout(() => eduEditorInstance.layout(), 50);
        }
    }
    
    // Clear Monaco execution decorations
    if (eduEditorInstance && window.eduExecutionDecorations) {
        window.eduExecutionDecorations = eduEditorInstance.deltaDecorations(window.eduExecutionDecorations, []);
    }
    
    // Re-apply standard layout
    resetTraversal();
}

function toggleExecutionPlay() {
    const playIcon = document.getElementById('btn-step-play-icon');
    const playBtn = document.getElementById('btn-step-play');
    
    if (window.stepperInterval) {
        // Pause playback
        clearInterval(window.stepperInterval);
        window.stepperInterval = null;
        if (playIcon) {
            playIcon.setAttribute('data-lucide', 'play');
            lucide.createIcons();
        }
        if (playBtn) playBtn.className = "p-1.5 rounded bg-indigo-600 hover:bg-indigo-500 text-white transition-all cursor-pointer flex items-center justify-center shadow-[0_0_10px_rgba(99,102,241,0.25)] active:scale-95";
    } else {
        // Start playback
        if (window.currentExecutionStep >= window.executionTrace.length - 1) {
            window.currentExecutionStep = -1; // wrap around
            window.previousStepVars = {};
        }
        
        window.stepperInterval = setInterval(() => {
            if (window.currentExecutionStep < window.executionTrace.length - 1) {
                nextExecutionStep();
            } else {
                toggleExecutionPlay(); // pause on end
            }
        }, window.stepperSpeed);
        
        if (playIcon) {
            playIcon.setAttribute('data-lucide', 'pause');
            lucide.createIcons();
        }
        if (playBtn) playBtn.className = "p-1.5 rounded bg-amber-600 hover:bg-amber-550 text-white transition-all cursor-pointer flex items-center justify-center shadow-[0_0_10px_rgba(245,158,11,0.25)] active:scale-95 hover:bg-amber-500";
    }
}

function updateStepperSpeed(val) {
    window.stepperSpeed = parseInt(val);
    const speedValLabel = document.getElementById('edu-stepper-speed-val');
    if (speedValLabel) {
        speedValLabel.innerText = `${val}ms`;
    }
    
    if (window.stepperInterval) {
        clearInterval(window.stepperInterval);
        window.stepperInterval = setInterval(() => {
            if (window.currentExecutionStep < window.executionTrace.length - 1) {
                nextExecutionStep();
            } else {
                toggleExecutionPlay();
            }
        }, window.stepperSpeed);
    }
}

window.resetExecutionStepper = function() {
    if (window.stepperInterval) {
        clearInterval(window.stepperInterval);
        window.stepperInterval = null;
    }
    window.currentExecutionStep = -1;
    window.previousStepVars = {};
    
    // Clear Monaco execution decorations
    if (eduEditorInstance && window.eduExecutionDecorations) {
        window.eduExecutionDecorations = eduEditorInstance.deltaDecorations(window.eduExecutionDecorations, []);
    }
    
    // Reset play button state
    const playIcon = document.getElementById('btn-step-play-icon');
    if (playIcon) {
        playIcon.setAttribute('data-lucide', 'play');
    }
    const playBtn = document.getElementById('btn-step-play');
    if (playBtn) {
        playBtn.className = "p-1.5 rounded bg-indigo-600 hover:bg-indigo-500 text-white transition-all cursor-pointer flex items-center justify-center shadow-[0_0_10px_rgba(99,102,241,0.25)] active:scale-95";
    }
    lucide.createIcons();
    
    // Reset status counter label
    const statusLabel = document.getElementById('edu-stepper-status');
    if (statusLabel) {
        statusLabel.innerText = `Step 0 / ${window.executionTrace.length}`;
    }
    
    // Reset active node label
    const activeNodeLabel = document.getElementById('edu-stepper-active-node');
    if (activeNodeLabel) {
        activeNodeLabel.innerText = "None";
    }
    
    // Clear scopes panel
    const scopesContainer = document.getElementById('edu-stepper-scopes');
    if (scopesContainer) {
        scopesContainer.innerHTML = '<div class="text-slate-500 italic text-[11px] text-center py-4">// Code highlights stopped. Press Step or Play to resume.</div>';
    }
    
    // Re-apply standard layout / reset traversal highlights
    resetTraversal();
};

window.prevExecutionStep = function() {
    if (!window.executionTrace || window.executionTrace.length === 0) return;
    if (window.currentExecutionStep > 0) {
        window.previousStepVars = {};
        renderExecutionStep(window.currentExecutionStep - 1);
    }
};

window.nextExecutionStep = function() {
    if (!window.executionTrace || window.executionTrace.length === 0) return;
    if (window.currentExecutionStep < window.executionTrace.length - 1) {
        renderExecutionStep(window.currentExecutionStep + 1);
    }
};

window.toggleExecutionPlay = toggleExecutionPlay;
window.updateStepperSpeed = updateStepperSpeed;
window.stopExecutionStepper = stopExecutionStepper;

function renderExecutionStep(index) {
    if (!window.executionTrace || window.executionTrace.length === 0) return;
    if (index < 0 || index >= window.executionTrace.length) return;
    
    window.currentExecutionStep = index;
    
    // Update stepper status
    const statusLabel = document.getElementById('edu-stepper-status');
    if (statusLabel) {
        statusLabel.innerText = `Step ${index + 1} / ${window.executionTrace.length}`;
    }
    
    const step = window.executionTrace[index];
    
    // Update active node name
    const activeNodeLabel = document.getElementById('edu-stepper-active-node');
    if (activeNodeLabel) {
        activeNodeLabel.innerText = step.node_type.replace("Node", "");
    }
    
    // Highlight AST Tree Node in D3
    const activeStableId = step.node_type + (step.pos_start ? `_${step.pos_start.line}_${step.pos_start.col}` : '');
    highlightExecutionNode(activeStableId, step.pos_start, step.pos_end);
    
    // Highlight Monaco Editor range in green
    if (eduEditorInstance && step.pos_start && step.pos_end) {
        const startLine = step.pos_start.line + 1;
        const startCol = step.pos_start.col + 1;
        const endLine = step.pos_end.line + 1;
        const endCol = step.pos_end.col + 1;
        
        if (startLine && startCol && endLine && endCol) {
            window.eduExecutionDecorations = eduEditorInstance.deltaDecorations(window.eduExecutionDecorations || [], [
                {
                    range: new monaco.Range(startLine, startCol, endLine, endCol),
                    options: {
                        inlineClassName: 'monaco-execution-highlight',
                        className: 'monaco-execution-line-highlight',
                        isWholeLine: false,
                        hoverMessage: { value: `**Executing**: ${step.node_type.replace("Node", "")}` }
                    }
                }
            ]);
            eduEditorInstance.revealRangeInCenterIfOutsideViewport(new monaco.Range(startLine, startCol, endLine, endCol));
        }
    }
    
    // Render variables in scope
    renderScopes(step.scopes);
}

function highlightExecutionNode(activeStableId, pos_start, pos_end) {
    if (!d3Svg) return;
    
    // Map virtual node types like FunctionCallReturn and ProgramEnd to their physical equivalents for SVG node matching
    let searchNodeType = activeStableId.split('_')[0];
    if (searchNodeType === "FunctionCallReturn") {
        searchNodeType = "FunctionCallNode";
    } else if (searchNodeType === "ProgramEnd") {
        searchNodeType = "ListNode";
    } else if (searchNodeType === "FunctionReturn") {
        searchNodeType = "ListNode";
    }
    
    const searchStableId = searchNodeType + (pos_start ? `_${pos_start.line}_${pos_start.col}` : '');
    let activeId = null;
    
    // Find the active node's SVG ID based on its stableId
    d3Svg.selectAll(".ast-node").each(function (d) {
        if (d.data.stableId === searchStableId) {
            activeId = d.data.id;
        }
    });
    
    // If not found by stableId, try finding by matching line and column position
    if (!activeId && pos_start) {
        d3Svg.selectAll(".ast-node").each(function (d) {
            if (d.data.pos_start && d.data.pos_start.line === pos_start.line && d.data.pos_start.col === pos_start.col && d.data.nodeType === searchNodeType) {
                activeId = d.data.id;
            }
        });
    }
    
    if (activeId) {
        d3Svg.selectAll(".ast-node")
            .style("pointer-events", "auto")
            .style("opacity", 1)
            .each(function (d) {
                const nodeElement = d3.select(this);
                if (d.data.id === activeId) {
                    nodeElement.classed("highlighted", true).classed("muted", false);
                } else {
                    nodeElement.classed("muted", true).classed("highlighted", false);
                }
            });
            
        d3Svg.selectAll(".ast-link")
            .style("opacity", 1)
            .each(function (d) {
                const linkElement = d3.select(this);
                const isConnected = (d.source.data.id === activeId || d.target.data.id === activeId);
                if (isConnected) {
                    linkElement.classed("highlighted", true).classed("muted", false);
                } else {
                    linkElement.classed("muted", true).classed("highlighted", false);
                }
            });
            
        if (d3ZoomBehavior && d3NodeCoords) {
            const coords = d3NodeCoords[activeId];
            if (coords) {
                const container = document.getElementById('edu-ast-tree-container');
                if (container) {
                    const W = container.clientWidth || 400;
                    const H = container.clientHeight || 500;
                    const k = 0.85;
                    const tx = W / 2 - k * coords.x;
                    const ty = H / 2 - k * coords.y;
                    
                    d3Svg.transition()
                        .duration(400)
                        .ease(d3.easeCubicOut)
                        .call(d3ZoomBehavior.transform, d3.zoomIdentity.translate(tx, ty).scale(k));
                }
            }
        }
    }
}

function renderVariableRow(v, scopeIdx, scopeName, parentPath = "") {
    const varKey = `${scopeIdx}_${scopeName}_${parentPath}${v.name}`;
    const prevVal = window.previousStepVars[varKey];
    const isUpdated = (prevVal !== undefined && prevVal !== v.value);
    const isNew = (prevVal === undefined && window.currentExecutionStep > 0);
    
    let highlightClass = "";
    if (isUpdated || isNew) {
        highlightClass = "animate-var-update";
    }
    if (v.is_accessed) {
        highlightClass += " accessed-var-highlight";
    }
    
    let valColor = "text-slate-200";
    if (v.type === "Integer" || v.type === "Float") valColor = "text-amber-400 font-semibold";
    else if (v.type === "String") valColor = "text-emerald-400 font-semibold";
    else if (v.type === "List" || v.type === "Dictionary" || v.type === "Dict") valColor = "text-sky-400 font-bold";
    else if (v.type === "Boolean") valColor = "text-pink-400 font-bold";
    else if (v.type === "ModelInstance" || v.type === "Model") valColor = "text-purple-400 font-bold";
    
    let rowHtml = `
        <div class="whitespace-nowrap text-indigo-200 font-semibold select-all py-0.5 px-1 rounded ${highlightClass}">${escapeHtml(v.name)}</div>
        <div class="text-[9px] font-black uppercase text-slate-500 bg-slate-900 border border-slate-800 px-1.5 py-0.5 rounded self-center select-none tracking-wide">${v.type}</div>
        <div class="whitespace-nowrap select-all py-0.5 px-1.5 rounded bg-slate-950/70 border border-slate-900/60 shadow-inner font-bold ${valColor} ${highlightClass}" title="${escapeHtml(v.value)}">${escapeHtml(v.value)}</div>
    `;
    
    if (v.props && v.props.length > 0) {
        rowHtml += `
            <div class="col-span-3 pl-3 ml-2 border-l-2 border-indigo-500/30 bg-indigo-950/5 p-2 rounded-r-lg space-y-1">
                <div class="text-[9px] font-bold text-indigo-400 select-none uppercase tracking-wider flex items-center space-x-1">
                    <i data-lucide="layers" class="h-3 w-3 text-indigo-450 shrink-0"></i>
                    <span>Attributes:</span>
                </div>
                <div class="grid grid-cols-[minmax(60px,auto)_auto_minmax(80px,auto)] gap-x-2 gap-y-1 font-mono min-w-[max-content]">
        `;
        v.props.forEach(prop => {
            rowHtml += renderVariableRow(prop, scopeIdx, scopeName, parentPath + v.name + ".");
        });
        rowHtml += `
                </div>
            </div>
        `;
    }
    
    window.nextStepVars[varKey] = v.value;
    return rowHtml;
}

function renderScopes(scopes) {
    const container = document.getElementById('edu-stepper-scopes');
    if (!container) return;
    
    if (!scopes || scopes.length === 0) {
        container.innerHTML = '<div class="text-slate-500 italic text-[11px] text-center py-4">// No variables in scope.</div>';
        return;
    }
    
    let html = "";
    window.nextStepVars = {};
    
    scopes.forEach((scope, scopeIdx) => {
        let scopeTitle = scope.name;
        const isGlobalScope = (scopeTitle === "<program>");
        if (isGlobalScope) scopeTitle = "Global Scope";
        else if (scopeIdx === 0) scopeTitle = `Local Scope (${scopeTitle})`;
        else scopeTitle = `Enclosing Scope (${scopeTitle})`;
        
        let parentInfo = "";
        if (scope.parent && scope.parent !== "None") {
            let parentDisp = scope.parent === "<program>" ? "Global Scope" : scope.parent;
            parentInfo = `<span class="text-[9px] text-slate-550 text-indigo-400 font-medium normal-case ml-auto flex items-center select-none"><i data-lucide="corner-down-right" class="h-3 w-3 mr-1 text-indigo-500/70"></i>Parent: ${parentDisp}</span>`;
        }
        
        html += `
            <div class="rounded-xl border border-slate-800/80 bg-slate-950/40 p-3 shadow-sm flex flex-col space-y-2">
                <div class="flex items-center text-xs font-bold text-slate-350 border-b border-slate-850/60 pb-1.5 uppercase select-none w-full">
                    <i data-lucide="${scopeIdx === 0 ? 'box' : 'database'}" class="h-3.5 w-3.5 text-indigo-400 mr-1.5 shrink-0"></i>
                    <span>${scopeTitle}</span>
                    ${parentInfo}
                </div>
        `;
        
        if (!scope.variables || scope.variables.length === 0) {
            html += `<div class="text-slate-550 italic text-[10px] pl-5 py-0.5">// No variables defined.</div>`;
        } else {
            const scrollWrapperClass = isGlobalScope ? "max-h-48 overflow-y-auto pr-1" : "";
            html += `
                <div class="${scrollWrapperClass}">
                    <div class="grid grid-cols-[minmax(80px,auto)_auto_minmax(100px,auto)] gap-x-3 gap-y-2 font-mono text-[11px] pl-1.5 pr-1 py-1 min-w-[max-content]">
            `;
            
            scope.variables.forEach(v => {
                html += renderVariableRow(v, scopeIdx, scope.name, "");
            });
            
            html += `
                    </div>
                </div>
            `;
        }
        
        html += `</div>`;
    });
    
    container.innerHTML = html;
    window.previousStepVars = window.nextStepVars;
    delete window.nextStepVars;
    lucide.createIcons();
}

function getTokenBadgeHTML(type, value) {
    let classes = "inline-flex items-center px-2 py-0.5 rounded text-[10px] font-bold tracking-wide border transition-all";
    if (type === "KEYWORD") {
        classes += " bg-fuchsia-500/15 text-fuchsia-400 border-fuchsia-500/30 shadow-[0_0_6px_rgba(217,70,239,0.15)]";
    } else if (type === "INT" || type === "FLOAT") {
        classes += " bg-amber-500/15 text-amber-400 border-amber-500/30 shadow-[0_0_6px_rgba(245,158,11,0.15)]";
    } else if (type === "STRING" || type === "FSTRING") {
        classes += " bg-emerald-500/15 text-emerald-400 border-emerald-500/30 shadow-[0_0_6px_rgba(16,185,129,0.15)]";
    } else if (type === "IDENTIFIER") {
        classes += " bg-blue-500/15 text-blue-400 border-blue-500/30 shadow-[0_0_6px_rgba(59,130,246,0.15)]";
    } else if (type === "NEWLINE") {
        classes += " bg-slate-800/50 text-slate-400 border-slate-700/40";
    } else if (type === "EOF") {
        classes += " bg-rose-500/15 text-rose-400 border-rose-500/30 shadow-[0_0_6px_rgba(239,68,68,0.15)]";
    } else {
        const bracketTypes = [
            'LPAREN', 'RPAREN', 'LPAREN2', 'RPAREN2', 'LPAREN3', 'RPAREN3', 'COMMA', 'DOT'
        ];
        if (bracketTypes.includes(type)) {
            classes += " bg-teal-500/15 text-teal-400 border-teal-500/30 shadow-[0_0_6px_rgba(20,184,166,0.15)]";
        } else {
            classes += " bg-pink-500/15 text-pink-400 border-pink-500/30 shadow-[0_0_6px_rgba(236,72,153,0.15)]";
        }
    }
    return `<span class="${classes}">${type}</span>`;
}

function renderTokensTable(tokens) {
    const tbody = document.getElementById('edu-tokens-table-body');
    if (!tbody) return;
    tbody.innerHTML = '';

    if (!tokens || tokens.length === 0) {
        tbody.innerHTML = '<tr><td colspan="2" class="p-4 text-center text-slate-500 italic">// No tokens found.</td></tr>';
        return;
    }

    tokens.forEach(tok => {
        const tr = document.createElement('tr');
        tr.className = "hover:bg-slate-900/60 border-b border-slate-900/60 transition-all duration-200 group hover:translate-x-1";

        const tdType = document.createElement('td');
        tdType.className = "py-2.5 px-3 font-mono align-middle";
        tdType.innerHTML = getTokenBadgeHTML(tok.type, tok.value);

        const tdValue = document.createElement('td');
        tdValue.className = "py-2.5 px-3 font-mono text-xs align-middle";

        let val = tok.value;
        if (val === null || val === undefined) {
            val = TOKEN_LITERALS[tok.type] || '';
        }

        let valHTML = '';
        if (tok.type === 'STRING' || tok.type === 'FSTRING') {
            valHTML = `<span class="text-emerald-400 font-semibold">"${escapeHtml(val)}"</span>`;
        } else if (tok.type === 'INT' || tok.type === 'FLOAT') {
            valHTML = `<span class="text-amber-400 font-semibold">${val}</span>`;
        } else if (tok.type === 'KEYWORD') {
            valHTML = `<span class="text-indigo-300 font-bold font-sans">${val}</span>`;
        } else if (tok.type === 'IDENTIFIER') {
            valHTML = `<span class="text-cyan-300 font-medium">${val}</span>`;
        } else if (TOKEN_LITERALS[tok.type]) {
            valHTML = `<span class="text-pink-400 font-bold">${escapeHtml(val)}</span>`;
        } else {
            valHTML = `<span class="text-slate-300">${escapeHtml(val)}</span>`;
        }

        tdValue.innerHTML = valHTML;

        tr.appendChild(tdType);
        tr.appendChild(tdValue);
        tbody.appendChild(tr);
    });
}
