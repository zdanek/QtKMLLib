find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED)
find_library(MiniZip NAMES minizip REQUIRED)
find_library(UriParser NAMES uriparser REQUIRED)
find_library(Expat NAMES expat REQUIRED)
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libkml/lib/cmake;${CMAKE_PREFIX_PATH}")

find_package(Qt5 REQUIRED COMPONENTS Gui Quick Positioning REQUIRED)


find_package(LibKML REQUIRED)

set(HEADERS
    qtkml.h
    rendervisitor.h
    graphicsvisitor.h
    stylevisitor.h
    mercatorprojection.h
    kmldocument.h
    coord.h
    kmlelement.h
    kmlgraphics.h
    kmlrenderer.h
    qmlkml.h)

set(SOURCES
    rendervisitor.cpp
    graphicsvisitor.cpp
    stylevisitor.cpp
    mercatorprojection.cpp
    kmldocument.cpp
    kmlelement.cpp
    kmlgraphics.cpp
    qmlimage.cpp
    kmlrenderer.cpp
    kmlitem.cpp)

add_library(qtkml
    ${HEADERS}
    ${SOURCES}
    )

target_include_directories(qtkml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_include_directories(qtkml PUBLIC ${LIBKML_INCLUDE_DIRS})

target_link_libraries(qtkml
    PUBLIC
    ${LIBKML_LIBRARIES}
    PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Positioning
    Qt::Quick
    ZLIB::ZLIB
    ${Expat}
    ${UriParser}
    ${MiniZip})


