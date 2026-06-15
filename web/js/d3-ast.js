window.collapsedNodeIds = new Set();
window.toggleNodeCollapse = function(stableId) {
    if (!stableId) return;
    if (window.collapsedNodeIds.has(stableId)) {
        window.collapsedNodeIds.delete(stableId);
    } else {
        window.collapsedNodeIds.add(stableId);
    }
    if (latestAst) {
        renderAstTree(latestAst);
    }
};

function getPositionAtIndex(sourceCode, index) {
    let line = 0;
    let col = 0;
    for (let i = 0; i < index; i++) {
        if (sourceCode[i] === '\n') {
            line++;
            col = 0;
        } else {
            col++;
        }
    }
    return { index, line, col };
}

function skipWhitespaceAndComments(sourceCode, index) {
    let i = index;
    while (i < sourceCode.length) {
        let c = sourceCode[i];
        if (c === ' ' || c === '\t' || c === '\r' || c === '\n') {
            i++;
        } else if (c === '#') {
            if (i + 1 < sourceCode.length && sourceCode[i + 1] === '*') {
                i += 2;
                while (i < sourceCode.length) {
                    if (sourceCode[i] === '*' && i + 1 < sourceCode.length && sourceCode[i + 1] === '#') {
                        i += 2;
                        break;
                    }
                    i++;
                }
            } else {
                while (i < sourceCode.length && sourceCode[i] !== '\n') {
                    i++;
                }
            }
        } else {
            break;
        }
    }
    return i;
}

function scanBackwardToChar(sourceCode, index, charToFind) {
    let i = index;
    while (i >= 0) {
        if (sourceCode[i] === charToFind) {
            return i;
        }
        i--;
    }
    return -1;
}

function findMatchingClosing(sourceCode, startIdx, openChar, closeChar) {
    let depth = 1;
    let i = startIdx;
    while (i < sourceCode.length) {
        let c = sourceCode[i];
        if (c === '"') {
            i++;
            while (i < sourceCode.length) {
                if (sourceCode[i] === '"' && sourceCode[i - 1] !== '\\') {
                    break;
                }
                i++;
            }
        } else if (c === openChar) {
            depth++;
        } else if (c === closeChar) {
            depth--;
            if (depth === 0) {
                return i;
            }
        }
        i++;
    }
    return -1;
}

function adjustFunctionCallPosition(sourceCode, pos_start, pos_end) {
    if (!sourceCode || !pos_start || !pos_end) return { pos_start, pos_end };
    let idx = pos_end.index;
    idx = skipWhitespaceAndComments(sourceCode, idx);
    if (idx < sourceCode.length && sourceCode[idx] === '(') {
        idx++;
        let matchIdx = findMatchingClosing(sourceCode, idx, '(', ')');
        if (matchIdx !== -1) {
            return { pos_start, pos_end: getPositionAtIndex(sourceCode, matchIdx) };
        }
    } else if (idx < sourceCode.length && sourceCode[idx] === ')') {
        return { pos_start, pos_end: getPositionAtIndex(sourceCode, idx) };
    }
    return { pos_start, pos_end };
}

function adjustListPosition(sourceCode, pos_start, pos_end) {
    if (!sourceCode || !pos_start || !pos_end) return { pos_start, pos_end };
    let startIdx = pos_start.index;
    startIdx = scanBackwardToChar(sourceCode, startIdx, '[');
    let new_pos_start = startIdx !== -1 ? getPositionAtIndex(sourceCode, startIdx) : pos_start;

    let idx = pos_end.index;
    idx = skipWhitespaceAndComments(sourceCode, idx);
    if (idx < sourceCode.length && sourceCode[idx] === ']') {
        return { pos_start: new_pos_start, pos_end: getPositionAtIndex(sourceCode, idx) };
    }
    return { pos_start: new_pos_start, pos_end };
}

function adjustDictPosition(sourceCode, pos_start, pos_end) {
    if (!sourceCode || !pos_start || !pos_end) return { pos_start, pos_end };
    let startIdx = pos_start.index;
    startIdx = scanBackwardToChar(sourceCode, startIdx, '{');
    let new_pos_start = startIdx !== -1 ? getPositionAtIndex(sourceCode, startIdx) : pos_start;

    let idx = pos_end.index;
    idx = skipWhitespaceAndComments(sourceCode, idx);
    if (idx < sourceCode.length && sourceCode[idx] === '}') {
        return { pos_start: new_pos_start, pos_end: getPositionAtIndex(sourceCode, idx) };
    }
    return { pos_start: new_pos_start, pos_end };
}

