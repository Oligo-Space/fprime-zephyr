module Zephyr {

  passive component ZephyrUartDriver {
    import Drv.ByteStreamDriver

    @ Polled sched-in for reading UART
    sync input port schedIn: Svc.Sched

    @ True when the UART has finished sending: the last stop bit has left
    @ the pin. Reads the hardware flag and returns at once. For a caller that
    @ switches an RS-485 transceiver in software after a send.
    sync input port txComplete: Zephyr.UartTxComplete

    @ Sets the UART's speed and keeps its other settings. Connect it to the
    @ one component that owns this link's protocol.
    sync input port setBaudRate: Zephyr.UartSetBaudRate

    @ Receive error totals since configure(), counted in the interrupt handler
    sync input port errorCounts: Zephyr.UartGetErrorCounts

    @Allocate new buffer
    output port allocate: Fw.BufferGet

    @return the allocated buffer
    output port deallocate: Fw.BufferSend
  }
}
