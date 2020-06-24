add_library(util
  src/debug.c
)

target_include_directories(util
  PUBLIC
  include/
)

target_compile_features(util
  PUBLIC
)

target_link_libraries(util
  PRIVATE
)