function adjustKeywordNodeStart(sourceCode, pos_start, keyword) {
    if (!sourceCode || !pos_start || !keyword) return pos_start;
    let idx = pos_start.index;
    while (idx > 0 && /\s/.test(sourceCode[idx - 1])) {
        idx--;
    }
    if (idx >= keyword.length) {
        let potentialKeyword = sourceCode.slice(idx - keyword.length, idx);
        if (potentialKeyword === keyword) {
            if (idx - keyword.length === 0 || !/[a-zA-Z0-9_]/.test(sourceCode[idx - keyword.length - 1])) {
                return getPositionAtIndex(sourceCode, idx - keyword.length);
            }
        }
    }
    return pos_start;
}

function minPosition(p1, p2) {
    if (!p1) return p2;
    if (!p2) return p1;
    return p1.index <= p2.index ? p1 : p2;
}

function maxPosition(p1, p2) {
    if (!p1) return p2;
    if (!p2) return p1;
    return p1.index >= p2.index ? p1 : p2;
}

function extractAstNode(obj) {
    if (!obj || typeof obj !== 'object') return null;

    // Assign custom virtual node types to case structures for clean rendering
    if (!obj.node_type) {
        if (obj.condition !== undefined && obj.body !== undefined) {
            obj.node_type = 'IfCaseNode';
        } else if (obj.value !== undefined && obj.body !== undefined) {
            if (obj.value === null) {
                obj.node_type = 'FallbackCaseNode';
            } else {
                obj.node_type = 'SwitchCaseNode';
            }
        } else if (obj.body !== undefined && obj.condition === undefined && obj.value === undefined) {
            obj.node_type = 'ElseCaseNode';
        }
    }

    let nodeType = obj.node_type || 'Node';

    // Intercept and simplify FStringNode format from WASM on the fly
    if (nodeType === 'FStringNode' && obj.parts) {
        let template = "";
        let expressions = [];
        obj.parts.forEach(part => {
            if (part.kind === 'literal') {
                template += part.value;
            } else if (part.kind === 'expr' && part.value) {
                template += "{...}";
                expressions.push(part.value);
            }
        });
        delete obj.parts;
        obj.template = template;
        obj.expressions = expressions;
    }

    let value = null;
    let children = [];

    // Extract values from token wrapper structures, fallback to mapped literals if value is empty/null
    if (obj.token && obj.token.value !== undefined) {
        value = obj.token.value !== null ? obj.token.value : (TOKEN_LITERALS[obj.token.type] || null);
    } else if (obj.operator && obj.operator.value !== undefined) {
        value = obj.operator.value !== null ? obj.operator.value : (TOKEN_LITERALS[obj.operator.type] || null);
    } else if (obj.var_name_tok && obj.var_name_tok.value !== undefined) {
        value = obj.var_name_tok.value !== null ? obj.var_name_tok.value : (TOKEN_LITERALS[obj.var_name_tok.type] || null);
    } else if (obj.module_tok && obj.module_tok.value !== undefined) {
        value = obj.module_tok.value !== null ? obj.module_tok.value : (TOKEN_LITERALS[obj.module_tok.type] || null);
    } else if (obj.name_tok && obj.name_tok.value !== undefined) {
        value = obj.name_tok.value !== null ? obj.name_tok.value : (TOKEN_LITERALS[obj.name_tok.type] || null);
    } else if (obj.template !== undefined) {
        value = obj.template;
    } else if (obj.value !== undefined && typeof obj.value !== 'object') {
        value = obj.value;
    }

    // Find direct positions on this object's fields
    let pos_start = obj.pos_start || null;
    let pos_end = obj.pos_end || null;

    if (!pos_start || !pos_end) {
        Object.keys(obj).forEach(key => {
            const val = obj[key];
            if (val && typeof val === 'object' && val.pos_start !== undefined && val.pos_end !== undefined) {
                pos_start = minPosition(pos_start, val.pos_start);
                pos_end = maxPosition(pos_end, val.pos_end);
            }
        });
    }

    // Recursively extract AST sub-nodes
    Object.keys(obj).forEach(key => {
        if (key === 'node_type' || key === 'token' || key === 'operator' || key === 'var_name_tok' || key === 'module_tok' || key === 'name_tok' || key === 'pos_start' || key === 'pos_end') {
            return;
        }

        const val = obj[key];
        if (!val) return;

        if (typeof val === 'object') {
            if (Array.isArray(val)) {
                val.forEach(item => {
                    let child = extractAstNode(item);
                    if (child) {
                        child.label = key;
                        children.push(child);
                        if (!obj.pos_start || !obj.pos_end) {
                            pos_start = minPosition(pos_start, child.pos_start);
                            pos_end = maxPosition(pos_end, child.pos_end);
                        }
                    }
                });
            } else {
                let child = extractAstNode(val);
                if (child) {
                    child.label = key;
                    children.push(child);
                    if (!obj.pos_start || !obj.pos_end) {
                        pos_start = minPosition(pos_start, child.pos_start);
                        pos_end = maxPosition(pos_end, child.pos_end);
                    }
                }
            }
        }
    });

    // Let's get the active file source code
    let sourceCode = (activeFilename && virtualFiles[activeFilename]) || "";

    // Adjust positions based on nodeType and sourceCode content
    if (sourceCode && pos_start && pos_end) {
        if (nodeType === 'FunctionCallNode') {
            const adjusted = adjustFunctionCallPosition(sourceCode, pos_start, pos_end);
            pos_start = adjusted.pos_start;
            pos_end = adjusted.pos_end;
        } else if (nodeType === 'ListNode' || nodeType === 'ListComprehensionNode') {
            if (!obj.is_block) {
                const adjusted = adjustListPosition(sourceCode, pos_start, pos_end);
                pos_start = adjusted.pos_start;
                pos_end = adjusted.pos_end;
            }
        } else if (nodeType === 'DictNode' || nodeType === 'DictComprehensionNode') {
            const adjusted = adjustDictPosition(sourceCode, pos_start, pos_end);
            pos_start = adjusted.pos_start;
            pos_end = adjusted.pos_end;
        } else if (nodeType === 'ForNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "cycle");
        } else if (nodeType === 'WhileNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "during");
        } else if (nodeType === 'FunctionDefinitionNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "method");
        } else if (nodeType === 'TryNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "risk");
        } else if (nodeType === 'CatchNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "trap");
        } else if (nodeType === 'FinallyNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "clean");
        } else if (nodeType === 'ModelNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "model");
        } else if (nodeType === 'IfNode') {
            pos_start = adjustKeywordNodeStart(sourceCode, pos_start, "when");
        }
    }

    const stableId = nodeType + (pos_start ? `_${pos_start.line}_${pos_start.col}` : '');
    const isCollapsed = window.collapsedNodeIds && window.collapsedNodeIds.has(stableId);

    const returnedNode = { nodeType, value, children, pos_start, pos_end, stableId };
    if (isCollapsed) {
        returnedNode._children = children;
        returnedNode.children = [];
    }
    return returnedNode;
}

