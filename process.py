import sys
import json

import junitparser


if __name__ == "__main__":
    input_file: str = sys.argv[1]
    output_file: str = sys.argv[2]

    output = {}
    xml = junitparser.JUnitXml.fromfile(input_file)
    for suite in xml:
        status = "pass"
        if suite.errors:
            status = "error"
        elif suite.failures:
            status = "fail"
        output["status"] = status

        # TODO: Set top level message on error

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

        with open(output_file, "w") as f:
            f.write(json.dumps(output))
