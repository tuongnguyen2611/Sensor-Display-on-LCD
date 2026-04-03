/************************ INCLUDE LIBRARIES ************************/
#include "temhumsensor.h"
#include "Ucglib.h"
#include "timer.h"
#include "stm32f401re_i2c.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "stm32f401re_rcc.h"
#include "stm32f401re_gpio.h"
#include "kalman_filter.h"

/************************ GLOBAL VARIABLES ************************/
static ucg_t ucg;
static char g_strTemp[30] = "";
static char g_strHumi[30] = "";
//static double g_temp_filtered = 0;
//static double g_humi_filtered = 0;
static double g_temp_display = 0;
static double g_humi_display = 0;
static double g_temp_raw =0;
static double g_humi_raw =0;

static uint32_t time_initial;

/************************ MACROS ************************/

#define I2C_SPEED                400000  // 400 KBit/s

#define I2C_MASTER_RCC           RCC_APB1Periph_I2C1
#define I2C_MASTER_INSTANCE      I2C1

#define I2C_MASTER_GPIO_RCC      RCC_AHB1Periph_GPIOB
#define I2C_MASTER_PORT          GPIOB
#define SCL_MASTER_PIN           GPIO_Pin_8    // PB8
#define SDA_MASTER_PIN           GPIO_Pin_9    // PB9

#define SENSOR_ADDR              0x40
#define TEMP_CMDCODE             0xE3
#define HUMI_CMDCODE             0xE5

#define TIME_WAIT_GET_TEMP       8
#define TIME_WAIT_GET_HUMI       8

#define PERIOD_UPDATE_SENSOR     1000
#define PERIOD_UPDATE_LCD        5000

#define CHANGE_VALUE_TEMP        2.0
#define CHANGE_VALUE_HUMI        2.0

/************************ FUNCTION PROTOTYPES ************************/

void AppInit_Manager(void);
void delay_ms(uint32_t milisecond);
static void Sensor_Init(void);
static void I2C_Start(void);
static void I2C_Address_Direction(uint8_t Address, uint8_t Direction);
static void I2C_TransmitData(uint8_t data);
static void I2C_Stop(void);
static uint8_t I2C_Receive_ACK(void);
static uint8_t I2C_Receive_NACK(void);
static void TemHumSensor_readRegister(uint8_t AddrSensor,
                                      uint8_t pAddrReg,
                                      uint8_t *pDataRead,
                                      uint8_t byDataLen,
                                      uint16_t rDelay);
static double TemHumSensor_getTemp(void);
static double TemHumSensor_getHumi(void);
static void Update_Sensor_Value(void);
static void Update_LCD(void);

/************************ FUNCTION  ************************/
/*
 * @func:  		main
 * @brief:		Main program
 * @param:		None
 * @retval:		None
 * @note:		None
 */
int main(void)
{
	AppInit_Manager();

    while (1)
    {
        processTimerScheduler();
        Update_Sensor_Value();
        Update_LCD();
    }

    return 0;
}

/*
 * @func:      Sensor_Init
 *
 * @brief:     Initialize I2C peripheral and GPIO pins for communication with temperature-humidity sensor.
 *
 * @param:     None
 *
 * @retval:    None
 *
 * @note:      Configure PB8 (SCL) and PB9 (SDA) in alternate function open-drain mode.
 */
