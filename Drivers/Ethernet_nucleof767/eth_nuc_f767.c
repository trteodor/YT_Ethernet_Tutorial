#include "System.h"
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

	/*Enable SYSCFG*/
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	/*Select RMII*/
	SYSCFG->PMC &=~(SYSCFG_PMC_MII_RMII_SEL);
	SYSCFG->PMC |=SYSCFG_PMC_MII_RMII_SEL;
 
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
    tmpreg =  ETH->MACMIIAR;
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
  DelayMs(nCount);
  // __IO uint32_t index = 0; 
  // for(index = nCount; index != 0; index--)
  // {
  // }
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
  * @brief  This function should be called to get the received frame (to be used 
  *   with polling method only).
  * @param  none
  * @retval Structure of type FrameTypeDef
  */
FrameTypeDef ETH_Get_Received_Frame(void)
{ 
  uint32_t framelength = 0;
  FrameTypeDef frame = {0,0,0}; 

  /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
  framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4;
  frame.length = framelength;

  /* Get the address of the first frame descriptor and the buffer start address */ 
  frame.descriptor = DMA_RX_FRAME_infos->FS_Rx_Desc;
  frame.buffer =(DMA_RX_FRAME_infos->FS_Rx_Desc)->Buffer1Addr;

  /* Update the ETHERNET DMA global Rx descriptor with next Rx descriptor */
  /* Chained Mode */
  /* Selects the next DMA Rx descriptor list for next buffer to read */ 
  DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);

  /* Return Frame */
  return (frame);
}


/**
  * @brief  This function check for a frame avaibility
  * @param  None
  * @retval Returns OK when a frame pending, ERR if none.
  */
ETH_CallStatus_Type ETH_CheckFrameAvaibility(void)
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
    return OK;
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
  return ERR;
}


/**
  * @brief  Selects the specified ETHERNET DMA Tx Desc Checksum Insertion.
  * @param  DMATxDesc: pointer on a DMA Tx descriptor 
  * @param  DMATxDesc_Checksum: specifies is the DMA Tx desc checksum insertion.
  *   This parameter can be one of the following values:
  *     @arg ETH_DMATxDesc_ChecksumByPass : Checksum bypass
  *     @arg ETH_DMATxDesc_ChecksumIPV4Header : IPv4 header checksum
  *     @arg ETH_DMATxDesc_ChecksumTCPUDPICMPSegment : TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present
  *     @arg ETH_DMATxDesc_ChecksumTCPUDPICMPFull : TCP/UDP/ICMP checksum fully in hardware including pseudo header                                                                
  * @retval None
  */
void ETH_DMATxDescChecksumInsertionConfig(ETH_DMADESCTypeDef *DMATxDesc, uint32_t DMATxDesc_Checksum)
{
  /* Set the selected DMA Tx desc checksum insertion control */
  DMATxDesc->Status |= DMATxDesc_Checksum;
}

/**
  * @brief  Configures the selected MAC address.
  * @param  MacAddr: The MAC address to configure.
  *   This parameter can be one of the following values:
  *     @arg ETH_MAC_Address0 : MAC Address0 
  *     @arg ETH_MAC_Address1 : MAC Address1 
  *     @arg ETH_MAC_Address2 : MAC Address2
  *     @arg ETH_MAC_Address3 : MAC Address3
  * @param  Addr: Pointer on MAC address buffer data (6 bytes).
  * @retval None
  */
void ETH_MACAddressConfig(uint32_t MacAddr, uint8_t *Addr)
{
  uint32_t tmpreg;
  /* Check the parameters */
  /* Calculate the selected MAC address high register */
  tmpreg = ((uint32_t)Addr[5] << 8) | (uint32_t)Addr[4];
  /* Load the selected MAC address high register */
  (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) = tmpreg;
  /* Calculate the selected MAC address low register */
  tmpreg = ((uint32_t)Addr[3] << 24) | ((uint32_t)Addr[2] << 16) | ((uint32_t)Addr[1] << 8) | Addr[0];
  /* Load the selected MAC address low register */
  (*(__IO uint32_t *) (ETH_MAC_ADDR_LBASE + MacAddr)) = tmpreg;
}



