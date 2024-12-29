import os
import subprocess

def run_tests(directory):
    # Get all .py files in the directory, sort them alphabetically, and skip 'tutorial_template.py'
    test_files = sorted(
        f for f in os.listdir(directory) if f.endswith(".py") and f != "tutorial_template.py"
    )

    # Run each test file
    for test_file in test_files:
        print(f"Running {test_file}...")
        test_path = os.path.join(directory, test_file)
        result = subprocess.run(["python3", test_path], capture_output=True, text=True)

        # Print the results
        print(f"Output of {test_file}:\n{result.stdout}")
        if result.returncode != 0:
            print(f"Error in {test_file}:\n{result.stderr}")
            print("--------------------------------------------------")
            print("Test failed.")
            return  # Stop batch execution if a test fails
        else:
            print("Test passed!")
            print("--------------------------------------------------")

    print("All tests completed successfully!")

if __name__ == "__main__":
    run_tests("../tutorials")
