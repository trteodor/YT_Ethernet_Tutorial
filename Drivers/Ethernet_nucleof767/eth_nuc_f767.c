
#include "eth_nuc_f767.h"
#include "GPIO_f7.h"

#ifndef NULL
#define NULL (void *)0
#endif

/** @defgroup PHY_Read_write_Timeouts 
  * @{
  */ 
#define PHY_READ_TO                     ((uint32_t)0x0004FFFF)
#define PHY_WRITE_TO                    ((uint32_t)0x0004FFFF)


ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB] __attribute__ ((aligned (4))); /* Ethernet Rx DMA Descriptor */
ETH_DMADESCTypeDef  DMATxDscrTab[ETH_TXBUFNB] __attribute__ ((aligned (4))); /* Ethernet Tx DMA Descriptor */
uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__ ((aligned (4))); /* Ethernet Receive Buffer */
uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__ ((aligned (4))); /* Ethernet Transmit Buffer */

/* Global pointers on Tx and Rx descriptor used to track transmit and receive descriptors */
__IO ETH_DMADESCTypeDef  *DMATxDescToSet;
__IO ETH_DMADESCTypeDef  *DMARxDescToGet;


/* Structure used to hold the last received packet descriptors info */
ETH_DMA_Rx_Frame_infos RX_Frame_Descriptor;
__IO ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;
__IO uint32_t Frame_Rx_index;




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

	/*ETH GPIO Configuration 	 * RM - section 43.2 page 1768*/

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






/**
  * @brief  Initializes the DMA Rx descriptors in chain mode.
  * @param  DMARxDescTab: Pointer on the first Rx desc list 
  * @param  RxBuff: Pointer on the first RxBuffer list
  * @param  RxBuffCount: Number of the used Rx desc in the list
  * @retval None
  */
void ETH_DMARxDescChainInit(ETH_DMADESCTypeDef *DMARxDescTab, uint8_t *RxBuff, uint32_t RxBuffCount)
{
  uint32_t i = 0;
  ETH_DMADESCTypeDef *DMARxDesc;
  
  /* Set the DMARxDescToGet pointer with the first one of the DMARxDescTab list */
  DMARxDescToGet = DMARxDescTab; 
  /* Fill each DMARxDesc descriptor with the right values */
  for(i=0; i < RxBuffCount; i++)
  {
    /* Get the pointer on the ith member of the Rx Desc list */
    DMARxDesc = DMARxDescTab+i;
    /* Set Own bit of the Rx descriptor Status */
    DMARxDesc->Status = ETH_DMARxDesc_OWN;

    /* Set Buffer1 size and Second Address Chained bit */
    DMARxDesc->ControlBufferSize = ETH_DMARxDesc_RCH | (uint32_t)ETH_RX_BUF_SIZE;  
    /* Set Buffer1 address pointer */
    DMARxDesc->Buffer1Addr = (uint32_t)(&RxBuff[i*ETH_RX_BUF_SIZE]);

    /* Initialize the next descriptor with the Next Descriptor Polling Enable */
    if(i < (RxBuffCount-1))
    {
      /* Set next descriptor address register with next descriptor base address */
      DMARxDesc->Buffer2NextDescAddr = (uint32_t)(DMARxDescTab+i+1); 
    }
    else
    {
      /* For last descriptor, set next descriptor address register equal to the first descriptor base address */ 
      DMARxDesc->Buffer2NextDescAddr = (uint32_t)(DMARxDescTab); 
    }
  }

  /* Set Receive Descriptor List Address Register */
  ETH->DMARDLAR = (uint32_t) DMARxDescTab;

  DMA_RX_FRAME_infos = &RX_Frame_Descriptor;
  
}



/**
  * @brief  Initializes the DMA Tx descriptors in chain mode.
  * @param  DMATxDescTab: Pointer on the first Tx desc list 
  * @param  TxBuff: Pointer on the first TxBuffer list
  * @param  TxBuffCount: Number of the used Tx desc in the list
  * @retval None
  */