// DFS pre-order traversal extraction
function buildDfsQueue(simplifiedAst) {
    const result = [];
    if (!simplifiedAst) return result;

    let idCounter = 0;
    function traverse(node) {
        if (!node) return;
        node.id = "ast-node-" + idCounter;
        node.dfsIndex = idCounter;
        idCounter++;
        result.push(node);

        if (node.children && node.children.length > 0) {
            node.children.forEach(child => {
                traverse(child);
            });
        }
    }
    traverse(simplifiedAst);
    return result;
}

function focusNode(nodeId) {
    if (!nodeId) return;

    // Find the node index in the DFS queue
    const index = dfsNodesQueue.findIndex(n => n.id === nodeId);
    if (index !== -1) {
        window.isCursorSourcedFocus = true;
        currentTraversalIndex = index;
        highlightCurrentNode(true, 'tree');
        setTimeout(() => {
            window.isCursorSourcedFocus = false;
        }, 300);
    }
}

function highlightCurrentNode(centerViewport = true, source = 'tree') {
    if (!d3Svg) return;

    if (currentTraversalIndex < 0 || currentTraversalIndex >= dfsNodesQueue.length) {
        // If nothing highlighted (exploration mode), clear all muting and show everything
        d3Svg.selectAll(".ast-node")
            .style("pointer-events", "auto")
            .classed("highlighted", false)
            .classed("muted", false)
            .transition()
            .duration(300)
            .style("opacity", 1);

        d3Svg.selectAll(".ast-link")
            .classed("highlighted", false)
            .classed("muted", false)
            .transition()
            .duration(300)
            .style("opacity", 1);

        // Clear editor decorations
        if (eduEditorInstance && window.eduEditorDecorations) {
            window.eduEditorDecorations = eduEditorInstance.deltaDecorations(window.eduEditorDecorations, []);
        }

        updateTraversalBarUI();
        return;
    }

    const activeNode = dfsNodesQueue[currentTraversalIndex];
    const activeId = activeNode.id;

    // Apply SVG node visibility and styles based on DFS index
    d3Svg.selectAll(".ast-node").each(function (d) {
        const nodeElement = d3.select(this);
        const nodeDfsIndex = d.data.dfsIndex;
        const isVisible = (currentTraversalIndex === -1 || nodeDfsIndex <= currentTraversalIndex);

        if (isVisible) {
            nodeElement
                .style("pointer-events", "auto")
                .transition()
                .duration(300)
                .style("opacity", 1);

            if (d.data.id === activeId) {
                nodeElement.classed("highlighted", true).classed("muted", false);
            } else {
                nodeElement.classed("muted", true).classed("highlighted", false);
            }
        } else {
            nodeElement
                .style("pointer-events", "none")
                .transition()
                .duration(200)
                .style("opacity", 0);
            nodeElement.classed("highlighted", false).classed("muted", false);
        }
    });

    // Apply SVG link visibility and styles based on DFS target index
    d3Svg.selectAll(".ast-link").each(function (d) {
        const linkElement = d3.select(this);
        const targetDfsIndex = d.target.data.dfsIndex;
        const isVisible = (currentTraversalIndex === -1 || targetDfsIndex <= currentTraversalIndex);

        if (isVisible) {
            linkElement
                .transition()
                .duration(300)
                .style("opacity", 1);

            const isConnected = (d.source.data.id === activeId || d.target.data.id === activeId);
            if (isConnected) {
                linkElement.classed("highlighted", true).classed("muted", false);
            } else {
                linkElement.classed("muted", true).classed("highlighted", false);
            }
        } else {
            linkElement
                .transition()
                .duration(200)
                .style("opacity", 0);
            linkElement.classed("highlighted", false).classed("muted", false);
        }
    });

    updateTraversalBarUI();

    // Highlight corresponding range in Monaco Editor
    if (eduEditorInstance) {
        if (!window.eduEditorDecorations) {
            window.eduEditorDecorations = [];
        }
        if (source === 'tree' && activeNode.pos_start && activeNode.pos_end) {
            const startLine = activeNode.pos_start.line + 1;
            const startCol = activeNode.pos_start.col + 1;
            const endLine = activeNode.pos_end.line + 1;
            const endCol = activeNode.pos_end.col + 1;

            if (startLine && startCol && endLine && endCol) {
                window.eduEditorDecorations = eduEditorInstance.deltaDecorations(window.eduEditorDecorations, [
                    {
                        range: new monaco.Range(startLine, startCol, endLine, endCol),
                        options: {
                            inlineClassName: 'monaco-ast-node-highlight',
                            className: 'monaco-ast-node-line-highlight',
                            isWholeLine: false,
                            hoverMessage: { value: `**AST Node**: ${activeNode.nodeType.replace("Node", "")}` }
                        }
                    }
                ]);

                if (centerViewport) {
                    eduEditorInstance.revealRangeInCenterIfOutsideViewport(new monaco.Range(startLine, startCol, endLine, endCol));
                }
            } else {
                window.eduEditorDecorations = eduEditorInstance.deltaDecorations(window.eduEditorDecorations, []);
            }
        } else if (source !== 'tree') {
            // Clear editor decorations when moving cursor in the editor so they don't overlay the cursor
            window.eduEditorDecorations = eduEditorInstance.deltaDecorations(window.eduEditorDecorations, []);
        }
    }

    // Auto-focus: pan viewport so the active node is visible/centered
    if (d3ZoomBehavior && d3Svg) {
        const coords = d3NodeCoords[activeId];
        if (coords) {
            const container = document.getElementById('edu-ast-tree-container');
            if (container) {
                const W = container.clientWidth || 400;
                const H = container.clientHeight || 500;

                if (centerViewport) {
                    const k = 0.85; // Use fixed clean layout scale 0.85
                    const tx = W / 2 - k * coords.x;
                    const ty = H / 2 - k * coords.y;

                    d3Svg.transition()
                        .duration(500)
                        .ease(d3.easeCubicOut)
                        .call(d3ZoomBehavior.transform, d3.zoomIdentity.translate(tx, ty).scale(k));
                }
            }
        }
    }
}