/**
  * @brief  Fills each ETH_InitStruct member with its default value.
  * @param  ETH_InitStruct: pointer to a ETH_InitTypeDef structure which will be initialized.
  * @retval None
  */
void ETH_StructInit(ETH_InitTypeDef* ETH_InitStruct)
{
  /* ETH_InitStruct members default value */
  /*------------------------   MAC Configuration   ---------------------------*/
  
  /* PHY Auto-negotiation enabled */
  ETH_InitStruct->ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
  /* MAC watchdog enabled: cuts off long frame */
  ETH_InitStruct->ETH_Watchdog = ETH_Watchdog_Enable;
  /* MAC Jabber enabled in Half-duplex mode */
  ETH_InitStruct->ETH_Jabber = ETH_Jabber_Enable;
  /* Ethernet interframe gap set to 96 bits */
  ETH_InitStruct->ETH_InterFrameGap = ETH_InterFrameGap_96Bit;
  /* Carrier Sense Enabled in Half-Duplex mode */
  ETH_InitStruct->ETH_CarrierSense = ETH_CarrierSense_Enable;
  /* PHY speed configured to 100Mbit/s */
  ETH_InitStruct->ETH_Speed = ETH_Speed_100M; 
  /* Receive own Frames in Half-Duplex mode enabled */
  ETH_InitStruct->ETH_ReceiveOwn = ETH_ReceiveOwn_Enable;
  /* MAC MII loopback disabled */ 
  ETH_InitStruct->ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  /* Full-Duplex mode selected */
  ETH_InitStruct->ETH_Mode = ETH_Mode_FullDuplex;
  /* IPv4 and TCP/UDP/ICMP frame Checksum Offload disabled */
  ETH_InitStruct->ETH_ChecksumOffload = ETH_ChecksumOffload_Disable;
  /* Retry Transmission enabled for half-duplex mode */ 
  ETH_InitStruct->ETH_RetryTransmission = ETH_RetryTransmission_Enable;
  /* Automatic PAD/CRC strip disabled*/
  ETH_InitStruct->ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  /* half-duplex mode retransmission Backoff time_limit = 10 slot times*/ 
  ETH_InitStruct->ETH_BackOffLimit = ETH_BackOffLimit_10;
  /* half-duplex mode Deferral check disabled */
  ETH_InitStruct->ETH_DeferralCheck = ETH_DeferralCheck_Disable;
  /* Receive all frames disabled */
  ETH_InitStruct->ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  /* Source address filtering (on the optional MAC addresses) disabled */
  ETH_InitStruct->ETH_SourceAddrFilter = ETH_SourceAddrFilter_Disable;
  /* Do not forward control frames that do not pass the address filtering */
  ETH_InitStruct->ETH_PassControlFrames = ETH_PassControlFrames_BlockAll;
  /* Disable reception of Broadcast frames */
  ETH_InitStruct->ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable;
  /* Normal Destination address filtering (not reverse addressing) */
  ETH_InitStruct->ETH_DestinationAddrFilter = ETH_DestinationAddrFilter_Normal;
  /* Promiscuous address filtering mode disabled */
  ETH_InitStruct->ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  /* Perfect address filtering for multicast addresses */
  ETH_InitStruct->ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  /* Perfect address filtering for unicast addresses */
  ETH_InitStruct->ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
  /* Initialize hash table high and low regs */
  ETH_InitStruct->ETH_HashTableHigh = 0x0;
  ETH_InitStruct->ETH_HashTableLow = 0x0;
  /* Flow control config (flow control disabled)*/
  ETH_InitStruct->ETH_PauseTime = 0x0;
  ETH_InitStruct->ETH_ZeroQuantaPause = ETH_ZeroQuantaPause_Disable;
  ETH_InitStruct->ETH_PauseLowThreshold = ETH_PauseLowThreshold_Minus4;
  ETH_InitStruct->ETH_UnicastPauseFrameDetect = ETH_UnicastPauseFrameDetect_Disable;
  ETH_InitStruct->ETH_ReceiveFlowControl = ETH_ReceiveFlowControl_Disable;
  ETH_InitStruct->ETH_TransmitFlowControl = ETH_TransmitFlowControl_Disable;
  /* VLANtag config (VLAN field not checked) */
  ETH_InitStruct->ETH_VLANTagComparison = ETH_VLANTagComparison_16Bit;
  ETH_InitStruct->ETH_VLANTagIdentifier = 0x0;
  
  /*---------------------- DMA Configuration   -------------------------------*/
  
  /* Drops frames with with TCP/IP checksum errors */
  ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Disable; 
  /* Store and forward mode enabled for receive */
  ETH_InitStruct->ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
  /* Flush received frame that created FIFO overflow */
  ETH_InitStruct->ETH_FlushReceivedFrame = ETH_FlushReceivedFrame_Enable;
  /* Store and forward mode enabled for transmit */
  ETH_InitStruct->ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;  
  /* Threshold TXFIFO level set to 64 bytes (used when threshold mode is enabled) */
  ETH_InitStruct->ETH_TransmitThresholdControl = ETH_TransmitThresholdControl_64Bytes;
  /* Disable forwarding frames with errors (short frames, CRC,...)*/
  ETH_InitStruct->ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
  /* Disable undersized good frames */
  ETH_InitStruct->ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
  /* Threshold RXFIFO level set to 64 bytes (used when Cut through mode is enabled) */
  ETH_InitStruct->ETH_ReceiveThresholdControl = ETH_ReceiveThresholdControl_64Bytes;
  /* Disable Operate on second frame (transmit a second frame to FIFO without 
  waiting status of previous frame*/
  ETH_InitStruct->ETH_SecondFrameOperate = ETH_SecondFrameOperate_Disable;
  /* DMA works on 32-bit aligned start source and destinations addresses */
  ETH_InitStruct->ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
  /* Enabled Fixed Burst Mode (mix of INC4, INC8, INC16 and SINGLE DMA transactions */
  ETH_InitStruct->ETH_FixedBurst = ETH_FixedBurst_Enable;
  /* DMA transfer max burst length = 32 beats = 32 x 32bits */
  ETH_InitStruct->ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
  ETH_InitStruct->ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  /* DMA Ring mode skip length = 0 */
  ETH_InitStruct->ETH_DescriptorSkipLength = 0x0; 
  /* Equal priority (round-robin) between transmit and receive DMA engines */
  ETH_InitStruct->ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_1_1;
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

	/*Software reset ETH MAC_DMA*/
	ETH->DMABMR |=ETH_DMABMR_SR;

	timeout =  GetSysTime();

	/*Wait for software reset*/
	while((ETH->DMABMR  & ETH_DMABMR_SR) != (uint32_t)RESET)
	{
		/*Check for timeout*/
		if((GetSysTime() - timeout) >  ETH_TIMEOUT_SWRESET)
		{
      break; /*dk how to handle*/
		}
	}


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
 

   /*------------------------ ETHERNET MACCR Configuration --------------------*/
    /* Get the ETHERNET MACCR value */
    tmpreg = ETH->MACCR;
    /* Clear WD, PCE, PS, TE and RE bits */
    tmpreg &= MACCR_CLEAR_MASK;

    tmpreg |= (uint32_t)(ETH_InitStruct->ETH_Watchdog | ETH_InitStruct->ETH_Jabber | 
                          ETH_InitStruct->ETH_InterFrameGap |ETH_InitStruct->ETH_CarrierSense |
                          ETH_InitStruct->ETH_Speed | ETH_InitStruct->ETH_ReceiveOwn |
                          ETH_InitStruct->ETH_LoopbackMode | ETH_InitStruct->ETH_Mode | 
                          ETH_InitStruct->ETH_ChecksumOffload | ETH_InitStruct->ETH_RetryTransmission | 
                          ETH_InitStruct->ETH_AutomaticPadCRCStrip | ETH_InitStruct->ETH_BackOffLimit | 
                          ETH_InitStruct->ETH_DeferralCheck);
    /* Write to ETHERNET MACCR */
    ETH->MACCR = (uint32_t)tmpreg;
    
  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
    tmpreg = ETH->MACCR;
    ETH_Delay(ETH_REG_WRITE_DELAY);
    ETH->MACCR = tmpreg; 
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
ETH_CallStatus_Type GetLinkState(uint16_t PHYAddress)
{
  uint32_t readval = 0;
  
  /* Read Status register  */
  if(ETH_ReadPHYRegister(PHYAddress, PHY_BSR, &readval) != OK)
  {
    return ERR;
  }
  
  /* Read Status register again */
  if(ETH_ReadPHYRegister(PHYAddress, PHY_BSR, &readval) != OK)
  {
    return ERR;
  }
  
  if(((readval & PHY_LINKED_STATUS) != PHY_LINKED_STATUS))
  {
    /* Return Link Down status */
    return ERR;    
  }

	return OK; //PHY_LINKED_STATUS should be TRUE (cable connected)
}























/**
  * @brief  Enables or disables the DMA transmission.
  * @param  NewState: new state of the DMA transmission.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ETH_DMATransmissionCmd(FunctionalState NewState)
{ 
  if (NewState != DISABLE)
  {
    /* Enable the DMA transmission */
    ETH->DMAOMR |= ETH_DMAOMR_ST;  
  }
  else
  {
    /* Disable the DMA transmission */
    ETH->DMAOMR &= ~ETH_DMAOMR_ST;
  }
}


