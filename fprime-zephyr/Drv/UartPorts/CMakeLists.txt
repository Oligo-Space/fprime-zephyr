####
# Port types for UART control beyond the byte stream. Port definitions only,
# with no Zephyr code, so components built for any platform can use them.
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/UartPorts.fpp"
)

register_fprime_module()
