#pragma once

#ifndef __TEXT_REPORT_GENERATOR_HPP__
#define __TEXT_REPORT_GENERATOR_HPP__

#include <iostream>

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

class TextReportGenerator {
public:
    void output(std::ostream& f, const json& report, bool show_region_depth);
    void output_template(std::ostream& f, const json& report, int template_type, bool show_region_depth);
private:
    void output_region(std::ostream& f, const json& region, bool show_unit_attitudes, bool show_region_depth);
    void output_region_header(std::ostream& f, const json& region, bool show_region_depth);
    void output_item_list(std::ostream& f, const json& item_list, string header);
    void output_items(std::ostream& f, const json& item_list, bool assume_singular = false, bool show_single_amt = false);
    void output_item(std::ostream& f, const json& item, bool assume_singular = false, bool show_single_amt = false);
    void output_structure(std::ostream& f, const json& structure, bool show_unit_attitudes);
    void output_unit(std::ostream& f, const json& unit, bool show_unit_attitudes);
    void output_unit_summary(std::ostream& f, const json& unit, bool show_faction = true);
    void output_region_template(std::ostream& f, const json& region, int template_type, bool show_region_depth);
    void output_region_map_header(std::ostream& f, const json& region, bool show_region_depth);
    void output_region_map_header_line(std::ostream& f, std::string line);
    std::string next_map_header_line(int line, const json& region);
    void output_unit_template(std::ostream& f, const json& unit, int template_type);
    void output_unit_orders(std::ostream& f, const json& orders);

    // Without this utility function for strings, we end up printing "<string>" instead of <string> since the
    // item is a json element at that point, so we have a nice utility function here.
    std::string to_s(const json& item);

    // Some private data members to help with output formatting of the map template
    static const int line_width;
    static const int map_width;
    static const size_t template_fill_size;
    static const int template_max_lines;

    static const std::string template_map[];
    static const std::map<const std::string, const std::array<std::string, 2>> terrain_fill;
    static const std::map<const std::string, const std::array<int, 2>> direction_offsets;
};
#endif
