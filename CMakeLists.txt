cmake_minimum_required(VERSION 3.22)

# Universal (Apple Silicon + Intel) on macOS, static runtime on Windows.
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

project(SocaChords VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SOCA_BUILD_TESTS "Build the library self-test" OFF)
set(SOCA_JUCE_PATH "" CACHE PATH "Path to an existing JUCE checkout (optional)")

if(SOCA_JUCE_PATH)
    add_subdirectory(${SOCA_JUCE_PATH} juce EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.9
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

if(APPLE)
    set(SOCA_FORMATS VST3 AU Standalone)
else()
    set(SOCA_FORMATS VST3 Standalone)
endif()

juce_add_plugin(SocaChords
    PRODUCT_NAME "Soca Chords"
    COMPANY_NAME "Dev Griffin"
    BUNDLE_ID "com.devgriffin.socachords"
    PLUGIN_MANUFACTURER_CODE Dvgr
    PLUGIN_CODE Soca
    FORMATS ${SOCA_FORMATS}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(SocaChordsAssets SOURCES
    Assets/Pacifico-Regular.ttf
    Assets/Poppins-Bold.ttf
    Assets/Poppins-ExtraBold.ttf)
set_target_properties(SocaChordsAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_sources(SocaChords PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(SocaChords PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(SocaChords
    PRIVATE
        juce::juce_audio_utils
        SocaChordsAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(SOCA_BUILD_TESTS)
    juce_add_console_app(SocaChordsTest PRODUCT_NAME "SocaChordsTest")
    target_sources(SocaChordsTest PRIVATE Tests/LibraryTest.cpp)
    target_compile_definitions(SocaChordsTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(SocaChordsTest PRIVATE juce::juce_audio_basics juce::juce_core)

    juce_add_console_app(SocaChordsEngineTest PRODUCT_NAME "SocaChordsEngineTest")
    target_sources(SocaChordsEngineTest PRIVATE Tests/EngineTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(SocaChordsEngineTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(SocaChordsEngineTest PRIVATE juce::juce_audio_utils SocaChordsAssets)
endif()