/**
  * @brief  Enables or disables the DMA reception.
  * @param  NewState: new state of the DMA reception.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ETH_DMAReceptionCmd(FunctionalState NewState)
{

  if (NewState != DISABLE)
  {
    /* Enable the DMA reception */
    ETH->DMAOMR |= ETH_DMAOMR_SR;
  }
  else
  {
    /* Disable the DMA reception */
    ETH->DMAOMR &= ~ETH_DMAOMR_SR;
  }
}


/**
  * @brief  Enables or disables the MAC reception.
  * @param  NewState: new state of the MAC reception.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ETH_MACReceptionCmd(FunctionalState NewState)
{ 
  __IO uint32_t tmpreg = 0;

  if (NewState != DISABLE)
  {
    /* Enable the MAC reception */
    ETH->MACCR |= ETH_MACCR_RE;
  }
  else
  {
    /* Disable the MAC reception */
    ETH->MACCR &= ~ETH_MACCR_RE; 
  }

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->MACCR;
  ETH_Delay(ETH_REG_WRITE_DELAY);
  ETH->MACCR = tmpreg;
}


/**
  * @brief  Enables or disables the MAC transmission.
  * @param  NewState: new state of the MAC transmission.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ETH_MACTransmissionCmd(FunctionalState NewState)
{ 
  __IO uint32_t tmpreg = 0;

  if (NewState != DISABLE)
  {
    /* Enable the MAC transmission */
    ETH->MACCR |= ETH_MACCR_TE;
  }
  else
  {
    /* Disable the MAC transmission */
    ETH->MACCR &= ~ETH_MACCR_TE;
  }

  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->MACCR;
  ETH_Delay(ETH_REG_WRITE_DELAY);
  ETH->MACCR = tmpreg;
}