function stepTraversal(direction) {
    if (dfsNodesQueue.length === 0) return;

    currentTraversalIndex += direction;
    if (currentTraversalIndex < 0) {
        currentTraversalIndex = 0;
    } else if (currentTraversalIndex >= dfsNodesQueue.length) {
        currentTraversalIndex = dfsNodesQueue.length - 1;
    }

    highlightCurrentNode(true);
}

function resetTraversal() {
    currentTraversalIndex = -1;
    highlightCurrentNode(false);

    // Re-center tree layout completely (top center for vertical layout)
    if (d3ZoomBehavior && d3Svg) {
        const container = document.getElementById('edu-ast-tree-container');
        const width = container.clientWidth || 400;
        const initialTransform = d3.zoomIdentity.translate(width / 2, 60).scale(0.85);

        d3Svg.transition()
            .duration(500)
            .call(d3ZoomBehavior.transform, initialTransform);
    }
}

function updateTraversalBarUI() {
    const label = document.getElementById('edu-traversal-mode-label');
    const status = document.getElementById('edu-traversal-status');

    if (currentTraversalIndex === -1) {
        if (label) label.innerText = "Exploration Mode";
        if (status) status.innerText = `0 / ${dfsNodesQueue.length} nodes`;
    } else {
        if (label) label.innerText = "Traversal Mode (DFS)";
        if (status) status.innerText = `${currentTraversalIndex + 1} / ${dfsNodesQueue.length} nodes`;
    }
}

