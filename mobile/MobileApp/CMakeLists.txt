cmake_minimum_required(VERSION 3.16)
project(MobileApp VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui)

qt_standard_project_setup(REQUIRES 6.10)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appMobileApp
    main.cpp
    peripherals/flashlight/FlashlightController.h
)

qt_add_qml_module(appMobileApp
    URI MobileApp
    QML_FILES
        qml/Main.qml
        qml/Dashboard.qml
        qml/FlashlightPage.qml
        qml/BluetoothPage.qml
        qml/GyroscopePage.qml
)

set_target_properties(appMobileApp PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(appMobileApp PRIVATE
    peripherals/flashlight
)

target_link_libraries(appMobileApp
    PRIVATE Qt6::Quick Qt6::Core Qt6::Gui
)

if(ANDROID)
    set_target_properties(appMobileApp PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
endif()

install(TARGETS appMobileApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)