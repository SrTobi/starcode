
################### setup include directories ###################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})



################### scan for source ##################

buildx_scan_here(STARCODE_SOURCE "cpp;hpp")
buildx_debug("source: ${STARCODE_SOURCE}" proj)
buildx_auto_group("${STARCODE_SOURCE}")

add_executable(starcode ${STARCODE_SOURCE})
target_link_libraries(starcode ${Boost_LIBRARIES} x::utilx pthread ssl crypto ${V8_LIBRARY} "/usr/lib/libv8_libplatform.so" pthread)
#buildx_copy_media(starcode "game")


buildx_add_internal_test(test-starcode ${PROJECT_SOURCE_DIR}/tests
							TEST_TARGET starcode
							DEFINITIONS STARCODE_TEST)
target_link_libraries(test-starcode ${Boost_LIBRARIES} x::utilx pthread ssl crypto ${V8_LIBRARY} "/usr/lib/libv8_libplatform.so" pthread)
# tests
#AUTO_SOURCE_GROUP("${TEST_SOURCE}")
#add_executable(test-starcode ${SOURCE} ${TEST_SOURCE})
#ADD_PRECOMPILED_HEADER(test-starcode "includes.hpp")
#target_link_libraries(test-starcode ${Boost_LIBRARIES})
