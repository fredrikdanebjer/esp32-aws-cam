add_library(fsu_eye_freertos
  fsu_eye_freertos_callbacks.c
 )

target_compile_features(fsu_eye_freertos
  PUBLIC
  )

target_link_libraries(fsu_eye_freertos
  PRIVATE
  AFR::kernel
  )
