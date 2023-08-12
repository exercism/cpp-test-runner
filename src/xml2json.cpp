#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>

#include <exception>
#include <iostream>
#include <string_view>

#include "helpers.h"
#include "xml2json.h"

std::string escape_line_breaks(std::string text)
{
    boost::replace_all(text, "\n", "\\n");
    return text;
}

void Output_message::load_from_catch2_xml(const std::string &xml_test_output_file, const std::string &compilation_error_file)
{
    // Create empty property tree object
    pt::ptree tree;

    // Parse the XML into the property tree.
    try
    {
        pt::read_xml(xml_test_output_file, tree);

        // Check if all test have succeded
        int failures = tree.get<int>("Catch2TestRun.OverallResultsCases.<xmlattr>.failures");
        if (failures == 0)
        {
            status = "pass";
        }
        else
        {
            status = "fail";
        }

        for (const auto &test_case : tree.get_child("Catch2TestRun"))
        {
            if (test_case.first != "TestCase")
                continue;

            std::string name{test_case.second.get<std::string>("<xmlattr>.name")};
            std::string status{"pass"};
            std::string message{""};
            if (!test_case.second.get<bool>("OverallResult.<xmlattr>.success"))
            {
                status = "fail";
                message = build_test_message(test_case.second);
            }

            int task_id{0};
            try
            {
                std::string tags{test_case.second.get<std::string>("<xmlattr>.tags")};
                size_t found = tags.find("task_"); // TODO: check if multiple tags have different listings and break the method
                if (found != std::string::npos)
                {
                    task_id = tags[found + 5] - '0';
                    // check if it is a double-digit number:
                    char possible_second_digit{tags[found + 6]};
                    if (possible_second_digit >= '0' && possible_second_digit <= '9')
                    {
                        task_id *= 10;
                        task_id += possible_second_digit - '0';
                    }
                }
            }
            catch (...)
            {
            }

            Test_result result{
                name,
                status,
                message,
                "", // TODO: Make output
                "", // TODO: Make test_code
                task_id};
            tests.emplace_back(result);
        }
    }
    catch (std::exception &e)
    {
        // Checks if xml output file exists, in case it doesn't
        // compilation must have failed and tests can't be run.
        status = "error";
        message = read_file(compilation_error_file);
    }
}

std::string to_json_pair(const std::string &name, const std::string &value, bool first_item = false)
{
    if (value != "")
    {
        return (first_item ? "\"" : ", \"") + name + "\": \"" + escape_line_breaks(value) + "\"";
    }
    return "";
}

std::string to_json_pair(const std::string &name, int value, bool first_item = false)
{
    if (value != 0)
    {
        return (first_item ? "\"" : ", \"") + name + "\": " + std::to_string(value);
    }
    return "";
}

void Output_message::save_as_exercism_json(const std::string &filename)
{
    // Boost's JSON export from ptree can only write strings, not pure int as values.
    // Thus, this is the manal way:
    std::ofstream json_file(filename);
    if (json_file.is_open())
    {
        // Write basics
        json_file << "{"
                  << to_json_pair("version", version, true)
                  << to_json_pair("status", status);
        if (status != "pass")
        {
            json_file << to_json_pair("message", message);
        }

        // Write test results:
        if (status != "error")
        {
            json_file << ", "
                      << "\"tests\": [";
            std::size_t test_counter{};
            for (const auto &result : tests)
            {
                result.add_result_to_json(json_file);
                if (tests.size() > ++test_counter)
                    json_file << ", ";
            }
            json_file << "]";
        }
        json_file << "}";
        json_file.close();
    }
}

std::string Output_message::build_test_message(const pt::ptree &tree)
{
    // V2 template:
    // \nFAILED:\n  REQUIRE( leap::is_leap_year(2000) )\nwith expansion:\n  false\nat /tmp/example-partial-fail/example_partial_fail_test.cpp:31\n
    std::string testing_type{tree.get<std::string>("Expression.<xmlattr>.type")};
    std::string original{trim_copy(tree.get<std::string>("Expression.Original"))};
    std::string expanded{trim_copy(tree.get<std::string>("Expression.Expanded"))};
    std::string filename{tree.get<std::string>("Expression.<xmlattr>.filename")};
    std::string line{tree.get<std::string>("Expression.<xmlattr>.line")};

    return "\nFAILED:\n  " + testing_type + "( " + original + " )\nwith expansion:\n  " + expanded + "\nat " + filename + ":" + line + "\n";
}

void Test_result::add_result_to_json(std::ofstream &json_file) const
{
    json_file << "{"
              << to_json_pair("name", name, true)
              << to_json_pair("status", status);
    if (status != "pass")
    {
        json_file << to_json_pair("message", message)
                  << to_json_pair("output", output)
                  << to_json_pair("test_code", test_code);
    }
    json_file << to_json_pair("task_id", task_id);
    json_file << "}";
}
