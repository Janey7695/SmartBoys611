#ifndef DHT11_H
#define DHT11_H

#define DHT11_Pin 19

#define DHT11_IO_IN() gpio_set_direction(DHT11_Pin,GPIO_MODE_OUTPUT)
#define DHT11_IO_OUT() gpio_set_direction(DHT11_Pin,GPIO_MODE_INPUT);

#define	DHT11_DQ_OUT_High() gpio_set_level(DHT11_Pin,1)
#define	DHT11_DQ_OUT_Low() gpio_set_level(DHT11_Pin,0)
#define	DHT11_DQ_IN()  gpio_get_level(DHT11_Pin)

unsigned char DHT11_Read_Data(unsigned char *temp,unsigned char *humi);
unsigned char DHT11_GPIO_Init(void);

#endif

