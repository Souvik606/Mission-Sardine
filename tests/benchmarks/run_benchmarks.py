import os
import sys
import time
import subprocess
import json
import ctypes
from ctypes import wintypes

# Windows API definitions for Peak Working Set Size
class PROCESS_MEMORY_COUNTERS(ctypes.Structure):
    _fields_ = [
        ('cb', wintypes.DWORD),
        ('PageFaultCount', wintypes.DWORD),
        ('PeakWorkingSetSize', ctypes.c_size_t),
        ('WorkingSetSize', ctypes.c_size_t),
        ('QuotaPeakPagedPoolUsage', ctypes.c_size_t),
        ('QuotaPagedPoolUsage', ctypes.c_size_t),
        ('QuotaPeakNonPagedPoolUsage', ctypes.c_size_t),
        ('QuotaNonPagedPoolUsage', ctypes.c_size_t),
        ('PagefileUsage', ctypes.c_size_t),
        ('PeakPagefileUsage', ctypes.c_size_t),
    ]

def get_peak_memory_windows(handle):
    try:
        counters = PROCESS_MEMORY_COUNTERS()
        counters.cb = ctypes.sizeof(PROCESS_MEMORY_COUNTERS)
        psapi = ctypes.WinDLL('psapi')
        if psapi.GetProcessMemoryInfo(handle, ctypes.byref(counters), counters.cb):
            return counters.PeakWorkingSetSize
    except Exception:
        pass
    return 0

try:
    import resource
except ImportError:
    resource = None

def run_process(cmd, env=None):
    # Capture resource usage if on Unix
    ru_before = None
    if resource:
        ru_before = resource.getrusage(resource.RUSAGE_CHILDREN)
        
    start_time = time.perf_counter()
    
    # We use creationflags to prevent popup windows if running GUI/detached, but default is fine
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=env
    )
    
    # Wait for process to complete
    stdout, stderr = proc.communicate()
    end_time = time.perf_counter()
    
    wall_ms = (end_time - start_time) * 1000.0
    
    # Capture peak memory footprint
    peak_bytes = 0
    if sys.platform == 'win32':
        handle = getattr(proc, '_handle', None)
        if handle:
            peak_bytes = get_peak_memory_windows(handle)
    elif resource and ru_before:
        ru_after = resource.getrusage(resource.RUSAGE_CHILDREN)
        diff = ru_after.ru_maxrss - ru_before.ru_maxrss
        if sys.platform == 'darwin': # macOS returns bytes
            peak_bytes = diff
        else: # Linux/Unix returns KB
            peak_bytes = diff * 1024
            
    return proc.returncode, stdout, stderr, wall_ms, peak_bytes