const NODE_EXPLANATIONS = {
    'IfNode': 'Evaluates a conditional expression. If it is true, the matching branch is executed.',
    'IfCaseNode': 'A branch of a conditional expression (when or orwhen) containing a condition and a body.',
    'ElseCaseNode': 'An otherwise branch. It is executed if all previous conditions evaluate to false.',
    'SwitchNode': 'A menu statement. It matches a value against multiple choice cases or fallback.',
    'SwitchCaseNode': 'A choice branch of a menu statement that matches a specific value.',
    'FallbackCaseNode': 'A fallback branch of a menu statement that executes if no choices match.',
    'ForNode': 'A loop that iterates over a numeric range from start to end with a step value.',
    'WhileNode': 'A loop that repeatedly executes its body as long as the condition remains true.',
    'ForEachLoopNode': 'A loop that iterates over every item in a list or collection.',
    'NumberNode': 'A primitive numeric literal (e.g. 42 or 3.14).',
    'StringNode': 'A primitive sequence of text characters.',
    'FStringNode': 'An interpolated string (f-string) supporting expressions inside curly braces.',
    'VariableUseNode': 'Retrieves the current value stored in a variable by its name.',
    'VariableAssignNode': 'Assigns or updates the value of one or more variables.',
    'BinaryOperationNode': 'An operation between two nodes (e.g. addition, subtraction, comparison).',
    'UnaryOperationNode': 'An operation on a single node (e.g. logical not, negative sign).',
    'TernaryOperationNode': 'An inline conditional assignment: condition ? true_val : false_val.',
    'ListNode': 'A collection of expressions grouped together inside brackets [].',
    'DictNode': 'A collection of key-value pairs stored in a dictionary.',
    'IndexAccessNode': 'Accesses an element at a specific index inside a list or string.',
    'AttrAccessNode': 'Retrieves the value of a property/attribute on an object.',
    'AttrAssignNode': 'Sets the value of a property/attribute on an object.',
    'ClassNode': 'Defines a class (model) schema with attributes and methods.',
    'FunctionDefinitionNode': 'Declares a reusable block of code with arguments.',
    'MethodDefinitionNode': 'Declares a function bound to a class model (method).',
    'FunctionCallNode': 'Invokes a function or method with positional and keyword arguments.',
    'ReturnNode': 'Returns a value from the current function (yield keyword).',
    'ContinueNode': 'Skips the rest of the current loop iteration (proceed keyword).',
    'BreakNode': 'Terminates and exits the loop immediately (escape keyword).',
    'TryNode': 'A block of code wrapped with risk/trap error handling.',
    'FinallyNode': 'A block of code that always runs after try/catch, regardless of errors.',
    'SummonNode': 'Imports modules or external components into the namespace.'
};

