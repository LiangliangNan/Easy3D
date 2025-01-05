import os
import subprocess

def run_tests(directory):
    # Get all .py files in the directory, sort them alphabetically, and skip 'tutorial_template.py'
    test_files = sorted(
        f for f in os.listdir(directory) if f.endswith(".py") and f != "tutorial_template.py"
    )

    # Run each test file
    for test_file in test_files:
        print(f"Running {test_file}...", flush=True)  # Immediate flush
        test_path = os.path.join(directory, test_file)

        # Set unbuffered mode by using PYTHONUNBUFFERED environment variable
        env = os.environ.copy()
        env["PYTHONUNBUFFERED"] = "1"  # Forces Python to flush immediately

        # Execute the script and stream its output to the console
        process = subprocess.Popen(
            ["python3", test_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            env=env,  # Pass the modified environment
        )

        # Stream the output line by line, flushing immediately
        for line in process.stdout:
            print(line, end='', flush=True)  # Print and flush each line immediately

        for line in process.stderr:
            print(line, end='', flush=True)  # Print and flush each line immediately

        process.wait()  # Wait for the process to finish

        # Check the return code to determine if the script succeeded
        if process.returncode != 0:
            print(f"Error in {test_file}. Test failed.", flush=True)
            print("--------------------------------------------------", flush=True)
            return  # Stop batch execution if a test fails
        else:
            print("Test passed!", flush=True)
            print("--------------------------------------------------", flush=True)

    print("All tests completed successfully!", flush=True)

if __name__ == "__main__":
    run_tests("../tutorials")