def main():
    benchmarks_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(benchmarks_dir, "..", ".."))
    
    # Find the Sardine executable
    sards_exec = os.path.join(project_root, "build", "MissionSardine.exe")
    if not os.path.exists(sards_exec):
        sards_exec = os.path.join(project_root, "build", "MissionSardine")
        
    if not os.path.exists(sards_exec):
        print(f"Error: Could not find MissionSardine executable at {sards_exec}.")
        print("Please compile the project first.")
        sys.exit(1)
        
    python_exec = sys.executable
    
    benchmarks = [
        {"name": "Recursive Fibonacci", "file": "fib"},
        {"name": "Prime Sieve", "file": "sieve"},
        {"name": "Mandelbrot Fractal", "file": "mandelbrot"},
        {"name": "Fannkuch Permutations", "file": "fannkuch"},
    ]
    
    runs = 3
    print(f"Starting Benchmark Suite (Running each program {runs} times for average)...")
    print(f"Sardine Executable: {sards_exec}")
    print(f"Python Executable:  {python_exec}\n")
    
    results = []
    
    for bench in benchmarks:
        name = bench["name"]
        file_prefix = bench["file"]
        
        sad_path = os.path.join(benchmarks_dir, file_prefix, f"{file_prefix}.sad")
        py_path = os.path.join(benchmarks_dir, file_prefix, f"{file_prefix}.py")
        
        print(f"Benchmarking {name}...")
        
        # 1. Run Sardine
        sad_wall_times = []
        sad_memories = []
        lex_times = []
        parse_times = []
        interpret_times = []
        
        # Setup environment path for MinGW DLLs on Windows
        env = os.environ.copy()
        if sys.platform == 'win32':
            env["PATH"] = "C:\\mingw64\\bin;" + env.get("PATH", "")
            
        for r in range(runs):
            code, out, err, wall_ms, mem_bytes = run_process(
                [sards_exec, "--unbounded", "--profile", sad_path],
                env=env
            )
            if code != 0:
                print(f"  [ERROR] Sardine failed to run {sad_path}. Return code: {code}")
                print(f"  Stderr:\n{err}")
                break
                
            sad_wall_times.append(wall_ms)
            sad_memories.append(mem_bytes / (1024.0 * 1024.0)) # Convert to MB
            
            # Parse timing breakdown from stderr JSON line
            # We look for a line starting with {"profile"
            lex_ms, parse_ms, interpret_ms = 0.0, 0.0, 0.0
            for line in err.splitlines():
                if line.strip().startswith('{"profile"'):
                    try:
                        data = json.loads(line)
                        profile = data["profile"]
                        lex_ms = profile.get("lex_ms", 0.0)
                        parse_ms = profile.get("parse_ms", 0.0)
                        interpret_ms = profile.get("interpret_ms", 0.0)
                    except Exception:
                        pass
            lex_times.append(lex_ms)
            parse_times.append(parse_ms)
            interpret_times.append(interpret_ms)
            
        # 2. Run Python
        py_wall_times = []
        py_memories = []
        
        for r in range(runs):
            code, out, err, wall_ms, mem_bytes = run_process([python_exec, py_path])
            if code != 0:
                print(f"  [ERROR] Python failed to run {py_path}. Return code: {code}")
                print(f"  Stderr:\n{err}")
                break
            py_wall_times.append(wall_ms)
            py_memories.append(mem_bytes / (1024.0 * 1024.0)) # Convert to MB
            
        if len(sad_wall_times) < runs or len(py_wall_times) < runs:
            print(f"Skipping {name} due to execution errors.")
            continue
            
        # Compute Averages
        avg_sad_time = sum(sad_wall_times) / runs
        avg_sad_mem = sum(sad_memories) / runs
        avg_lex = sum(lex_times) / runs
        avg_parse = sum(parse_times) / runs
        avg_interpret = sum(interpret_times) / runs
        
        avg_py_time = sum(py_wall_times) / runs
        avg_py_mem = sum(py_memories) / runs
        
        time_ratio = avg_sad_time / avg_py_time if avg_py_time > 0 else 0.0
        mem_ratio = avg_sad_mem / avg_py_mem if avg_py_mem > 0 else 0.0
        
        results.append({
            "name": name,
            "sad_time": avg_sad_time / 1000.0, # in seconds
            "sad_mem": avg_sad_mem,
            "py_time": avg_py_time / 1000.0, # in seconds
            "py_mem": avg_py_mem,
            "lex": avg_lex,
            "parse": avg_parse,
            "interpret": avg_interpret,
            "time_ratio": time_ratio,
            "mem_ratio": mem_ratio
        })
        
    # Print results table
    print("\n" + "=" * 80)
    print("BENCHMARK RESULTS SUMMARY")
    print("=" * 80 + "\n")
    
    headers = ["Benchmark", "Language", "Time (s)", "Memory (MB)", "Lex (ms)", "Parse (ms)", "Interpret (ms)", "Sardine/Python Ratio"]
    print(f"| {' | '.join(headers)} |")
    print(f"|{'|'.join(['---' for _ in headers])}|")
    
    markdown_lines = []
    markdown_lines.append(f"| {' | '.join(headers)} |")
    markdown_lines.append(f"|{'|'.join(['---' for _ in headers])}|")
    
    for r in results:
        # Sardine row
        sad_row = [
            r["name"],
            "Sardine",
            f"{r['sad_time']:.4f}s",
            f"{r['sad_mem']:.2f} MB",
            f"{r['lex']:.2f} ms",
            f"{r['parse']:.2f} ms",
            f"{r['interpret']:.2f} ms",
            f"{r['time_ratio']:.2f}x slower"
        ]
        # Python row
        py_row = [
            "",
            "Python",
            f"{r['py_time']:.4f}s",
            f"{r['py_mem']:.2f} MB",
            "-",
            "-",
            "-",
            "-"
        ]
        
        line_sad = f"| {' | '.join(sad_row)} |"
        line_py = f"| {' | '.join(py_row)} |"
        
        print(line_sad)
        print(line_py)
        markdown_lines.append(line_sad)
        markdown_lines.append(line_py)
        
    # Save results to markdown file in the benchmarks directory
    results_file = os.path.join(benchmarks_dir, "benchmark_results.md")
    with open(results_file, "w") as f:
        f.write("# Mission Sardine Benchmark Results\n\n")
        f.write("\n".join(markdown_lines))
        f.write("\n")
        
    print(f"\nBenchmark results saved to {results_file}")

if __name__ == "__main__":
    main()
