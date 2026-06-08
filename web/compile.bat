@echo off
echo Compiling Mission-Sardine to WebAssembly...
d:\Github\wasm-test\emsdk\upstream\emscripten\emcc.bat shell.cpp data_types/data_type.cpp data_types/user_function_type.cpp data_types/model_type.cpp -o web/interpreter.js -Iweb/include -std=c++17 -O3 -s EXPORTED_RUNTIME_METHODS="['ccall','FS']" -s EXPORTED_FUNCTIONS="['_run_interpreter']" -s FORCE_FILESYSTEM=1 -s DISABLE_EXCEPTION_CATCHING=0 -s ALLOW_MEMORY_GROWTH=1 -s STACK_SIZE=4194304 --embed-file stdlib@/stdlib
if %errorlevel% neq 0 (
    echo Compilation failed!
    exit /b %errorlevel%
)
echo Compilation successful! Generated web/interpreter.js and web/interpreter.wasm
