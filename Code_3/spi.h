/*
 * spi.h
 *
 * Created: 3-2-2018 13:36:33
 *  Author: Thomas Plooijer
 */ 

#define SPI_SS_bm      0x10
#define SPI_MOSI_bm    0x20
#define SPI_MISO_bm    0x40
#define SPI_SCK_bm     0x80

#define FOO            0x00

void    spi_init(void);
uint8_t spi_transfer(uint8_t data);
void    spi_write(uint8_t data);
uint8_t spi_read(void);
