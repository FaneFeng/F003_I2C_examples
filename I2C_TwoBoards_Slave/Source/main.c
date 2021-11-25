/*!
 * @file        main.c
 *
 * @brief       Main program body        
 *
 * @version     V1.0.0
 *
 * @date        2019-11-4
 *
 */

#include "main.h"
#include "Board.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_misc.h"
#include "apm32f00x_eint.h"
#include "apm32f00x_i2c.h"

/* Slave Address */
#define SLAVE_ADDR          (0XB0)

#define DATA_BUF_SIZE       (32)
uint8_t dataBufPt = 0;
uint8_t dataBuf[DATA_BUF_SIZE] = {0};

/* I2C Init */
void I2CInit(void);
/* delay */
void Delay(uint32_t count);
/* Data buffer init */
void DataBufInit(void);

/*!
 * @brief       Main program
 *
 * @param       None
 *
 * @retval      None
 *
 * @note       
 */
int main(void)
{
    I2CInit();
   
    while(1)
    {
        DataBufInit();
        
        dataBufPt = 0;
        /** Wait end of reception ��������*/
        while(dataBufPt < DATA_BUF_SIZE);
        Delay(0x300);
        dataBufPt = 0;
        /** Wait end of transmission �ط�����*/
        while(dataBufPt < DATA_BUF_SIZE){};
        Delay(0x300);
    }
}

/*!
 * @brief       I2C Init   
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void I2CInit(void)
{
    I2C_Config_T i2cConfig;
    
    /** ACK when current byte being received  */
    i2cConfig.ack = I2C_ACK_CURRENT;//ACKEN=1�����յ��ֽ����ݻ��ߵ�ַ���ݺ�����ϻط�һ��ACK
    /** Set Address  */
    i2cConfig.addr = SLAVE_ADDR;
    /** 7-bit address */
    i2cConfig.addrMode = I2C_ADDR_7_BIT;
    /** duty cycle = 2 */
    i2cConfig.dutyCycle = I2C_DUTYCYCLE_2;
    /** Input clock frequency is 48MHZ */
    i2cConfig.inputClkFreqMhz = 48;
    /** Enable buffer and event interrupt */
    i2cConfig.interrupt = I2C_INT_BUFFER | I2C_INT_EVENT;
    /** Output clock frequency is 100KHZ */
    i2cConfig.outputClkFreqHz = 400000;
    I2C_Config(&i2cConfig);
    
    NVIC_EnableIRQRequest(IIC_IRQn, 0X01);
    
    /** Enable I2C */
    I2C_Enable();
}

/*!
 * @brief       Delay   
 *
 * @param       count:  delay count
 *
 * @retval      None
 *
 * @note
 */
void Delay(uint32_t count)
{
    volatile uint32_t delay = count;
    
    while(delay--);
}

/*!
 * @brief       I2C interrupt service routine   
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void I2CIsr(void)
{
	/** ���յ���ַ��Ϣ��I2C�ӻ��ᷢ��һ��ACK�ź�;ACKEN=1Ҫ��ǰ���� */
    if(I2C_ReadStatusFlag(I2C_FLAG_ADDR) == SET)
    {
        /** Clear ADDR flag by reading STS3 register */
        (void)I2C->STS3;
    }
    
    /** STS1_RXBNEF = 1 and STS3_RWMF = 0 */
	/** ���յ��ֽ����ݺ�I2C�ӻ��ᷢ��һ��ACK�ź�;ACKEN=1Ҫ��ǰ���� */
    if((I2C_ReadStatusFlag(I2C_FLAG_RXBNE) == SET) && (I2C_ReadStatusFlag(I2C_FLAG_RWMF) == RESET))
    {
        if(dataBufPt < DATA_BUF_SIZE)
        {
            dataBuf[dataBufPt++] = I2C_RxData();
        }    
    }
    
    /** STS1_TXBEF = 1 and STS3_RWMF = 1 */
	/** �յ�I2C�������͵�ACK�źź�I2C�ӻ���TXBEλ��Ӳ����1*/
	/** ͨ����ַƥ������λ������д��������������I2C���豸��MMF��λ���*/
    if(I2C_ReadStatusFlag(I2C_FLAG_TXBE |I2C_FLAG_RWMF) == SET)  
    {
        if(dataBufPt < DATA_BUF_SIZE)
        {
            I2C_TxData(dataBuf[dataBufPt++]);
//			I2C->DATA = dataBuf[dataBufPt++];
        }    
    }
    
    /** Stop condition is detected */
	/** ֹͣ�ź����������͵�*/
    if(I2C_ReadStatusFlag(I2C_FLAG_STOP) == SET)
    {
        I2C->CTRL2 = I2C->CTRL2;
    } 
}

/*!
 * @brief       Data buffer init   
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void DataBufInit(void)
{
    uint8_t i;
    
    for(i = 0; i < DATA_BUF_SIZE; i++)
    {
        dataBuf[i] = 0;
    }
}
