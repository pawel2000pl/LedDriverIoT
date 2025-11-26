
FILE(GLOB CppSources 
    main.cpp 
    src/*.cpp 
    src/endpoints/*.cpp 
    src/lib/esp32_https_server/*.cpp 
    src/lib/fastlz/*.cpp
    src/lib/fixedpoint/*.cpp
)

idf_component_register(
    SRCS ${CppSources}
    INCLUDE_DIRS "src"
)