const NODE_TYPE_STYLING = {
    'IfCaseNode': { icon: 'git-commit', color: 'text-orange-400 font-extrabold', name: 'When Branch', border: 'border-orange-500/50 border-2', bg: 'bg-orange-500/10' },
    'SwitchCaseNode': { icon: 'git-commit', color: 'text-orange-400 font-extrabold', name: 'Choice Branch', border: 'border-orange-500/50 border-2', bg: 'bg-orange-500/10' },
    'ElseCaseNode': { icon: 'git-commit', color: 'text-orange-400 font-extrabold', name: 'Otherwise Branch', border: 'border-orange-500/50 border-2', bg: 'bg-orange-500/10' },
    'FallbackCaseNode': { icon: 'git-commit', color: 'text-orange-400 font-extrabold', name: 'Fallback Branch', border: 'border-orange-500/50 border-2', bg: 'bg-orange-500/10' },
    'ContinueNode': { icon: 'arrow-right', color: 'text-rose-400 font-extrabold', name: 'Proceed', border: 'border-rose-500/50 border-2', bg: 'bg-rose-500/10' },
    'BreakNode': { icon: 'x-circle', color: 'text-rose-400 font-extrabold', name: 'Escape', border: 'border-rose-500/50 border-2', bg: 'bg-rose-500/10' },
    'NumberNode': { icon: 'hash', color: 'text-amber-400 font-extrabold', name: 'Number', border: 'border-amber-500/50 border-2', bg: 'bg-amber-500/10' },
    'StringNode': { icon: 'type', color: 'text-emerald-400 font-extrabold', name: 'String', border: 'border-emerald-500/50 border-2', bg: 'bg-emerald-500/10' },
    'FStringNode': { icon: 'type', color: 'text-emerald-400 font-extrabold', name: 'F-String', border: 'border-emerald-500/50 border-2', bg: 'bg-emerald-500/10' },
    'VariableUseNode': { icon: 'user', color: 'text-cyan-400 font-extrabold', name: 'Variable', border: 'border-cyan-500/50 border-2', bg: 'bg-cyan-500/10' },
    'VariableAssignNode': { icon: 'edit-3', color: 'text-indigo-400 font-extrabold', name: 'Assign', border: 'border-indigo-500/50 border-2', bg: 'bg-indigo-500/10' },
    'BinaryOperationNode': { icon: 'git-commit', color: 'text-pink-400 font-extrabold', name: 'Binary Op', border: 'border-pink-500/50 border-2', bg: 'bg-pink-500/10' },
    'UnaryOperationNode': { icon: 'minus-circle', color: 'text-rose-400 font-extrabold', name: 'Unary Op', border: 'border-rose-500/50 border-2', bg: 'bg-rose-500/10' },
    'TernaryOperationNode': { icon: 'help-circle', color: 'text-purple-400 font-extrabold', name: 'Ternary Op', border: 'border-purple-500/50 border-2', bg: 'bg-purple-500/10' },
    'IndexAccessNode': { icon: 'key', color: 'text-yellow-400 font-extrabold', name: 'Index Access', border: 'border-yellow-500/50 border-2', bg: 'bg-yellow-500/10' },
    'ListNode': { icon: 'list', color: 'text-sky-400 font-extrabold', name: 'List', border: 'border-sky-500/50 border-2', bg: 'bg-sky-500/10' },
    'DictNode': { icon: 'database', color: 'text-teal-400 font-extrabold', name: 'Dictionary', border: 'border-teal-500/50 border-2', bg: 'bg-teal-500/10' },
    'IfNode': { icon: 'git-branch', color: 'text-orange-400 font-extrabold', name: 'If', border: 'border-orange-500/50 border-2', bg: 'bg-orange-500/10' },
    'SwitchNode': { icon: 'git-pull-request', color: 'text-orange-400 font-extrabold', name: 'Switch', border: 'border-orange-500/50 border-2', bg: 'bg-orange-500/10' },
    'ForNode': { icon: 'repeat', color: 'text-indigo-400 font-extrabold', name: 'For Loop', border: 'border-indigo-500/50 border-2', bg: 'bg-indigo-500/10' },
    'WhileNode': { icon: 'repeat', color: 'text-indigo-400 font-extrabold', name: 'While Loop', border: 'border-indigo-500/50 border-2', bg: 'bg-indigo-500/10' },
    'ForEachLoopNode': { icon: 'repeat', color: 'text-indigo-400 font-extrabold', name: 'ForEach Loop', border: 'border-indigo-500/50 border-2', bg: 'bg-indigo-500/10' },
    'FunctionCallNode': { icon: 'play-circle', color: 'text-violet-400 font-extrabold', name: 'Call', border: 'border-violet-500/50 border-2', bg: 'bg-violet-500/10' },
    'ReturnNode': { icon: 'corner-down-left', color: 'text-red-400 font-extrabold', name: 'Return', border: 'border-red-500/50 border-2', bg: 'bg-red-500/10' },
    'TryNode': { icon: 'shield-alert', color: 'text-amber-500', name: 'Try', border: 'border-amber-500/50 border-2', bg: 'bg-amber-500/10' },
    'FinallyNode': { icon: 'shield-check', color: 'text-emerald-500', name: 'Finally', border: 'border-emerald-500/50 border-2', bg: 'bg-emerald-500/10' },
    'AttrAccessNode': { icon: 'link', color: 'text-cyan-400 font-extrabold', name: 'Attr Access', border: 'border-cyan-500/50 border-2', bg: 'bg-cyan-500/10' },
    'AttrAssignNode': { icon: 'link-2', color: 'text-cyan-400 font-extrabold', name: 'Attr Assign', border: 'border-cyan-500/50 border-2', bg: 'bg-cyan-500/10' },
    'ClassNode': { icon: 'box', color: 'text-blue-400 font-extrabold', name: 'Class', border: 'border-blue-500/50 border-2', bg: 'bg-blue-500/10' },
    'FunctionDefinitionNode': { icon: 'code', color: 'text-violet-400 font-extrabold', name: 'Function', border: 'border-violet-500/50 border-2', bg: 'bg-violet-500/10' },
    'MethodDefinitionNode': { icon: 'code-2', color: 'text-violet-400 font-extrabold', name: 'Method', border: 'border-violet-500/50 border-2', bg: 'bg-violet-500/10' },
    'SummonNode': { icon: 'download-cloud', color: 'text-pink-400 font-extrabold', name: 'Summon', border: 'border-pink-500/50 border-2', bg: 'bg-pink-500/10' },
    'ListComprehensionNode': { icon: 'layers', color: 'text-sky-400 font-extrabold', name: 'Comprehension', border: 'border-sky-500/50 border-2', bg: 'bg-sky-500/10' }
};