/**
  * @brief  Clears the ETHERNET transmit FIFO.
  * @param  None
  * @retval None
  */
void ETH_FlushTransmitFIFO(void)
{
  __IO uint32_t tmpreg = 0;
  /* Set the Flush Transmit FIFO bit */
  ETH->DMAOMR |= ETH_DMAOMR_FTF;
  
  /* Wait until the write operation will be taken into account :
   at least four TX_CLK/RX_CLK clock cycles */
  tmpreg = ETH->DMAOMR;
  ETH_Delay(ETH_REG_WRITE_DELAY);
  ETH->DMAOMR = tmpreg;
}



/**
  * @brief  Enables ENET MAC and DMA reception/transmission 
  * @param  None
  * @retval None
  */
void ETH_Start(void)
{
  /* Enable transmit state machine of the MAC for transmission on the MII */
  ETH_MACTransmissionCmd(ENABLE);

  /* Enable receive state machine of the MAC for reception from the MII */
  ETH_MACReceptionCmd(ENABLE);

  /* Flush Transmit FIFO */
  ETH_FlushTransmitFIFO();

  /* Start DMA transmission */
  ETH_DMATransmissionCmd(ENABLE);

  /* Start DMA reception */
  ETH_DMAReceptionCmd(ENABLE);

}
