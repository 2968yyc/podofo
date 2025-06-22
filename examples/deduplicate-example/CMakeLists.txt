add_executable(deduplicate-example deduplicate-example.cpp)
target_link_libraries(deduplicate-example podofo)

add_executable(create-test-pdf create-test-pdf.cpp)
target_link_libraries(create-test-pdf podofo) 