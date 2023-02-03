
#include "eth_nuc_f767.h"
#include "GPIO_f7.h"




/** @defgroup PHY_Read_write_Timeouts 
  * @{
  */ 
#define PHY_READ_TO                     ((uint32_t)0x0004FFFF)
#define PHY_WRITE_TO                    ((uint32_t)0x0004FFFF)




/*!
 ************************************************************************************************
 * \brief ETH_gpio_rcc_init
 * \details This function initializes gpio and rcc in ethernet module
 *      , for nucleo-f767 board
 * \return None
 * */
void ETH_gpio_rcc_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/*ETH GPIO Configuration 	 * RM - section 43.2 page 1768

	/*
		RMII_REF_CLK ----------------------> PA1
		RMII_MDIO -------------------------> PA2
		RMII_MDC --------------------------> PC1
		RMII_MII_CRS_DV -------------------> PA7
		RMII_MII_RXD0 ---------------------> PC4
		RMII_MII_RXD1 ---------------------> PC5
		RMII_MII_TX_EN --------------------> PG11
		RMII_MII_TXD0 ---------------------> PG13
		RMII_MII_TXD1 ---------------------> PB13
	*/

	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOGEN);

	/* Configure PA1, PA2 and PA7 */
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;

	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure PB13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Configure PC1, PC4 and PC5 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configure PG11 and PG13 */
	GPIO_InitStruct.Pin =  GPIO_PIN_11 | GPIO_PIN_13;
	GPIO_Init(GPIOG, &GPIO_InitStruct);


	/*Turn on ETH Clocks*/
	RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACEN;
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_ETHMACRXEN);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_ETHMACTXEN);
}


/*!
 ************************************************************************************************
 * \brief ETH_WritePHYRegister
 * \details --
 * \param in PHYAddress - PHY address
 * \param in PHYReg - Reg id to be write
 * \param in RegValue - register value to write
 * \return Status of write, if "OK" then write process should be finished succed
 * 
 * */
ETH_CallStatus_Type ETH_WritePHYRegister(uint16_t PHYAddress, uint16_t PHYReg, uint32_t RegValue)
{
	uint32_t tmpreg    = 0;
    __IO uint32_t timeout = 0;

	tmpreg  = ETH->MACMIIAR;
	tmpreg &= ~MACMIIAR_CR_MASK;
	tmpreg |= ((uint32_t)((PHYAddress<<11) & ETH_MACMIIAR_PA));
	tmpreg |= (((uint32_t)PHYReg <<6)&ETH_MACMIIAR_MR);
	tmpreg |= ETH_MACMIIAR_MW;
	tmpreg |= ETH_MACMIIAR_MB;
	ETH->MACMIIDR = (uint16_t)RegValue;
	ETH->MACMIIAR  = tmpreg;

	while((tmpreg & ETH_MACMIIAR_MB) ==  ETH_MACMIIAR_MB)
	{
		if(timeout++ == PHY_READ_TO)
		{
			return (uint16_t)ERR;
		}
	}

	return OK;
}

/*!
 ************************************************************************************************
 * \brief ETH_ReadPHYRegister
 * \details --
 * \param in PHYAddress - PHY address
 * \param in PHYReg - Reg id to be read
 * \param out RegValueRet - register value: Data read from the selected PHY register (if )
 * \return Status of read, if "OK" then read process should be finished succed
 * */
ETH_CallStatus_Type ETH_ReadPHYRegister(uint16_t PHYAddress, uint16_t PHYReg, uint32_t *RegValueRet)
{
	uint32_t tmpreg    = 0;
    __IO uint32_t timeout = 0;

	tmpreg  = ETH->MACMIIAR;
	tmpreg &= ~MACMIIAR_CR_MASK;
	tmpreg |= ((uint32_t)((PHYAddress<<11) & ETH_MACMIIAR_PA));
	tmpreg |= (((uint32_t)PHYReg <<6)&ETH_MACMIIAR_MR);
    tmpreg &= ~ETH_MACMIIAR_MW;
	tmpreg |= ETH_MACMIIAR_MB;
	ETH->MACMIIAR  = tmpreg;

	/*Check busy flag*/
	while((tmpreg & ETH_MACMIIAR_MB) ==  ETH_MACMIIAR_MB)
	{
		if(timeout++ == PHY_READ_TO)
		{
			return (uint16_t)ERR;
		}
			tmpreg =  ETH->MACMIIAR;
	}

	*RegValueRet = (uint16_t) (ETH->MACMIIDR);

	return OK;
}


