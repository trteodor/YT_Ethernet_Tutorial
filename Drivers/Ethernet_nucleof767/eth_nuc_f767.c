
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
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
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
ETH_CallStatus_Type  ETH_ReadPHYRegister(uint16_t PHYAddress, uint16_t PHYReg, uint32_t *RegValueRet)
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


/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void ETH_Delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = nCount; index != 0; index--)
  {
  }
}

/*!
 ************************************************************************************************
 * \brief ETH_ReadPHYRegister
 * \details --
 * \param RingBuffer_t *Buf 
 * \param out MessageSize
 * \param out MessagePointer
 *
 * */
/**
  * @brief  Initializes the ETHERNET peripheral according to the specified
  *   parameters in the ETH_InitStruct .
  * @param ETH_InitStruct: pointer to a ETH_InitTypeDef structure that contains
  *   the configuration information for the specified ETHERNET peripheral.
  * @param PHYAddress: external PHY address
  * @retval ERR: Ethernet initialization failed
  *         OK: Ethernet successfully initialized
  */


ETH_CallStatus_Type ETH_Init(ETH_InitTypeDef* ETH_InitStruct, uint16_t PHYAddress)
{
  uint32_t RegValue = 0, tmpreg = 0;
  __IO uint32_t i = 0;
  uint32_t  rcc_clocks;
  uint32_t hclk = SYSTEM_CORE_CLOCK_HZ_VALUE;
  __IO uint32_t timeout = 0, err = OK;

  /*-------------------------------- MAC Config ------------------------------*/
  /*---------------------- ETHERNET MACMIIAR Configuration -------------------*/
  /* Get the ETHERNET MACMIIAR value */
  tmpreg = ETH->MACMIIAR;
  /* Clear CSR Clock Range CR[2:0] bits */
  tmpreg &= MACMIIAR_CR_MASK;

  /* Set CR bits depending on hclk value */
  if((hclk >= 20000000)&&(hclk < 35000000))
  {
    /* CSR Clock Range between 20-35 MHz */
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div16;
  }
  else if((hclk >= 35000000)&&(hclk < 60000000))
  {
    /* CSR Clock Range between 35-60 MHz */
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div26;
  }
  else if((hclk >= 60000000)&&(hclk < 100000000))
  {
    /* CSR Clock Range between 60-100 MHz */
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div42;
  }
  else if((hclk >= 100000000)&&(hclk < 150000000))
  {
    /* CSR Clock Range between 100-150 MHz */
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div62;
  }
  else
  {
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div102;
  }

  /* Write to ETHERNET MAC MIIAR: Configure the ETHERNET CSR Clock Range */
  ETH->MACMIIAR = (uint32_t)tmpreg;
  /*-------------------- PHY initialization and configuration ----------------*/
  /* Put the PHY in reset mode */
  if(!(ETH_WritePHYRegister(PHYAddress, PHY_BCR, PHY_Reset)))
  {
    /* Return ERROR in case of write timeout */
    err = ERR;
    goto error;
  }

  /* Delay to assure PHY reset */
  ETH_Delay(PHY_RESET_DELAY);

  if(ETH_InitStruct->ETH_AutoNegotiation != ETH_AutoNegotiation_Disable)
  {
    /* We wait for linked status...*/
    do
    {
      timeout++;
    } while (!(ETH_ReadPHYRegister(PHYAddress, PHY_BSR) & PHY_Linked_Status) && (timeout < PHY_READ_TO));

    /* Return ERROR in case of timeout */
    if(timeout == PHY_READ_TO)
    {
      err = ERR;
      goto error;
    }

    /* Reset Timeout counter */
    timeout = 0;
    /* Enable Auto-Negotiation */
    if(!(ETH_WritePHYRegister(PHYAddress, PHY_BCR, PHY_AutoNegotiation)))
    {
      /* Return ERROR in case of write timeout */
      err = ERR;
    }

    /* Wait until the auto-negotiation will be completed */
    do
    {
      timeout++;
    } while (!(ETH_ReadPHYRegister(PHYAddress, PHY_BSR) & PHY_AutoNego_Complete) && (timeout < (uint32_t)PHY_READ_TO));

    /* Return ERROR in case of timeout */
    if(timeout == PHY_READ_TO)
    {
      err = ERR;
      goto error;
    }

    /* Reset Timeout counter */
    timeout = 0;
    /* Read the result of the auto-negotiation */
    RegValue = ETH_ReadPHYRegister(PHYAddress, PHY_SR);
    /* Configure the MAC with the Duplex Mode fixed by the auto-negotiation process */
    if((RegValue & PHY_DUPLEX_STATUS) != (uint32_t)RESET)
    {
      /* Set Ethernet duplex mode to Full-duplex following the auto-negotiation */
      ETH_InitStruct->ETH_Mode = ETH_Mode_FullDuplex;
    }
    else
    {
      /* Set Ethernet duplex mode to Half-duplex following the auto-negotiation */
      ETH_InitStruct->ETH_Mode = ETH_Mode_HalfDuplex;
    }
    /* Configure the MAC with the speed fixed by the auto-negotiation process */
    if(RegValue & PHY_SPEED_STATUS)
    {
      /* Set Ethernet speed to 10M following the auto-negotiation */
      ETH_InitStruct->ETH_Speed = ETH_Speed_10M;
    }
    else
    {
      /* Set Ethernet speed to 100M following the auto-negotiation */
      ETH_InitStruct->ETH_Speed = ETH_Speed_100M;
    }
  }
  else
  {
    if(!ETH_WritePHYRegister(PHYAddress, PHY_BCR, ((uint16_t)(ETH_InitStruct->ETH_Mode >> 3) |
                                                   (uint16_t)(ETH_InitStruct->ETH_Speed >> 1))))
    {
      /* Return ERROR in case of write timeout */
       err = ERR;
    }
    /* Delay to assure PHY configuration */
    _eth_delay_(PHY_CONFIG_DELAY);
  }
error:
  if (err == ERR) /* Auto-negotiation failed */
  {
    /* Set Ethernet duplex mode to Full-duplex */
    ETH_InitStruct->ETH_Mode = ETH_Mode_FullDuplex;

    /* Set Ethernet speed to 100M */
    ETH_InitStruct->ETH_Speed = ETH_Speed_100M;
  }
 
  /*----------------------- ETHERNET MACFFR Configuration --------------------*/ 
  ETH->MACFFR = (uint32_t)(ETH_InitStruct->ETH_ReceiveAll | ETH_InitStruct->ETH_SourceAddrFilter |
                           ETH_InitStruct->ETH_PassControlFrames | ETH_InitStruct->ETH_BroadcastFramesReception | 
                           ETH_InitStruct->ETH_DestinationAddrFilter | ETH_InitStruct->ETH_PromiscuousMode |
                           ETH_InitStruct->ETH_MulticastFramesFilter |
                           ETH_InitStruct->ETH_UnicastFramesFilter);

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->MACFFR;
  _eth_delay_(ETH_REG_WRITE_DELAY);
  ETH->MACFFR = tmpreg;

  /*--------------- ETHERNET MACHTHR and MACHTLR Configuration ---------------*/
  /* Write to ETHERNET MACHTHR */
  ETH->MACHTHR = (uint32_t)ETH_InitStruct->ETH_HashTableHigh;

  /* Write to ETHERNET MACHTLR */
  ETH->MACHTLR = (uint32_t)ETH_InitStruct->ETH_HashTableLow;
  /*----------------------- ETHERNET MACFCR Configuration --------------------*/

  /* Get the ETHERNET MACFCR value */  
  tmpreg = ETH->MACFCR;
  /* Clear xx bits */
  tmpreg &= MACFCR_CLEAR_MASK;

  tmpreg |= (uint32_t)((ETH_InitStruct->ETH_PauseTime << 16) | ETH_InitStruct->ETH_ZeroQuantaPause |
                        ETH_InitStruct->ETH_PauseLowThreshold | ETH_InitStruct->ETH_UnicastPauseFrameDetect | 
                        ETH_InitStruct->ETH_ReceiveFlowControl | ETH_InitStruct->ETH_TransmitFlowControl); 
  /* Write to ETHERNET MACFCR */
  ETH->MACFCR = (uint32_t)tmpreg;
  
  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->MACFCR;
  _eth_delay_(ETH_REG_WRITE_DELAY);
  ETH->MACFCR = tmpreg;

  /*----------------------- ETHERNET MACVLANTR Configuration -----------------*/
  /* Set the ETV bit according to ETH_VLANTagComparison value */
  /* Set the VL bit according to ETH_VLANTagIdentifier value */  
  ETH->MACVLANTR = (uint32_t)(ETH_InitStruct->ETH_VLANTagComparison | 
                              ETH_InitStruct->ETH_VLANTagIdentifier);

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->MACVLANTR;
  _eth_delay_(ETH_REG_WRITE_DELAY);
  ETH->MACVLANTR = tmpreg;

  /*-------------------------------- DMA Config ------------------------------*/
  /*----------------------- ETHERNET DMAOMR Configuration --------------------*/

  /* Get the ETHERNET DMAOMR value */
  tmpreg = ETH->DMAOMR;
  /* Clear xx bits */
  tmpreg &= DMAOMR_CLEAR_MASK;

  tmpreg |= (uint32_t)(ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame | 
                       ETH_InitStruct->ETH_ReceiveStoreForward | ETH_InitStruct->ETH_FlushReceivedFrame |
                       ETH_InitStruct->ETH_TransmitStoreForward | ETH_InitStruct->ETH_TransmitThresholdControl |
                       ETH_InitStruct->ETH_ForwardErrorFrames | ETH_InitStruct->ETH_ForwardUndersizedGoodFrames |
                       ETH_InitStruct->ETH_ReceiveThresholdControl |
                       ETH_InitStruct->ETH_SecondFrameOperate); 
  /* Write to ETHERNET DMAOMR */
  ETH->DMAOMR = (uint32_t)tmpreg;

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->DMAOMR;
  _eth_delay_(ETH_REG_WRITE_DELAY);
  ETH->DMAOMR = tmpreg;

  /*----------------------- ETHERNET DMABMR Configuration --------------------*/
  ETH->DMABMR = (uint32_t)(ETH_InitStruct->ETH_AddressAlignedBeats | ETH_InitStruct->ETH_FixedBurst |
                           ETH_InitStruct->ETH_RxDMABurstLength | /* !! if 4xPBL is selected for Tx or Rx it is applied for the other */
                           ETH_InitStruct->ETH_TxDMABurstLength | (ETH_InitStruct->ETH_DescriptorSkipLength << 2) |
                           ETH_InitStruct->ETH_DMAArbitration |
                           ETH_DMABMR_USP); /* Enable use of separate PBL for Rx and Tx */

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->DMABMR;
  _eth_delay_(ETH_REG_WRITE_DELAY);
  ETH->DMABMR = tmpreg;

#ifdef USE_ENHANCED_DMA_DESCRIPTORS
  /* Enable the Enhanced DMA descriptors */
  ETH->DMABMR |= ETH_DMABMR_EDE;

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->DMABMR;
  _eth_delay_(ETH_REG_WRITE_DELAY);
  ETH->DMABMR = tmpreg;
#endif /* USE_ENHANCED_DMA_DESCRIPTORS */

  /* Return Ethernet configuration success */
  if(err == OK)
  {
    /* Return Ethernet configuration success */
    return OK;
  }
  else /* Auto-negotiation failed */
  {
    /* Return Ethernet error */
    return ERR;
  }
}