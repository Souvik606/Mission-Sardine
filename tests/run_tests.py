
import os
import subprocess
import sys

def run_tests():
    tests_dir = os.path.dirname(__file__)
    sards_executable = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', 'MissionSardine.exe'))
    if not os.path.exists(sards_executable):
        sards_executable = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', 'MissionSardine'))
    
    passed_count = 0
    failed_count = 0

    for root, _, files in os.walk(tests_dir):
        for file in files:
            if file.endswith('.sad'):
                test_path = os.path.join(root, file)
                output_path = test_path.replace('.sad', '.out')
                input_path = test_path.replace('.sad', '.in')

                if not os.path.exists(output_path):
                    print(f"SKIPPING: No .out file for {test_path}")
                    continue
                
                input_data = None
                if os.path.exists(input_path):
                    with open(input_path, 'r') as f:
                        input_data = f.read()

                try:
                    # Run the .sad file using the compiled C++ executable
                    env = os.environ.copy()
                    env["PATH"] = "C:\\mingw64\\bin;" + env.get("PATH", "")
                    result = subprocess.run(
                        [sards_executable, test_path],
                        capture_output=True,
                        text=True,
                        check=True,
                        input=input_data,
                        env=env
                    )
                    actual_output = result.stdout.strip().replace('\r\n', '\n')

                    with open(output_path, 'r') as f:
                        expected_output = f.read().strip().replace('\r\n', '\n')

                    if actual_output == expected_output:
                        print(f"PASS: {test_path}")
                        passed_count += 1
                    else:
                        print(f"FAIL: {test_path}")
                        print("Expected:")
                        print(expected_output)
                        print("Got:")
                        print(actual_output)
                        failed_count += 1
                except subprocess.CalledProcessError as e:
                    print(f"ERROR: {test_path} failed to run.")
                    print(e.stderr.strip().replace('\r\n', '\n'))
                    failed_count += 1
                except FileNotFoundError:
                    print(f"ERROR: Could not find {sards_executable}. Make sure it is compiled in build/ directory.")
                    sys.exit(1)

    print("\n--- Test Summary ---")
    print(f"Passed: {passed_count}")
    print(f"Failed: {failed_count}")

    if failed_count > 0:
        sys.exit(1)

if __name__ == "__main__":
    run_tests()
