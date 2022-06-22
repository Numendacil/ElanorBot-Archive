add_library(cppcodec INTERFACE)
target_include_directories(cppcodec INTERFACE cppcodec/)
target_link_libraries(Elanor PRIVATE cppcodec)

target_sources(Elanor 
	PRIVATE 
		croncpp.h
		date.h
		log.h
		json.hpp
		httplib.hpp
		uuid.h
	)