
v_cc_library(
  NAME s3
  SRCS
    client.cc
    signature.cc
    error.cc
  DEPS
    Seastar::seastar
    v::bytes
    v::http
)
add_subdirectory(tests)
add_subdirectory(test_client)

