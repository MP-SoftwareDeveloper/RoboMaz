cmake_minimum_required(VERSION 3.16)
project(MobileApp VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui Sensors)
find_package(Qt6 OPTIONAL_COMPONENTS Bluetooth QUIET)

qt_standard_project_setup(REQUIRES 6.10)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appMobileApp
    main.cpp
    peripherals/flashlight/FlashlightController.h
    peripherals/bluetooth/BluetoothController.h
    peripherals/bluetooth/BluetoothController.cpp
    peripherals/sensors/SensorController.h
    peripherals/sensors/SensorController.cpp
)

qt_add_qml_module(appMobileApp
    URI MobileApp
    QML_FILES
        qml/Main.qml
        qml/Dashboard.qml
        qml/FlashlightPage.qml
        qml/GyroscopePage.qml
        qml/CompassPage.qml
        qml/BluetoothPage.qml
)

set_target_properties(appMobileApp PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(appMobileApp PRIVATE
    peripherals/flashlight
    peripherals/bluetooth
    peripherals/sensors
)

target_link_libraries(appMobileApp
    PRIVATE Qt6::Quick Qt6::Core Qt6::Gui Qt6::Sensors
)

if(Qt6Bluetooth_FOUND)
    target_link_libraries(appMobileApp PRIVATE Qt6::Bluetooth)
    target_compile_definitions(appMobileApp PRIVATE HAS_QT_BLUETOOTH)
    message(STATUS "Qt Bluetooth found — BT scanning enabled")
else()
    message(STATUS "Qt Bluetooth NOT installed — BT page disabled.")
endif()

if(ANDROID)
    set_target_properties(appMobileApp PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
    set_property(TARGET appMobileApp APPEND PROPERTY
        QT_ANDROID_EXTRA_PLUGINS
        "${Qt6_DIR}/../../../qml/QtQuick/VirtualKeyboard"
        # Force sensor backend plugins into load_local_libs so the Android
        # runtime System.loadLibrary()'s them before Qt's plugin scanner runs.
        # Without this, Qt6Sensors is absent from libs.xml → types:0 backends:0.
        "${Qt6_DIR}/../../../plugins/sensors"
    )
endif()

install(TARGETS appMobileApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
