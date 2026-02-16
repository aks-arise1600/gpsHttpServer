cmake_minimum_required(VERSION 3.16)

project(gpsHttpServerApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Find Qt ----
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    HttpServer
)

# ---- Executable ----
add_executable(${PROJECT_NAME}
    main.cpp
)

# ---- Link Qt ----
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
)

# ---- Required for Qt ----
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

