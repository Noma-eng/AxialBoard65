zephyr_library()

# ZMK のヘッダを外部モジュールから参照できるようにする
# WEST_TOPDIR は CI だと /tmp/zmk-config になる
zephyr_library_include_directories(
  ${WEST_TOPDIR}/zmk/app/include
  ${WEST_TOPDIR}/zmk/app/module/include
)

zephyr_library_sources(${CMAKE_CURRENT_LIST_DIR}/../src/matrix_encoders.c)