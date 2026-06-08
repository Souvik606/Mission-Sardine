import os
import subprocess
import sys

def run_tests():
    tests_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'tests'))
    run_test_js = os.path.abspath(os.path.join(os.path.dirname(__file__), 'run_test.js'))
    
    passed_count = 0
    failed_count = 0

    for root, _, files in os.walk(tests_dir):
        for file in files:
            if file.endswith('.sad'):
                test_path = os.path.join(root, file)
                output_path = test_path.replace('.sad', '.out')
                input_path = test_path.replace('.sad', '.in')

                if not os.path.exists(output_path):
                    continue
                
                cmd = ['node', run_test_js, test_path]
                if os.path.exists(input_path):
                    cmd.append(input_path)

                try:
                    result = subprocess.run(
                        cmd,
                        cwd=os.path.dirname(run_test_js),
                        capture_output=True,
                        text=True,
                        check=True
                    )
                    actual_output = result.stdout.rstrip().replace('\r\n', '\n')
                    with open(output_path, 'r') as f:
                        expected_output = f.read().rstrip().replace('\r\n', '\n')

                    import re
                    def normalize_output(text):
                        # Remove "Error in ...:" header lines (only print by native launcher)
                        lines = [line for line in text.split('\n') if not line.startswith('Error in ') and not line.strip().startswith('Error in ')]
                        text = '\n'.join(lines)
                        # Normalize File "...", line X traceback headers to File <stdin>, line X
                        text = re.sub(r'File [^,]+, line (\d+)', r'File <stdin>, line \1', text)
                        # Remove folder prefix of the test path in any other searched paths
                        test_dir_rel = os.path.relpath(os.path.dirname(test_path), os.path.join(os.path.dirname(__file__), '..')).replace('\\', '/')
                        if test_dir_rel != '.':
                            text = text.replace(test_dir_rel + '/', '')
                        return text

                    actual_norm = normalize_output(actual_output)
                    expected_norm = normalize_output(expected_output)

                    if actual_norm == expected_norm:
                        print(f"WASM PASS: {os.path.relpath(test_path, tests_dir)}")
                        passed_count += 1
                    else:
                        print(f"WASM FAIL: {os.path.relpath(test_path, tests_dir)}")
                        print("Expected:")
                        print(expected_norm)
                        print("Got:")
                        print(actual_norm)
                        failed_count += 1
                except subprocess.CalledProcessError as e:
                    print(f"WASM ERROR: {os.path.relpath(test_path, tests_dir)} failed to run.")
                    print(e.stderr)
                    failed_count += 1

    print("\n--- WASM Test Summary ---")
    print(f"Passed: {passed_count}")
    print(f"Failed: {failed_count}")

    if failed_count > 0:
        sys.exit(1)

if __name__ == "__main__":
    run_tests()