void ETH_DMATxDescChainInit(ETH_DMADESCTypeDef *DMATxDescTab, uint8_t* TxBuff, uint32_t TxBuffCount)
{
  uint32_t i = 0;
  ETH_DMADESCTypeDef *DMATxDesc;
  
  /* Set the DMATxDescToSet pointer with the first one of the DMATxDescTab list */
  DMATxDescToSet = DMATxDescTab;
  /* Fill each DMATxDesc descriptor with the right values */   
  for(i=0; i < TxBuffCount; i++)
  {
    /* Get the pointer on the ith member of the Tx Desc list */
    DMATxDesc = DMATxDescTab + i;
    /* Set Second Address Chained bit */
    DMATxDesc->Status = ETH_DMATxDesc_TCH;  

    /* Set Buffer1 address pointer */
    DMATxDesc->Buffer1Addr = (uint32_t)(&TxBuff[i*ETH_TX_BUF_SIZE]);
    
    /* Initialize the next descriptor with the Next Descriptor Polling Enable */
    if(i < (TxBuffCount-1))
    {
      /* Set next descriptor address register with next descriptor base address */
      DMATxDesc->Buffer2NextDescAddr = (uint32_t)(DMATxDescTab+i+1);
    }
    else
    {
      /* For last descriptor, set next descriptor address register equal to the first descriptor base address */ 
      DMATxDesc->Buffer2NextDescAddr = (uint32_t) DMATxDescTab;  
    }
  }

  /* Set Transmit Desciptor List Address Register */
  ETH->DMATDLAR = (uint32_t) DMATxDescTab;
}



/**
  * @brief  Prepares DMA Tx descriptors to transmit an ethernet frame
  * @param  FrameLength : length of the frame to send
  * @retval error status
  */
uint32_t ETH_Prepare_Transmit_Descriptors(uint16_t FrameLength)
{   
  uint32_t buf_count =0, size=0,i=0;
  __IO ETH_DMADESCTypeDef *DMATxDesc;

  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (uint32_t)RESET)
  {  
    /* Return ERROR: OWN bit set */
    return ETH_ERROR;
  }

  DMATxDesc = DMATxDescToSet;
  
  if (FrameLength > ETH_TX_BUF_SIZE)
  {
    buf_count = FrameLength/ETH_TX_BUF_SIZE;
    if (FrameLength%ETH_TX_BUF_SIZE) buf_count++;
  }
  else buf_count =1;

  if (buf_count ==1)
  {
    /*set LAST and FIRST segment */
    DMATxDesc->Status |=ETH_DMATxDesc_FS|ETH_DMATxDesc_LS;
    /* Set frame size */
    DMATxDesc->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1);
    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMATxDesc->Status |= ETH_DMATxDesc_OWN;
    DMATxDesc= (ETH_DMADESCTypeDef *)(DMATxDesc->Buffer2NextDescAddr);
  }
  else
  {
    for (i=0; i< buf_count; i++)
    {
      /* Clear FIRST and LAST segment bits */
      DMATxDesc->Status &= ~(ETH_DMATxDesc_FS | ETH_DMATxDesc_LS);
      
      if (i==0) 
      {
        /* Setting the first segment bit */
        DMATxDesc->Status |= ETH_DMATxDesc_FS;  
      }

      /* Program size */
      DMATxDesc->ControlBufferSize = (ETH_TX_BUF_SIZE & ETH_DMATxDesc_TBS1);
      
      if (i== (buf_count-1))
      {
        /* Setting the last segment bit */
        DMATxDesc->Status |= ETH_DMATxDesc_LS;
        size = FrameLength - (buf_count-1)*ETH_TX_BUF_SIZE;
        DMATxDesc->ControlBufferSize = (size & ETH_DMATxDesc_TBS1);
      }

      /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
      DMATxDesc->Status |= ETH_DMATxDesc_OWN;

      DMATxDesc = (ETH_DMADESCTypeDef *)(DMATxDesc->Buffer2NextDescAddr);
    }
  }
  
  DMATxDescToSet = DMATxDesc;

  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if ((ETH->DMASR & ETH_DMASR_TBUS) != (uint32_t)RESET)
  {
    /* Clear TBUS ETHERNET DMA flag */
    ETH->DMASR = ETH_DMASR_TBUS;
    /* Resume DMA transmission*/
    ETH->DMATPDR = 0;
  }

  /* Return SUCCESS */
  return ETH_SUCCESS;   
}



/**
  * @brief  This function check for a frame avaibility
  * @param  None
  * @retval Returns OK when a frame pending, ERR if none.
  */
ETH_CallStatus_Type ETH_CheckFrameAvaibility(void)
{
    if(((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) == (uint32_t)RESET) &&
     ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET))
     {
      return OK;
     }
     return ERR;
}

/**
  * @brief  This function polls for a frame reception
  * @param  None
  * @retval Returns 1 when a frame is received, 0 if none.
  */
