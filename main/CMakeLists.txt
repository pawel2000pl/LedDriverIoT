idf_component_register(
    SRCS "main.cpp" "src/ledc_driver.cpp" "src/conversions.cpp" "src/dynamic_buffers.cpp" "src/hardware_configuration.cpp" "src/temperature.cpp" "src/outputs.cpp" "src/endpoints/initialization.cpp" "src/endpoints/colors.cpp" "src/endpoints/network.cpp" "src/endpoints/temperature.cpp" "src/endpoints/outputs.cpp" "src/endpoints/update.cpp" "src/endpoints/configuration.cpp" "src/endpoints/favorites.cpp" "src/knobs.cpp" "src/fastlz.cpp" "src/server.cpp" "src/taylormath.cpp" "src/inputs.cpp" "src/modules.cpp" "src/resources.cpp" "src/wifi.cpp" "src/filter_functions.cpp" "src/configuration.cpp" "src/timer_shutdown.cpp" "src/timer_mgr.cpp" "src/validate_json.cpp"
    INCLUDE_DIRS "src"
)
