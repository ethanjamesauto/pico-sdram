// Software driver for pico_sdram - an SDRAM controller for the Raspberry Pi
// Pico

#include "hardware/pio.h"

typedef struct {
} sdram_controller_t;

/**
 * @brief Initialize the SDRAM controller to a working state. After this
 * function is called, the SDRAM is ready for use.
 *
 * @return true if the controller was initialized successfully, false otherwise
 */
bool sdram_init(sdram_controller_t* controller);

/**
 * @brief Write a buffer of data to the SDRAM at the specified address.
 *
 * @param controller the SDRAM controller instance
 * @param addr the address to write to
 * @param data the data buffer to write
 * @param len the length of the data buffer
 * @return true if the data was written successfully, false otherwise
 */
bool sdram_write_blocking(sdram_controller_t* controller,
                          uint32_t addr,
                          uint8_t* data,
                          size_t len);

/**
 * @brief Write a buffer of data to the SDRAM at the specified address without
 * blocking.
 *
 * @param controller the SDRAM controller instance
 * @param addr the address to write to
 * @param data the data buffer to write
 * @param len the length of the data buffer
 * @param done a pointer to a boolean that will be set to true when the read is complete
 * @return true if the data was written successfully, false otherwise
 */
bool sdram_write_nonblocking(sdram_controller_t* controller,
                             uint32_t addr,
                             uint8_t* data,
                             size_t len,
                             bool* done);

/**
 * @brief Write a buffer of data to the SDRAM at the specified address without
 * blocking, and call a callback function when the write is complete.
 *
 * @param controller the SDRAM controller instance
 * @param addr the address to write to
 * @param data the data buffer to write
 * @param len the length of the data buffer
 * @param callback the function to call when the write is complete
 * @return true if the data was written successfully, false otherwise
 */
bool sdram_write_nonblocking_callback(sdram_controller_t* controller,
                                      uint32_t addr,
                                      uint8_t* data,
                                      size_t len,
                                      void (*callback)(void));

/**
 * @brief Read a buffer of data from the SDRAM at the specified address.
 *
 * @param controller the SDRAM controller instance
 * @param addr the address to read from
 * @param data the buffer to store the read data
 * @param len the length of the data buffer
 * @return true if the data was read successfully, false otherwise
 */
bool sdram_read_blocking(sdram_controller_t* controller,
                         uint32_t addr,
                         uint8_t* data,
                         size_t len);

/**
 * @brief Read a buffer of data from the SDRAM at the specified address without
 * blocking.
 *
 * @param controller the SDRAM controller instance
 * @param addr the address to read from
 * @param data the buffer to store the read data
 * @param len the length of the data buffer
 * @param done a pointer to a boolean that will be set to true when the read is complete
 * @return true if the data was read successfully, false otherwise
 */
bool sdram_read_nonblocking(sdram_controller_t* controller,
                            uint32_t addr,
                            uint8_t* data,
                            size_t len,
                            bool* done);

/**
 * @brief Read a buffer of data from the SDRAM at the specified address without
 * blocking, and call a callback function when the read is complete.
 *
 * @param controller the SDRAM controller instance
 * @param addr the address to read from
 * @param data the buffer to store the read data
 * @param len the length of the data buffer
 * @param callback the function to call when the read is complete
 * @return true if the data was read successfully, false otherwise
 */
bool sdram_read_nonblocking_callback(sdram_controller_t* controller,
                                     uint32_t addr,
                                     uint8_t* data,
                                     size_t len,
                                     void (*callback)(void));

/**
 * @brief Enter self-refresh mode. This function should be called when the SDRAM
 * is not in use to save power.
 */
bool sdram_self_refresh(sdram_controller_t* controller);