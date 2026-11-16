// ======================================================================
// \title  ZephyrUartDriver.cpp
// \author ethanchee
// \brief  cpp file for ZephyrUartDriver component implementation class
// ======================================================================


#include "fprime-zephyr/Drv/ZephyrUartDriver/ZephyrUartDriver.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Types/Assert.hpp"
#include <Fw/FPrimeBasicTypes.hpp>

namespace Zephyr {

    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    ZephyrUartDriver ::
        ZephyrUartDriver(
            const char *const compName
        ) : ZephyrUartDriverComponentBase(compName),
            m_dev(nullptr),
            m_framingErrors(ATOMIC_INIT(0)),
            m_noiseErrors(ATOMIC_INIT(0)),
            m_overrunErrors(ATOMIC_INIT(0))
    {
    }

    ZephyrUartDriver ::
        ~ZephyrUartDriver()
    {

    }

    void ZephyrUartDriver::configure(const struct device *dev, U32 baud_rate, bool flow, U8 *ring_buf_data,
                                     U32 ring_buf_size) {
        FW_ASSERT(dev != nullptr);
        m_dev = dev;

        if (!device_is_ready(this->m_dev)) {
            return;
        }

        struct uart_config uart_cfg = {
            .baudrate = baud_rate,
            .parity = UART_CFG_PARITY_NONE,
            .stop_bits = UART_CFG_STOP_BITS_1,
            .data_bits = UART_CFG_DATA_BITS_8,
            .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
        };

        if(flow){
            uart_cfg.flow_ctrl = UART_CFG_FLOW_CTRL_RS485;
        }
        uart_configure(this->m_dev, &uart_cfg);

        // Use the caller's ring storage when given; otherwise the member array
        if ((ring_buf_data != nullptr) && (ring_buf_size != 0)) {
            ring_buf_init(&this->m_ring_buf, ring_buf_size, ring_buf_data);
        } else {
            ring_buf_init(&this->m_ring_buf, RING_BUF_SIZE, this->m_ring_buf_data);
        }
        uart_irq_callback_user_data_set(this->m_dev, serial_cb, this);

        uart_irq_rx_enable(this->m_dev);
	    uart_irq_tx_disable(this->m_dev);

        if (this->isConnected_ready_OutputPort(0)) {
            this->ready_out(0);
        }
    }

    void ZephyrUartDriver::serial_cb(const struct device *dev, void *user_data)
    {
        ZephyrUartDriver *self = reinterpret_cast<ZephyrUartDriver *>(user_data);
        struct ring_buf *ring_buf = &self->m_ring_buf;

        if (!uart_irq_update(dev)) {
            return;
        }

        // Read and clear the receive error flags before the data: reading the data clears the overrun flag.
        const int errors = uart_err_check(dev);
        if (errors > 0) {
            if ((errors & UART_ERROR_FRAMING) != 0) {
                (void)atomic_inc(&self->m_framingErrors);
            }
            if ((errors & UART_ERROR_NOISE) != 0) {
                (void)atomic_inc(&self->m_noiseErrors);
            }
            if ((errors & UART_ERROR_OVERRUN) != 0) {
                (void)atomic_inc(&self->m_overrunErrors);
            }
        }

        if (!uart_irq_rx_ready(dev)) {
            return;
        }

        U8 c;
        // TODO: Get rid of the endless loop (in an IRQ handler!).
        while (uart_fifo_read(dev, &c, 1) == 1) {
            if (ring_buf_put(ring_buf, &c, 1) != 1) {
                // TODO: Handle properly.
                printk("UART buffer overrun\n");
            }
        }
    }

    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    void ZephyrUartDriver ::
        schedIn_handler(
            const FwIndexType portNum,
            U32 context
        )
    {
        Fw::Buffer recv_buffer = this->allocate_out(0, SERIAL_BUFFER_SIZE);

        U32 recv_size = ring_buf_get(&this->m_ring_buf, recv_buffer.getData(), recv_buffer.getSize());
        if (recv_size == 0) {
            // No data received, deallocate buffer
            this->deallocate_out(0, recv_buffer);
        } else {
            recv_buffer.setSize(recv_size);
            recv_out(0, recv_buffer, Drv::ByteStreamStatus::OP_OK);
        }
    }

    Drv::ByteStreamStatus ZephyrUartDriver ::
        send_handler(
            const FwIndexType portNum,
            Fw::Buffer &sendBuffer
        )
    {
        for (U32 i = 0; i < sendBuffer.getSize(); i++) {
            uart_poll_out(this->m_dev, sendBuffer.getData()[i]);
        }
        return Drv::ByteStreamStatus::OP_OK;
    }

    void ZephyrUartDriver ::recvReturnIn_handler(const FwIndexType portNum, Fw::Buffer &returnBuffer) {
        this->deallocate_out(0, returnBuffer);
    }

    bool ZephyrUartDriver ::txComplete_handler(const FwIndexType portNum) {
        // uart_poll_out() returns once a byte is in the transmit register, before it has left the pin.
        // The transmission-complete flag is set only when the last stop bit is out; writing a byte clears it.
        FW_ASSERT(this->m_dev != nullptr);
        return uart_irq_tx_complete(this->m_dev) > 0;
    }

    Fw::Success ZephyrUartDriver ::setBaudRate_handler(const FwIndexType portNum, U32 baudRate) {
        FW_ASSERT(this->m_dev != nullptr);
        struct uart_config config;
        if (uart_config_get(this->m_dev, &config) != 0) {
            return Fw::Success::FAILURE;
        }
        config.baudrate = baudRate;
        return (uart_configure(this->m_dev, &config) == 0) ? Fw::Success::SUCCESS : Fw::Success::FAILURE;
    }

    Zephyr::UartErrorCounts ZephyrUartDriver ::errorCounts_handler(const FwIndexType portNum) {
        return Zephyr::UartErrorCounts(static_cast<U32>(atomic_get(&this->m_framingErrors)),
                                       static_cast<U32>(atomic_get(&this->m_noiseErrors)),
                                       static_cast<U32>(atomic_get(&this->m_overrunErrors)));
    }

} // end namespace Zephyr
