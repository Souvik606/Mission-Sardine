let sharedBuffer = null;
let sharedInt32 = null;
let sharedUint8 = null;

let inputBuffer = [];
let inputBufferIndex = 0;
let hasReturnedNewline = false;

let isCollectingJson = false;
let jsonBuffer = [];

// Intercept Emscripten's stdout/stderr before loading the runtime module
var Module = {
  print: function (text) {
    const trimmed = typeof text === 'string' ? text.trim() : '';
    if (trimmed === "--- EDUCATIONAL_MODE_OUTPUT_START ---") {
      isCollectingJson = true;
      jsonBuffer = [];
      return;
    }
    if (trimmed === "--- EDUCATIONAL_MODE_OUTPUT_END ---") {
      isCollectingJson = false;
      try {
        const jsonStr = jsonBuffer.join("\n");
        const parsedJson = JSON.parse(jsonStr);
        postMessage({ type: "educational_json", data: parsedJson });
      } catch (e) {
        console.error("Failed to parse educational json:", e);
      }
      return;
    }
    if (isCollectingJson) {
      jsonBuffer.push(text);
      return;
    }
    postMessage({ type: "stdout", data: text });
  },
  printErr: function (text) {
    postMessage({ type: "stderr", data: text });
  },
  onRuntimeInitialized: function () {
    postMessage({ type: "ready" });
  },
  locateFile: function (path, prefix) {
    if (path.endsWith(".wasm")) {
      return prefix + path + "?v=" + Date.now();
    }
    return prefix + path;
  },
  preRun: [function() {
    function stdinCallback() {
      // If we have characters in our local buffer, return them first
      if (inputBufferIndex < inputBuffer.length) {
        const char = inputBuffer[inputBufferIndex++];
        if (char === 10) { // 10 is '\n'
          hasReturnedNewline = true;
        }
        return char;
      }

      // Check if we have the shared buffer initialized
      if (!sharedInt32) {
        return null; // Fallback: EOF
      }

      if (hasReturnedNewline) {
        hasReturnedNewline = false;
        return null; // EOF for the current read chunk
      }

      // Reset the ready flag (index 0) to 0 (waiting)
      Atomics.store(sharedInt32, 0, 0);

      // Notify the main thread that we need input
      postMessage({ type: "request_input" });

      // Block the worker thread until the main thread changes index 0 of sharedInt32 to 1
      Atomics.wait(sharedInt32, 0, 0);

      // Read length of string from index 1
      const length = sharedInt32[1];
      inputBuffer = [];
      for (let i = 0; i < length; i++) {
        inputBuffer.push(sharedUint8[i]);
      }
      inputBufferIndex = 0;

      if (inputBuffer.length === 0) {
        return null; // EOF
      }
      
      const char = inputBuffer[inputBufferIndex++];
      if (char === 10) { // 10 is '\n'
        hasReturnedNewline = true;
      }
      return char;
    }

    // Initialize FS with our custom stdin
    if (typeof FS !== 'undefined') {
      FS.init(stdinCallback, null, null);
    } else if (Module.FS) {
      Module.FS.init(stdinCallback, null, null);
    }
  }],
};

// Listen for messages from the Main UI Thread
onmessage = function (e) {
  const { type, code, unbounded, educational, sharedBuffer: sb } = e.data;

  if (type === "init") {
    sharedBuffer = sb;
    sharedInt32 = new Int32Array(sharedBuffer);
    sharedUint8 = new Uint8Array(sharedBuffer, 8); // String data starts at offset 8
  } else if (type === "load") {
    try {
      importScripts("interpreter.js?v=" + Date.now());
    } catch (err) {
      postMessage({
        type: "error",
        data: "Failed to load WASM script: " + err.message,
      });
    }
  } else if (type === "run") {
    if (typeof Module.ccall === "function") {
      try {
        // Clear any leftover stdin buffer
        inputBuffer = [];
        inputBufferIndex = 0;
        hasReturnedNewline = false;
        isCollectingJson = false;
        jsonBuffer = [];

        // Sync files to WebAssembly virtual filesystem (MEMFS)
        if (e.data.files && Module.FS) {
          try {
            // Delete old files to prevent deleted files from persisting across runs
            let existing = Module.FS.readdir('/');
            existing.forEach(name => {
              if (name !== '.' && name !== '..' && name !== 'dev' && name !== 'proc' && name !== 'tmp' && name !== 'stdlib') {
                let path = '/' + name;
                let stat = Module.FS.stat(path);
                if (Module.FS.isFile(stat.mode)) {
                  Module.FS.unlink(path);
                }
              }
            });
          } catch (cleanErr) {
            console.error("Failed to clean MEMFS before execution:", cleanErr);
          }

          // Write fresh files
          for (let filename in e.data.files) {
            try {
              Module.FS.writeFile('/' + filename, e.data.files[filename]);
            } catch (writeErr) {
              console.error("Failed to write to MEMFS:", filename, writeErr);
            }
          }
        }

        // Execute the interpreter entrypoint
        Module.ccall(
          "run_interpreter",
          null,
          ["string", "string", "number", "number"],
          [code, "<stdin>", unbounded ? 1 : 0, educational ? 1 : 0]
        );

        // Sync MEMFS files
        let sandboxFiles = {};
        if (Module.FS) {
          try {
            // Read root directory '/'
            let files = Module.FS.readdir('/');
            files.forEach(name => {
              // Exclude default system folders/files and stdlib
              if (name !== '.' && name !== '..' && name !== 'dev' && name !== 'proc' && name !== 'tmp' && name !== 'stdlib') {
                let path = '/' + name;
                let stat = Module.FS.stat(path);
                if (Module.FS.isFile(stat.mode)) {
                  let content = Module.FS.readFile(path, { encoding: 'utf8' });
                  sandboxFiles[name] = content;
                }
              }
            });
          } catch (fsErr) {
            console.error("Failed to read virtual files:", fsErr);
          }
        }

        // Notify main thread done, sending files payload
        postMessage({ type: "done", files: sandboxFiles });
      } catch (err) {
        postMessage({
          type: "stderr",
          data: "Runtime exception: " + err.message,
        });
        postMessage({ type: "done", files: {} });
      }
    }
  }
};
