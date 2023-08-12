#pragma once

#include <boost/property_tree/ptree.hpp>

#include <string>
#include <vector>

namespace pt = boost ::property_tree;

struct Test_result
{
    std::string name{};
    std::string status{};
    std::string message{};
    std::string output{};
    std::string test_code{};
    int task_id{};
    void add_result_to_json(std::ofstream &json_file) const;
};

struct Output_message
{
    int version{3};
    std::string status{};
    std::string message{};
    std::vector<Test_result> tests{};
    void load_from_catch2_xml(const std::string &xml_test_output_file, const std::string &compilation_error_file);
    void save_as_exercism_json(const std::string &filename);
    std::string build_test_message(const pt::ptree &tree);
};