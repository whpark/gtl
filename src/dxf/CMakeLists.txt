# gtl.dxf - header-based port of the former biscuit.dxf C++ module.

find_package(boost_pfr CONFIG REQUIRED)

add_library(gtl.dxf STATIC
	dxf.cpp
)

target_include_directories(gtl.dxf
	PUBLIC ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(gtl.dxf
	PUBLIC gtl gtl.shape Boost::pfr
)

gtl_apply_common_options(gtl.dxf)
gtl_set_output_name(gtl.dxf "gtl.dxf")