uint32_t ETH_GetReceivedFrame(void)
{
  /* check if last segment */
  if(((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) == (uint32_t)RESET) &&
     ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET)) 
  {
    DMA_RX_FRAME_infos->Seg_Count++;
    if (DMA_RX_FRAME_infos->Seg_Count == 1)
    {
      DMA_RX_FRAME_infos->FS_Rx_Desc = DMARxDescToGet;
    }
    DMA_RX_FRAME_infos->LS_Rx_Desc = DMARxDescToGet;
    return 1;
  }

  /* check if first segment */
  else if(((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) == (uint32_t)RESET) &&
          ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (uint32_t)RESET)&&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) == (uint32_t)RESET))
  {
    DMA_RX_FRAME_infos->FS_Rx_Desc = DMARxDescToGet;
    DMA_RX_FRAME_infos->LS_Rx_Desc = NULL;
    DMA_RX_FRAME_infos->Seg_Count = 1;
    DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);
  }

  /* check if intermediate segment */ 
  else if(((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) == (uint32_t)RESET) &&
          ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) == (uint32_t)RESET)&&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) == (uint32_t)RESET))
  {
    (DMA_RX_FRAME_infos->Seg_Count) ++;
    DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);
  } 
  return 0;
}



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
  if(ETH_WritePHYRegister(PHYAddress, PHY_BCR, PHY_Reset) == ERR)
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
      ETH_ReadPHYRegister(PHYAddress, PHY_BSR,&RegValue);
      timeout++;
    } while (!( RegValue & PHY_Linked_Status) && (timeout < PHY_READ_TO));

    /* Return ERROR in case of timeout */
    if(timeout == PHY_READ_TO)
    {
      err = ERR;
      goto error;
    }

    /* Reset Timeout counter */
    timeout = 0;
    /* Enable Auto-Negotiation */
    if(ETH_WritePHYRegister(PHYAddress, PHY_BCR, PHY_AutoNegotiation) == ERR)
    {
      /* Return ERROR in case of write timeout */
      err = ERR;
    }

    /* Wait until the auto-negotiation will be completed */
    do
    {
      timeout++;
      ETH_ReadPHYRegister(PHYAddress, PHY_BSR,&RegValue);

    } while (!( RegValue & PHY_AutoNego_Complete) && (timeout < (uint32_t)PHY_READ_TO));

    /* Return ERROR in case of timeout */
    if(timeout == PHY_READ_TO)
    {
      err = ERR;
      goto error;
    }

    /* Reset Timeout counter */
    timeout = 0;
    /* Read the result of the auto-negotiation */
    ETH_ReadPHYRegister(PHYAddress, PHY_SR,&RegValue);
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
    if(ETH_WritePHYRegister(PHYAddress, PHY_BCR, ((uint16_t)(ETH_InitStruct->ETH_Mode >> 3) |
                                                   (uint16_t)(ETH_InitStruct->ETH_Speed >> 1))) == ERR)
    {
      /* Return ERROR in case of write timeout */
       err = ERR;
    }
    /* Delay to assure PHY configuration */
    ETH_Delay(PHY_CONFIG_DELAY);
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
  ETH_Delay(ETH_REG_WRITE_DELAY);
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
  ETH_Delay(ETH_REG_WRITE_DELAY);
  ETH->MACFCR = tmpreg;

  /*----------------------- ETHERNET MACVLANTR Configuration -----------------*/
  /* Set the ETV bit according to ETH_VLANTagComparison value */
  /* Set the VL bit according to ETH_VLANTagIdentifier value */  
  ETH->MACVLANTR = (uint32_t)(ETH_InitStruct->ETH_VLANTagComparison | 
                              ETH_InitStruct->ETH_VLANTagIdentifier);

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->MACVLANTR;
  ETH_Delay(ETH_REG_WRITE_DELAY);
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
  ETH_Delay(ETH_REG_WRITE_DELAY);
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
  ETH_Delay(ETH_REG_WRITE_DELAY);
  ETH->DMABMR = tmpreg;

#ifdef USE_ENHANCED_DMA_DESCRIPTORS
  /* Enable the Enhanced DMA descriptors */
  ETH->DMABMR |= ETH_DMABMR_EDE;

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->DMABMR;
  ETH_Delay(ETH_REG_WRITE_DELAY);
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


/*!
 ************************************************************************************************
 * \brief ETH_WritePHYRegister
 * \details using this function user can check shall Link is connect
 * \return Status if "OK" Link connected, "ERR" if is disconnect
 * 
 * */
ETH_CallStatus_Type GetLinkState(void)
{
  uint32_t readval = 0;
  
  /* Read Status register  */
  if(ETH_ReadPHYRegister(LAN8742A_PHY_ADDRESS, PHY_BSR, &readval) != OK)
  {
    return ERR;
  }
  
  /* Read Status register again */
  if(ETH_ReadPHYRegister(LAN8742A_PHY_ADDRESS, PHY_BSR, &readval) != OK)
  {
    return ERR;
  }
  
  if(((readval & PHY_LINKED_STATUS) != PHY_LINKED_STATUS))
  {
    /* Return Link Down status */
    return ERR;    
  }

	return OK; //PHY_LINKED_STATUS should be TRUE
}
