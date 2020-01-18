#include <stdio.h>
#include "NuMicro.h"
#include "SYS_init.h"

void SYS_Init(void)
{
		/* Unlock protected registers */
    SYS_UnlockReg();
    /* Enable HIRC */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
    /* Switch HCLK clock source to HIRC */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
	//////////////////////////////////////////////////////////////////////
    /* Set core clock as PLL_CLOCK from PLL */
    //CLK_SetCoreClock(96000000);
    /* Set PCLK0/PCLK1 to HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1;
	/////////////////////////////////////////////////////////////////////////
    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2);
    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
    /* Enable UART peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);
		CLK_EnableModuleClock(PWM0_MODULE);
		CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, 0);
		SYS_ResetModule(PWM0_RST);
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
		SystemCoreClockUpdate();

    /*----------------------------------------------------------------------*/
    /* Init I/O Multi-function                                              */
    /*----------------------------------------------------------------------*/
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////device init//////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
		/* Set GPB multi-function pins for UART0 RXD and TXD */
    /*SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);*/
		
		device_GPIO_Init();
		
    /* Lock protected registers */
    SYS_LockReg();
		/* Reset UART0 */
    //SYS_ResetModule(UART0_RST); 
    /* Configure UART0 and set UART0 baud rate */
    //UART_Open(UART0, 115200);
}
