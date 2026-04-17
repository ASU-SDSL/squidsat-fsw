#include "i2c_util.h"

#include "hardware/gpio.h"

#include "HardwareConfig.h"

#define I2CSpeed 100000 			/// Default of 100k
#define I2CTimeout_us 1000000 		/// Timeout for read and writes in micro-seconds

void i2c_util_init() {
   
    // i2c1 initialize 
    i2c_init(i2c0, I2CSpeed);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

}


int i2c_write_to_register(	i2c_inst_t *i2c,
							const uint8_t addr,
							const uint8_t reg,
							uint8_t *buf,
							const uint8_t nbytes){

	if (nbytes < 1) { return 1; }

	// create message, putting the register first (mes is just [reg, [buf]])
	uint8_t msg[nbytes + 1];
	msg[0] = reg;
	for (int i = 0; i < nbytes; i++) {
		msg[i + 1] = buf[i];
	}

	int num_bytes_written = i2c_write_timeout_us(i2c, addr, msg, (nbytes + 1), false, I2CTimeout_us);
	if (num_bytes_written != nbytes + 1) { return 1; }

	return 0; // no errors
}

int i2c_read_from_register(	i2c_inst_t *i2c,
							const uint8_t addr,
							const uint8_t reg,
							uint8_t *buf,
							const uint8_t nbytes){

	if (nbytes < 1) { return 1; }

	int bytes_written = i2c_write_timeout_us(i2c, addr, &reg, 1, true, I2CTimeout_us);
	if (bytes_written != 1) { return 1; } // return error

	int num_bytes_read = i2c_read_timeout_us(i2c, addr, buf, nbytes, false, I2CTimeout_us);
	if (num_bytes_read != nbytes) { return 1; }

	return 0; // no errors
}