function renderAstTree(astObj) {
    latestAst = astObj;
    const container = document.getElementById('edu-ast-tree-container');
    if (!container) return;
    container.innerHTML = '';

    const placeholder = document.getElementById('edu-ast-placeholder');
    if (placeholder) placeholder.classList.add('hidden');

    const traversalBar = document.getElementById('edu-ast-traversal-bar');
    if (traversalBar) traversalBar.classList.remove('hidden');

    if (!astObj) {
        container.innerHTML = '<div class="text-slate-500 italic p-4 text-center">// No AST available.</div>';
        if (traversalBar) traversalBar.classList.add('hidden');
        return;
    }

    const simplifiedAst = extractAstNode(astObj);

    // Build DFS Queue
    dfsNodesQueue = buildDfsQueue(simplifiedAst);
    currentTraversalIndex = -1;
    updateTraversalBarUI();

    // Set up SVG container dimensions
    const width = container.clientWidth || 400;
    const height = container.clientHeight || 500;

    // Create SVG element
    const svg = d3.select(container)
        .append("svg")
        .attr("width", "100%")
        .attr("height", "100%")
        .attr("class", "w-full h-full relative");

    d3Svg = svg;

    // Create main container group for pan and zoom
    const g = svg.append("g");
    d3G = g;

    // Define Zoom behavior
    d3ZoomBehavior = d3.zoom()
        .scaleExtent([0.1, 3])
        .on("zoom", (event) => {
            g.attr("transform", event.transform);
        });

    svg.call(d3ZoomBehavior);

    // Create D3 Hierarchy structure
    const root = d3.hierarchy(simplifiedAst);

    // Compute D3 Tree Layout (horizontal separation 460px, vertical separation 300px)
    const treeLayout = d3.tree().nodeSize([460, 300]);
    treeLayout(root);

    // Store each node's D3 layout coordinates indexed by node id
    d3NodeCoords = {};
    root.descendants().forEach(d => {
        d3NodeCoords[d.data.id] = { x: d.x, y: d.y };
    });

    // Center root node initially at the top center
    const initialTransform = d3.zoomIdentity.translate(width / 2, 60).scale(0.85);
    svg.call(d3ZoomBehavior.transform, initialTransform);

    // Draw links (connector lines)
    const links = g.selectAll(".ast-link")
        .data(root.links())
        .enter()
        .append("path")
        .attr("class", "ast-link")
        .attr("d", d3.linkVertical()
            .x(d => d.x)
            .y(d => d.y)
        )
        .attr("id", d => `link-${d.source.data.id}-${d.target.data.id}`);

    // Draw nodes
    const nodes = g.selectAll(".ast-node")
        .data(root.descendants())
        .enter()
        .append("g")
        .attr("class", "ast-node")
        .attr("id", d => d.data.id)
        .attr("transform", d => `translate(${d.x},${d.y})`)
        .on("click", (event, d) => {
            event.stopPropagation();
            focusNode(d.data.id);
        });

    // Embedded HTML Card within each node group using foreignObject (bounds expanded to fit 440x280 highlighted cards)
    const fo = nodes.append("foreignObject")
        .attr("width", 480)
        .attr("height", 320)
        .attr("x", -240)
        .attr("y", -160);

    fo.html(d => {
        const style = NODE_TYPE_STYLING[d.data.nodeType] || {
            icon: 'help-circle',
            color: 'text-indigo-300 font-extrabold',
            name: d.data.nodeType.replace("Node", ""),
            border: 'border-indigo-500/40 border-2',
            bg: 'bg-indigo-500/10'
        };

        const parentLabel = d.data.label
            ? `<span class="px-2 py-0.5 rounded text-[9px] font-bold uppercase tracking-wider bg-slate-900/90 text-indigo-300 border border-slate-800 w-max leading-none select-none">${d.data.label}</span>`
            : '';

        const val = d.data.value;
        const valText = (val !== null && val !== undefined) ? (typeof val === 'string' ? `"${val}"` : val) : "";
        const valDiv = valText !== ""
            ? `<div class="ast-node-val text-sm text-emerald-300 font-bold font-mono mt-2.5 px-3 py-2 rounded-lg bg-slate-950/80 border border-emerald-500/35 truncate select-all shadow-inner">${escapeHtml(valText)}</div>`
            : "";

        const explanation = NODE_EXPLANATIONS[d.data.nodeType] || 'A structural node in the program AST.';

        const detailContent = `
            <div class="mt-3 text-[13px] text-slate-200 border-t border-slate-700/80 pt-3 flex flex-col space-y-2 overflow-y-auto max-h-[175px] font-sans">
                <div class="flex justify-between items-center"><span class="font-black text-slate-350">Node Class:</span> <span class="font-mono text-indigo-200 font-bold bg-indigo-950/70 px-2.5 py-0.5 rounded border border-indigo-700/60">${d.data.nodeType}</span></div>
                ${d.data.pos_start ? `<div class="flex justify-between items-center"><span class="font-black text-slate-350">Code Range:</span> <span class="font-mono text-slate-100 bg-slate-900/80 px-2.5 py-0.5 rounded border border-slate-700">L${d.data.pos_start.line + 1}:${d.data.pos_start.col + 1} - L${d.data.pos_end.line + 1}:${d.data.pos_end.col + 1}</span></div>` : ''}
                ${valText !== "" ? `<div class="flex justify-between items-center"><span class="font-black text-slate-350">Value:</span> <span class="font-mono text-emerald-300 bg-emerald-950/50 px-2.5 py-0.5 rounded border border-emerald-700/60 truncate max-w-[200px]">${escapeHtml(valText)}</span></div>` : ''}
                <div class="flex justify-between items-center"><span class="font-black text-slate-350">Children:</span> <span class="font-bold text-slate-100 bg-slate-900/80 px-2.5 py-0.5 rounded border border-slate-700">${(d.data.children ? d.data.children.length : 0) + (d.data._children ? d.data._children.length : 0)}</span></div>
                <div class="text-[12px] text-slate-100 bg-slate-950/80 p-2.5 rounded-lg border border-slate-700/80 mt-1.5 italic leading-relaxed select-text font-medium">${explanation}</div>
            </div>
        `;

        const hasChildren = (d.data.children && d.data.children.length > 0) || (d.data._children && d.data._children.length > 0);
        const isCollapsed = !!d.data._children;
        const toggleButton = hasChildren
            ? `<button class="ast-node-collapse-btn ml-1.5 px-2.5 py-1 rounded bg-slate-900/80 text-[11px] font-bold text-slate-200 hover:text-white border border-slate-700 hover:border-slate-650 hover:bg-indigo-950/30 transition-all flex items-center space-x-1" onclick="event.stopPropagation(); window.toggleNodeCollapse('${d.data.stableId}')">
                <i data-lucide="${isCollapsed ? 'chevron-down' : 'chevron-up'}" class="h-3.5 w-3.5 shrink-0"></i>
                <span>${isCollapsed ? 'Expand' : 'Collapse'}</span>
               </button>`
            : '';

        return `
            <div class="ast-fo-wrapper w-full h-full flex items-center justify-center pointer-events-none">
                <div class="ast-node-card pointer-events-auto rounded-xl border ${style.border} ${style.bg} text-left p-3.5 select-none flex flex-col justify-start backdrop-blur-md shadow-lg transition-all">
                    <div class="flex items-center justify-between w-full">
                        <div class="flex items-center space-x-2 truncate">
                            <i data-lucide="${style.icon}" class="h-5.5 w-5.5 ${style.color} shrink-0"></i>
                            <span class="text-[15px] font-black text-white truncate font-sans tracking-wide">${style.name}</span>
                        </div>
                        <div class="flex items-center space-x-1.5 shrink-0">
                            ${parentLabel}
                            ${toggleButton}
                        </div>
                    </div>
                    
                    ${valDiv}
                    
                    <div class="ast-node-details">
                        ${detailContent}
                    </div>
                </div>
            </div>
        `;
    });

    // Clicking outside on the canvas resets highlight/muting
    svg.on("click", () => {
        resetTraversal();
    });

    // Create SVG Icons via Lucide in foreignObjects
    setTimeout(() => {
        lucide.createIcons();
    }, 10);
}

function renderAstError(err) {
    latestAst = null;
    const container = document.getElementById('edu-ast-tree-container');
    if (!container) return;

    const traversalBar = document.getElementById('edu-ast-traversal-bar');
    if (traversalBar) traversalBar.classList.add('hidden');

    container.innerHTML = `
        <div class="p-3 rounded-lg bg-rose-950/30 border border-rose-900/50 text-rose-300 font-sans text-xs">
            <div class="font-bold uppercase tracking-wider flex items-center space-x-1.5 text-rose-400">
                <i data-lucide="alert-triangle" class="h-4 w-4"></i>
                <span>Parser / Lexer Error</span>
            </div>
            <div class="mt-2 font-mono text-[11px] font-semibold">${err.type || 'Error'}</div>
            <div class="mt-1 text-slate-350">${err.details || ''}</div>
            ${err.pos_start ? `<div class="mt-2 text-[10px] text-rose-450 text-rose-400/80 font-mono">Line ${err.pos_start.line}, Col ${err.pos_start.col}</div>` : ''}
        </div>
    `;
    lucide.createIcons();
}
