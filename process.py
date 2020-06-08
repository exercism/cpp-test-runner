import sys
import os
import json
from typing import Dict

import junitparser


def process_tests_results(compilation_errors_file: str, tests_output_file: str) -> Dict:
    output = {}

    # Checks if JUnit xml output file exists, in case it doesn't
    # compilation must have failed and tests can't be run.
    if not os.path.isfile(tests_output_file):
        output["status"] = "error"
        with open(compilation_errors_file, "r") as f:
            output["message"] = f.read()
        output["tests"] = []
        return output

    xml = junitparser.JUnitXml.fromfile(tests_output_file)
    for suite in xml:
        status = "pass"
        if suite.errors:
            status = "error"
        elif suite.failures:
            status = "fail"
        output["status"] = status

        tests = []
        for test in suite:
            test_data = {"name": test.name, "status": "pass"}
            # Sets test status and message in case of failure or error
            if (t := test.result) == junitparser.Failure:
                test_data["status"] = "fail"
                test_data["message"] = t.message
            elif (t := test.result) == junitparser.Error:
                test_data["status"] = "fail"
                test_data["message"] = t.message

            if out := test.system_out:
                # Output MUST be limited at 500 chars
                if len(out) > 500:
                    # Truncation message is counted toward the 500 limit,
                    # so we truncate even more chars
                    out = (
                        f"{out[:452]}\nOutput was truncated. Please limit to 500 chars"
                    )
                test_data["output"] = out

            tests.append(test_data)
        output["tests"] = tests

    return output


if __name__ == "__main__":
    output: Dict = process_tests_results(sys.argv[1], sys.argv[2])
    output_file: str = sys.argv[3]

    with open(output_file, "w") as f:
        f.write(json.dumps(output))
