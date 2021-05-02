#include "dht11.h"
#include "esp32/rom/ets_sys.h"
#include "driver/gpio.h"

#define delay_us(x) ets_delay_us(x)

static void DHT11_RST(void)
{
    DHT11_IO_OUT();
    DHT11_DQ_OUT_Low();
    delay_us(20000);
    DHT11_DQ_OUT_High();
    delay_us(30);
}

static unsigned char DHT11_Check(void)
{
    unsigned char retry=0;
    DHT11_IO_IN();
    while(DHT11_DQ_IN() && retry<100)
    {
        retry++;
        delay_us(1);
    }
    if(retry>=100)
    {
        return 1;
    }
    else
    {
        retry=0;
    }
    while(!DHT11_DQ_IN() && retry<100)
    {
        retry++;
        delay_us(1);
    }
    if(retry>=100) return 1;
    return 0;
}

static unsigned char DHT11_Read_Bit(void)
{
    unsigned char retry=0;
    while(DHT11_DQ_IN() && retry<100)
    {
        retry++;
        delay_us(1);
    }
    retry=0;
    while(!DHT11_DQ_IN() && retry<100)
    {
        retry++;
        delay_us(1);
    }
    delay_us(29);
    if(DHT11_DQ_IN()) return 1;
    else return 0;
}

static unsigned char DHT11_Read_Byte(void)
{
    unsigned char i,data;
    data = 0;
    for(i=0;i<8;i++)
    {
        data|=(DHT11_Read_Bit()<<(7-i));
    }
    return data;
}

unsigned char DHT11_Read_Data(unsigned char *temp,unsigned char *humi)
{
    unsigned char buf[5];
    unsigned char i;
    DHT11_RST();
    if(DHT11_Check()==0)
    {
        for(i=0;i<5;i++)
        {
            buf[i]=DHT11_Read_Byte();
        }
        if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
        {
            *humi = buf[0];
            *temp = buf[2];
        }
    }
    else
    {
        return 1;
    }
    return 0;
}
unsigned char DHT11_GPIO_Init()
{
	gpio_pad_select_gpio(DHT11_Pin);
    gpio_set_direction(DHT11_Pin,GPIO_MODE_OUTPUT);
    gpio_pullup_en(DHT11_Pin);
    gpio_set_pull_mode(DHT11_Pin,GPIO_PULLUP_ONLY);

    DHT11_RST();
    return DHT11_Check();
}