static void Sensor_Init(void)
{
    I2C_InitTypeDef I2C_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB1PeriphClockCmd(I2C_MASTER_RCC, ENABLE);
    RCC_AHB1PeriphClockCmd(I2C_MASTER_GPIO_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = SCL_MASTER_PIN;
    GPIO_Init(I2C_MASTER_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SDA_MASTER_PIN;
    GPIO_Init(I2C_MASTER_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(I2C_MASTER_PORT, GPIO_PinSource8, GPIO_AF_I2C1);
    GPIO_PinAFConfig(I2C_MASTER_PORT, GPIO_PinSource9, GPIO_AF_I2C1);

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;

    I2C_Init(I2C_MASTER_INSTANCE, &I2C_InitStructure);
    I2C_Cmd(I2C_MASTER_INSTANCE, ENABLE);
}

/*
 * @func:      I2C_Start
 *
 * @brief:     Generate I2C START condition and wait until master mode is selected.
 *
 * @param:     None
 *
 * @retval:    None
 *
 * @note:      Blocking function.
 */
static void I2C_Start(void)
{
    while (I2C_GetFlagStatus(I2C_MASTER_INSTANCE, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_MASTER_INSTANCE, ENABLE);
    while (!I2C_CheckEvent(I2C_MASTER_INSTANCE, I2C_EVENT_MASTER_MODE_SELECT));
}

/*
 * @func:      I2C_Address_Direction
 *
 * @brief:     Send slave address with read/write direction.
 *
 * @param:     Address - 7-bit slave address
 * @param:     Direction - I2C_Direction_Transmitter or I2C_Direction_Receiver
 *
 * @retval:    None
 *
 * @note:      Waits for corresponding I2C event.
 */

static void I2C_Address_Direction(uint8_t Address, uint8_t Direction)
{
    I2C_Send7bitAddress(I2C_MASTER_INSTANCE, Address, Direction);
    if (Direction == I2C_Direction_Transmitter)
    {
        while (!I2C_CheckEvent(I2C_MASTER_INSTANCE, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    }
    else if (Direction == I2C_Direction_Receiver)
    {
        while (!I2C_CheckEvent(I2C_MASTER_INSTANCE, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }
}

/*
 * @func:      I2C_TransmitData
 *
 * @brief:     Transmit one byte of data via I2C.
 *
 * @param:     data - byte to be transmitted
 *
 * @retval:    None
 *
 * @note:      Blocking until transmission complete.
 */
static void I2C_TransmitData(uint8_t data)
{
    I2C_SendData(I2C_MASTER_INSTANCE, data);
    while (!I2C_CheckEvent(I2C_MASTER_INSTANCE, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/*
 * @func:      I2C_Stop
 *
 * @brief:     Generate I2C STOP condition.
 *
 * @param:     None
 *
 * @retval:    None
 *
 * @note:      Ends I2C communication.
 */
static void I2C_Stop(void)
{
    I2C_GenerateSTOP(I2C_MASTER_INSTANCE, ENABLE);
}

/*
 * @func:      I2C_Receive_NACK
 *
 * @brief:     Receive one byte from I2C and send NACK.
 *
 * @param:     None
 *
 * @retval:    Received data byte
 *
 * @note:      Used for the last byte.
 */
static uint8_t I2C_Receive_NACK(void)
{
    uint8_t data_receive;
    I2C_AcknowledgeConfig(I2C_MASTER_INSTANCE, DISABLE);
    while (!I2C_CheckEvent(I2C_MASTER_INSTANCE, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data_receive = I2C_ReceiveData(I2C_MASTER_INSTANCE);
    return data_receive;
}

/*
 * @func:      I2C_Receive_ACK
 *
 * @brief:     Receive one byte from I2C and send ACK.
 *
 * @param:     None
 *
 * @retval:    Received data byte
 *
 * @note:      Used when more bytes are expected.
 */

static uint8_t I2C_Receive_ACK(void)
{
    uint8_t data_receive;
    I2C_AcknowledgeConfig(I2C_MASTER_INSTANCE, ENABLE);
    while (!I2C_CheckEvent(I2C_MASTER_INSTANCE, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data_receive = I2C_ReceiveData(I2C_MASTER_INSTANCE);
    return data_receive;
}

/*
 * @func:      TemHumSensor_readRegister
 *
 * @brief:     Read data from sensor register via I2C.
 *
 * @param:     AddrSensor - Sensor I2C address
 * @param:     pAddrReg   - Register/command code
 * @param:     pDataRead  - Buffer to store data
 * @param:     byDataLen  - Number of bytes to read
 * @param:     rDelay     - Delay before reading (ms)
 *
 * @retval:    None
 *
 * @note:      Implements write-then-read sequence.
 */

static void TemHumSensor_readRegister(uint8_t AddrSensor, uint8_t pAddrReg, uint8_t *pDataRead, uint8_t byDataLen, uint16_t rDelay)
{
    I2C_Start();
    I2C_Address_Direction(AddrSensor << 1, I2C_Direction_Transmitter);
    I2C_TransmitData(pAddrReg);

    if (rDelay > 0) delay_ms(rDelay);

    I2C_Stop();

    I2C_Start();
    I2C_Address_Direction(AddrSensor << 1, I2C_Direction_Receiver);

    for (uint8_t i = 0; i < byDataLen; i++)
    {
        if (i == byDataLen - 1)
            pDataRead[i] = I2C_Receive_NACK();
        else
            pDataRead[i] = I2C_Receive_ACK();
    }
    I2C_Stop();
}

/*
 * @func:      TemHumSensor_getTemp
 *
 * @brief:     Read raw temperature data and convert to Celsius.
 *
 * @param:     None
 *
 * @retval:    Temperature value (°C)
 *
 * @note:      Uses formula from sensor datasheet.
 */
static double TemHumSensor_getTemp(void)
{
    double temperature = 0;
    uint8_t strTemp[2] = {0};
    TemHumSensor_readRegister(SENSOR_ADDR, TEMP_CMDCODE, strTemp, 2, TIME_WAIT_GET_TEMP);
    uint16_t tempCode = (uint16_t)(strTemp[0] << 8) | strTemp[1];
    temperature = (double)((175.72 * tempCode) / 65536 - 46.85);
    return temperature;
}


/*
 * @func:      TemHumSensor_getHumi
 *
 * @brief:     Read raw humidity data and convert to percentage.
 *
 * @param:     None
 *
 * @retval:    Relative humidity (%)
 *
 * @note:      Uses formula from sensor datasheet.
 */
static double TemHumSensor_getHumi(void)
{
    double humidity = 0;
    uint8_t strHumi[2] = {0};
    TemHumSensor_readRegister(SENSOR_ADDR, HUMI_CMDCODE, strHumi, 2, TIME_WAIT_GET_HUMI);
    uint16_t RHCode = (uint16_t)((strHumi[0] << 8) | strHumi[1]);
    humidity = (double)((125 * RHCode) / 65536 - 6);
    return humidity;
}

/*
 * @func:      Update_Sensor_Value
 *
 * @brief:     Periodically read temperature and humidity from sensor.
 *
 * @param:     None
 *
 * @retval:    None
 *
 * @note:      Sampling interval controlled by PERIOD_UPDATE_SENSOR.
 */

static void Update_Sensor_Value(void)
{
    static uint32_t TimeLastSample = 0;
    uint32_t TimeCurrent = GetMilSecTick();

    if ((TimeCurrent - TimeLastSample) >= PERIOD_UPDATE_SENSOR)
    {
        g_temp_raw = TemHumSensor_getTemp();
//        g_temp_filtered = KalmanFilter_updateEstimate(temp_raw);

//        KalmanFilterInit(0.1, 0.1, 0.1); // reset for humidity

        g_humi_raw = TemHumSensor_getHumi();
//        g_humi_filtered = KalmanFilter_updateEstimate(humi_raw);

        TimeLastSample = TimeCurrent;
    }
}

/*
 * @func:      Update_LCD
 *
 * @brief:     Update temperature and humidity values on LCD screen.
 *
 * @param:     None
 *
 * @retval:    None
 *
 * @note:      Display updated every PERIOD_UPDATE_LCD.
 */

static void Update_LCD(void)
{
    static uint32_t TimeLastDisplay = 0;
    uint32_t TimeCurrent = GetMilSecTick();


    if ((TimeCurrent - TimeLastDisplay) < PERIOD_UPDATE_LCD)
        return;


    if (fabs(g_temp_raw - g_temp_display) >= CHANGE_VALUE_TEMP)
        g_temp_display = g_temp_raw;

    if (fabs(g_humi_raw - g_humi_display) >= CHANGE_VALUE_HUMI)
        g_humi_display = g_humi_raw;


    snprintf(g_strTemp, sizeof(g_strTemp), "Temp: %.2f oC", g_temp_display);
    snprintf(g_strHumi, sizeof(g_strHumi), "Humi: %.2f %%", g_humi_display);

    ucg_ClearScreen(&ucg);

    ucg_SetColor(&ucg, 0, 255, 255, 255);
    ucg_DrawString(&ucg, 10, 25, 0, "Assignment 2");
    ucg_DrawString(&ucg, 10, 50, 0, g_strTemp);
    ucg_DrawString(&ucg, 10, 75, 0, g_strHumi);

    TimeLastDisplay = TimeCurrent;
}

/*
 * @func:      delay_ms
 *
 * @brief:     Create blocking delay in milliseconds.
 *
 * @param:     milisecond - delay duration
 *
 * @retval:    None
 *
 * @note:      Uses simple loop (not accurate for precise timing).
 */
void delay_ms(uint32_t milisecond)
{
    for (uint32_t i = 0; i < milisecond; i++)
    {
        for (uint32_t j = 0; j < 5000; j++);
    }
}

/*
 * @func:      AppInit_Manager
 *
 * @brief:     Initialize system clock, timer, sensor (I2C), Kalman filter and LCD.
 *
 * @param:     None
 *
 * @retval:    None
 *
 * @note:      Must be called once at the beginning of main().
 */
void AppInit_Manager(void)
{
	SystemCoreClockUpdate();
	TimerInit();
	Sensor_Init();

	KalmanFilterInit(0.1, 0.1, 0.1);

	Ucglib4WireSWSPI_begin(&ucg, UCG_FONT_MODE_SOLID);
	ucg_ClearScreen(&ucg);
	ucg_SetFont(&ucg, ucg_font_ncenR10_hf);
	ucg_SetColor(&ucg, 0, 255, 255, 255);
	ucg_SetColor(&ucg, 1, 0, 0, 0);
	ucg_SetRotate180(&ucg);
	time_initial = GetMilSecTick();
}


