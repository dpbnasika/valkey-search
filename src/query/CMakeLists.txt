set(SRCS_PREDICATE ${CMAKE_CURRENT_LIST_DIR}/predicate.cc
                   ${CMAKE_CURRENT_LIST_DIR}/predicate.h)

valkey_search_add_static_library(predicate "${SRCS_PREDICATE}")
target_include_directories(predicate PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(predicate PUBLIC numeric)
target_link_libraries(predicate PUBLIC tag)
target_link_libraries(predicate PUBLIC log)
target_link_libraries(predicate PUBLIC managed_pointers)
target_link_libraries(predicate PUBLIC type_conversions)

set(SRCS_PREDICATE_HEADER ${CMAKE_CURRENT_LIST_DIR}/predicate.h)

add_library(predicate_header INTERFACE ${SRCS_PREDICATE_HEADER})
target_include_directories(predicate_header INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(predicate_header INTERFACE managed_pointers)
target_link_libraries(predicate_header INTERFACE type_conversions)

set(SRCS_SEARCH ${CMAKE_CURRENT_LIST_DIR}/search.cc
                ${CMAKE_CURRENT_LIST_DIR}/search.h)

valkey_search_add_static_library(search "${SRCS_SEARCH}")
target_include_directories(search PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(search PUBLIC planner)
target_link_libraries(search PUBLIC predicate)
target_link_libraries(search PUBLIC attribute_data_type)
target_link_libraries(search PUBLIC index_schema)
target_link_libraries(search PUBLIC metrics)
target_link_libraries(search PUBLIC filter_parser)
target_link_libraries(search PUBLIC index_base)
target_link_libraries(search PUBLIC numeric)
target_link_libraries(search PUBLIC tag)
target_link_libraries(search PUBLIC vector_base)
target_link_libraries(search PUBLIC vector_flat)
target_link_libraries(search PUBLIC vector_hnsw)
target_link_libraries(search PUBLIC hnswlib_vmsdk)
target_link_libraries(search PUBLIC latency_sampler)
target_link_libraries(search PUBLIC log)
target_link_libraries(search PUBLIC managed_pointers)
target_link_libraries(search PUBLIC thread_pool)
target_link_libraries(search PUBLIC time_sliced_mrmw_mutex)
target_link_libraries(search PUBLIC type_conversions)
target_link_libraries(search PUBLIC status_macros)
target_link_libraries(search PUBLIC valkey_module)

set(SRCS_SEARCH_HEADER ${CMAKE_CURRENT_LIST_DIR}/search.h)

add_library(search_header INTERFACE ${SRCS_SEARCH_HEADER})
target_include_directories(search_header INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(search_header INTERFACE predicate_header)
target_link_libraries(search_header INTERFACE index_schema)
target_link_libraries(search_header INTERFACE filter_parser)
target_link_libraries(search_header INTERFACE index_base)
target_link_libraries(search_header INTERFACE vector_base)
target_link_libraries(search_header INTERFACE hnswlib_vmsdk)
target_link_libraries(search_header INTERFACE managed_pointers)
target_link_libraries(search_header INTERFACE thread_pool)

set(SRCS_FANOUT ${CMAKE_CURRENT_LIST_DIR}/fanout.cc
                ${CMAKE_CURRENT_LIST_DIR}/fanout.h)

valkey_search_add_static_library(fanout "${SRCS_FANOUT}")
target_include_directories(fanout PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(fanout PUBLIC search)
target_link_libraries(fanout PUBLIC attribute_data_type)
target_link_libraries(fanout PUBLIC index_schema)
target_link_libraries(fanout PUBLIC client_pool)
target_link_libraries(fanout PUBLIC coordinator_cc_proto)
target_link_libraries(fanout PUBLIC search_converter)
target_link_libraries(fanout PUBLIC util)
target_link_libraries(fanout PUBLIC vector_base)
target_link_libraries(fanout PUBLIC string_interning)
target_link_libraries(fanout PUBLIC log)
target_link_libraries(fanout PUBLIC managed_pointers)
target_link_libraries(fanout PUBLIC thread_pool)
target_link_libraries(fanout PUBLIC type_conversions)
target_link_libraries(fanout PUBLIC status_macros)
target_link_libraries(fanout PUBLIC valkey_module)

set(SRCS_RESPONSE_GENERATOR ${CMAKE_CURRENT_LIST_DIR}/response_generator.cc
                            ${CMAKE_CURRENT_LIST_DIR}/response_generator.h)

valkey_search_add_static_library(response_generator
                                 "${SRCS_RESPONSE_GENERATOR}")
target_include_directories(response_generator PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(response_generator PUBLIC predicate_header)
target_link_libraries(response_generator PUBLIC search_header)
target_link_libraries(response_generator PUBLIC attribute_data_type)
target_link_libraries(response_generator PUBLIC coordinator_cc_proto)
target_link_libraries(response_generator PUBLIC tag)
target_link_libraries(response_generator PUBLIC vector_base)
target_link_libraries(response_generator PUBLIC managed_pointers)
target_link_libraries(response_generator PUBLIC type_conversions)
target_link_libraries(response_generator PUBLIC status_macros)
target_link_libraries(response_generator PUBLIC valkey_module)

set(SRCS_PLANNER ${CMAKE_CURRENT_LIST_DIR}/planner.cc
                 ${CMAKE_CURRENT_LIST_DIR}/planner.h)

valkey_search_add_static_library(planner "${SRCS_PLANNER}")
target_include_directories(planner PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(planner PUBLIC index_base)
target_link_libraries(planner PUBLIC vector_base)
