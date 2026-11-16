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

}
