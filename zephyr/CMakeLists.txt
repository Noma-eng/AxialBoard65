target_include_directories(app PRIVATE
  ${WEST_TOPDIR}/zmk/app/include
  ${WEST_TOPDIR}/zmk/app/module/include
)

target_sources(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/../src/matrix_encoders.c
)