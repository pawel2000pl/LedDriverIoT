
#pragma once

struct Resource {
    const char* name;
    const char* mime_type;
    const unsigned char* data;
    const unsigned int size;
    const unsigned int decompressed_size;
};


extern const unsigned char main_js_data[];
extern const char main_js_filename[];
extern const char main_js_mime_type[];
extern const unsigned int main_js_size;
extern const unsigned int main_js_decompressed_size;
extern const struct Resource resource_main_js;

extern const unsigned char favicon_svg_data[];
extern const char favicon_svg_filename[];
extern const char favicon_svg_mime_type[];
extern const unsigned int favicon_svg_size;
extern const unsigned int favicon_svg_decompressed_size;
extern const struct Resource resource_favicon_svg;

extern const unsigned char config_fetcher_js_data[];
extern const char config_fetcher_js_filename[];
extern const char config_fetcher_js_mime_type[];
extern const unsigned int config_fetcher_js_size;
extern const unsigned int config_fetcher_js_decompressed_size;
extern const struct Resource resource_config_fetcher_js;

extern const unsigned char config_schema_json_data[];
extern const char config_schema_json_filename[];
extern const char config_schema_json_mime_type[];
extern const unsigned int config_schema_json_size;
extern const unsigned int config_schema_json_decompressed_size;
extern const struct Resource resource_config_schema_json;

extern const unsigned char config_html_data[];
extern const char config_html_filename[];
extern const char config_html_mime_type[];
extern const unsigned int config_html_size;
extern const unsigned int config_html_decompressed_size;
extern const struct Resource resource_config_html;

extern const unsigned char colors_desc_html_data[];
extern const char colors_desc_html_filename[];
extern const char colors_desc_html_mime_type[];
extern const unsigned int colors_desc_html_size;
extern const unsigned int colors_desc_html_decompressed_size;
extern const struct Resource resource_colors_desc_html;

extern const unsigned char chart_js_data[];
extern const char chart_js_filename[];
extern const char chart_js_mime_type[];
extern const unsigned int chart_js_size;
extern const unsigned int chart_js_decompressed_size;
extern const struct Resource resource_chart_js;

extern const unsigned char knob_js_data[];
extern const char knob_js_filename[];
extern const char knob_js_mime_type[];
extern const unsigned int knob_js_size;
extern const unsigned int knob_js_decompressed_size;
extern const struct Resource resource_knob_js;

extern const unsigned char filters_desc_html_data[];
extern const char filters_desc_html_filename[];
extern const char filters_desc_html_mime_type[];
extern const unsigned int filters_desc_html_size;
extern const unsigned int filters_desc_html_decompressed_size;
extern const struct Resource resource_filters_desc_html;

extern const unsigned char styles_css_data[];
extern const char styles_css_filename[];
extern const char styles_css_mime_type[];
extern const unsigned int styles_css_size;
extern const unsigned int styles_css_decompressed_size;
extern const struct Resource resource_styles_css;

extern const unsigned char default_config_json_data[];
extern const char default_config_json_filename[];
extern const char default_config_json_mime_type[];
extern const unsigned int default_config_json_size;
extern const unsigned int default_config_json_decompressed_size;
extern const struct Resource resource_default_config_json;

extern const unsigned char config_js_data[];
extern const char config_js_filename[];
extern const char config_js_mime_type[];
extern const unsigned int config_js_size;
extern const unsigned int config_js_decompressed_size;
extern const struct Resource resource_config_js;

extern const unsigned char include_raw_js_data[];
extern const char include_raw_js_filename[];
extern const char include_raw_js_mime_type[];
extern const unsigned int include_raw_js_size;
extern const unsigned int include_raw_js_decompressed_size;
extern const struct Resource resource_include_raw_js;

extern const unsigned char wifi_desc_html_data[];
extern const char wifi_desc_html_filename[];
extern const char wifi_desc_html_mime_type[];
extern const unsigned int wifi_desc_html_size;
extern const unsigned int wifi_desc_html_decompressed_size;
extern const struct Resource resource_wifi_desc_html;

extern const unsigned char conversions_js_data[];
extern const char conversions_js_filename[];
extern const char conversions_js_mime_type[];
extern const unsigned int conversions_js_size;
extern const unsigned int conversions_js_decompressed_size;
extern const struct Resource resource_conversions_js;

extern const unsigned char filter_selector_js_data[];
extern const char filter_selector_js_filename[];
extern const char filter_selector_js_mime_type[];
extern const unsigned int filter_selector_js_size;
extern const unsigned int filter_selector_js_decompressed_size;
extern const struct Resource resource_filter_selector_js;

extern const unsigned char config_components_js_data[];
extern const char config_components_js_filename[];
extern const char config_components_js_mime_type[];
extern const unsigned int config_components_js_size;
extern const unsigned int config_components_js_decompressed_size;
extern const struct Resource resource_config_components_js;

extern const unsigned char knobPanels_js_data[];
extern const char knobPanels_js_filename[];
extern const char knobPanels_js_mime_type[];
extern const unsigned int knobPanels_js_size;
extern const unsigned int knobPanels_js_decompressed_size;
extern const struct Resource resource_knobPanels_js;

extern const unsigned char index_html_data[];
extern const char index_html_filename[];
extern const char index_html_mime_type[];
extern const unsigned int index_html_size;
extern const unsigned int index_html_decompressed_size;
extern const struct Resource resource_index_html;

extern const struct Resource* resources[];
extern const unsigned int resources_count;

extern const unsigned int max_resource_compressed_buffer;
extern const unsigned int max_resource_decompressed_buffer;
