module Zephyr {

  @ Receive errors a UART has flagged since it was configured
  struct UartErrorCounts {
    framing: U32 @< A byte arrived without its stop bit
    noise: U32 @< The samples within one bit disagreed
    overrun: U32 @< A byte arrived before the previous one was read
  }

  @ Asks whether the UART has finished sending: the last stop bit has left
  @ the pin and nothing waits to be sent. Never blocks.
  port UartTxComplete -> bool

  @ Sets the UART's speed and keeps its other settings. Returns SUCCESS when
  @ the UART runs at the new speed.
  port UartSetBaudRate(
    baudRate: U32 @< Bits per second
  ) -> Fw.Success

  @ Reads the UART's receive error totals since it was configured
  port UartGetErrorCounts -> UartErrorCounts

  @ Capacity of a receive trace
  constant UART_RX_TRACE_SIZE = 64

  @ The most recent bytes the UART received, oldest first, each with the
  @ cycle counter value read in the interrupt that received it and the
  @ error flags that interrupt saw (Zephyr UART_ERROR_* bits; 0 = clean).
  @ Diagnostic: lets the caller place each byte and each error in time
  @ relative to its own actions on the line.
  struct UartRxTrace {
    count: U32 @< Valid entries, at most UART_RX_TRACE_SIZE
    total: U32 @< Bytes received since configure(), wrapping
    cycles: [UART_RX_TRACE_SIZE] U32 @< k_cycle_get_32() at receipt
    bytes: [UART_RX_TRACE_SIZE] U8 @< Byte value
    flags: [UART_RX_TRACE_SIZE] U8 @< Error flags seen with this byte
  }

  @ Copies the receive trace into the caller's struct
  port UartGetRxTrace(
    ref trace: UartRxTrace @< Filled by the driver
  )

}
