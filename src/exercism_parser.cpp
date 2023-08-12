#include <iostream>
#include "xml2json.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "Input, output, and compilation errors file names are required but were not supplied.\n"
                  << "Usage: " << argv[0] << " input_file_name  output_file_name compilation_errors_file_name\n";
        return -1;
    }

    std::string input_file_file_name_path{argv[1]};
    std::string output_file_file_name_path{argv[2]};
    std::string compilation_errors_file_name_path{argv[3]};

    try
    {
        Output_message om{};
        om.load_from_catch2_xml(input_file_file_name_path, compilation_errors_file_name_path);
        om.save_as_exercism_json(output_file_file_name_path);
    }
    catch (std::exception &e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }
    return 0;
}