// ======================================================================
// \title  ZephyrUartDriver.hpp
// \author ethanchee
// \brief  hpp file for ZephyrUartDriver component implementation class
// ======================================================================

#ifndef ZephyrUartDriver_HPP
#define ZephyrUartDriver_HPP

#include "fprime-zephyr/Drv/ZephyrUartDriver/ZephyrUartDriverComponentAc.hpp"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>

#define RING_BUF_SIZE 1024

namespace Zephyr {

  class ZephyrUartDriver :
    public ZephyrUartDriverComponentBase
  {

    const FwSizeType SERIAL_BUFFER_SIZE = 64;

    public:

        // ----------------------------------------------------------------------
        // Construction, initialization, and destruction
        // ----------------------------------------------------------------------

        //! Construct object ZephyrUartDriver
        //!
        ZephyrUartDriver(
            const char *const compName /*!< The component name*/
        );

        //! Destroy object ZephyrUartDriver
        //!
        ~ZephyrUartDriver();

        //! Configure the UART and start interrupt-driven receive
        //!
        //! The receive ring is filled by the UART interrupt handler and emptied once per schedIn call, into
        //! one buffer from the allocate port. By default the ring is the member array of RING_BUF_SIZE bytes.
        //! A caller can give its own ring buffer for a link that receives more than RING_BUF_SIZE bytes in one
        //! schedIn period. The ring should not be larger than the allocator's buffer size: bytes in excess of
        //! one allocated buffer wait for the next schedIn call.
        //!
        void configure(const struct device *dev, /*!< The UART device*/
                       U32 baud_rate, /*!< The baud rate*/
                       bool flow = false, /*!< true selects RS-485 flow control*/
                       U8 *ring_buf_data = nullptr, /*!< Caller-owned receive ring storage, or nullptr*/
                       U32 ring_buf_size = 0 /*!< Size of ring_buf_data in bytes, or 0*/
        );

    public:

        static void serial_cb(const struct device *dev, void *user_data);

        // ----------------------------------------------------------------------
        // Handler implementations for user-defined typed input ports
        // ----------------------------------------------------------------------

        //! Handler implementation for schedIn
        //!
        void schedIn_handler(
            const FwIndexType portNum, /*!< The port number*/
            U32 context /*!< 
        The call order
        */
        );


        //! Handler implementation for send
        //!
        Drv::ByteStreamStatus send_handler(
            const FwIndexType portNum, /*!< The port number*/
            Fw::Buffer &sendBuffer 
        );

        void recvReturnIn_handler(
            const FwIndexType portNum, /*!< The port number*/
            Fw::Buffer &returnBuffer
        );

        const struct device *m_dev;

        U8 m_ring_buf_data[RING_BUF_SIZE];
        struct ring_buf m_ring_buf;
    };

} // end namespace Zephyr

#endif
