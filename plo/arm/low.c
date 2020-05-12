/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Low-level routines
 *
 * @copyright 2015 Phoenix Systems
 * @author: Katarzyna Baranowska <katarzyna.baranowska@phoesys.com>
 *
 * This file is part of Phoenix-RTOS.
 *
 * Phoenix-RTOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Phoenix-RTOS kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phoenix-RTOS kernel; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "MVF50GS10MK50.h"
#include "low.h"
#include "plostd.h"
#include "config.h"

#define CPU_XTAL_CLK_HZ                 24000000u /* Value of the external crystal or oscillator clock frequency in Hz */
#define CPU_XTAL32k_CLK_HZ              32768u   /* Value of the external 32k crystal or oscillator clock frequency in Hz */
#define CPU_INT_SLOW_CLK_HZ             32768u   /* Value of the slow internal oscillator clock frequency in Hz  */
#define CPU_INT_FAST_CLK_HZ             24000000u /* Value of the fast internal oscillator clock frequency in Hz  */

#if (CLOCK_SETUP == 0) //396 MHz A5 Core - Sample Code Default set in tower.h
    #define DEFAULT_SYSTEM_CLOCK            264000000u /* Default System clock value */
#elif (CLOCK_SETUP == 1) //500 MHz A5 Core - Max speed
    #define DEFAULT_SYSTEM_CLOCK            264000000u /* Default System clock value */
#elif (CLOCK_SETUP == 2) //264 MHz A5 Core - BootROM default 
    #define DEFAULT_SYSTEM_CLOCK            264000000u /* Default System clock value */
#elif (CLOCK_SETUP == 3) //396 MHz A5 Core setup that assumes BootROM did not run
    #define DEFAULT_SYSTEM_CLOCK            24000000u /* Default System clock value */
#endif 

#define CR		0x0D
#define CSI		"\x1B["
#define ESC		"\x1B"


plo_syspage_t	plo_syspage;
u16		_plo_timeout = 3;
char	_plo_command[CMD_SIZE] = DEFAULT_CMD;

extern void plostd_printf(char attr, char *, ...);

void irq_disable()
{
	asm("cpsid iaf");
}

void irq_enable()
{
	asm("cpsie iaf");
}

void low_getregs(int*x, int*y)
{
	//TODO
// 		stmia	r0, {r0-r15}
// 		mrs		r2, cpsr
// 		str		r2, [r1]
// 		bx		lr
// 		nop
}

void low_memcpy(void *dst_, const void *src_, unsigned int l)
{
	char* dst = dst_;
	const char* src = src_;
	int i;
 	for(i = 0; i<l ;i++){
	 	*(dst+i) = *(src+i);
	}
}

void low_memset(void *dst_, char c, unsigned int l)
{
	char* dst = dst_;
	int i;
 	for(i = 0; i<l ;i++){
	 	*(dst+i) = c;
	}
}

/* Serial initialisation */
/** Init clocks, pins, multiplexing needed for UARTs **/
/* In TWR-VF6 only UARTs 1 and 2 are routed to the board. They can be routed further via jumpers.
 * One of them can go to TWR-SER2 via the elevator board, where it gets taken to the D-SUB 9 connector.
 * The other goes to the OpenSDA uC and is converted to USB, so it can not be used while that USB port is used
 * for debugging via DS-5 (...or there was a firmware version which allowed that simultaneous use?).
 * However, there are 2 non-populated 0R resistors (R81 and R82) in TWR-VF6 which would make possible
 * to route the OpenSDA-routed UART to the elevator board and to the serial-USB converter in TWR-SER2.
 */
void low_consoleInit()
{
	/* Clocks init */

    /*we set every pin to (Input or Output) + ALT mux mode + speed 100MHz, drive strength 75R, pull-up 100KR*/
	CCM->CCGR0 |= CCM_CCGR0_CG7(CG_CLK_ON_RUN_OFF_WAIT_STOP_MODES);     /*Ungate UART0 clock*/
	IOMUXC->SINGLE.PTB10 = IOMUXC_PTB10_MUX_MODE(1) 	| /* tx, alt1*/
                                IOMUXC_PTB10_OBE_MASK	|
                                IOMUXC_PTB10_SPEED(1)	|
                                IOMUXC_PTB10_DSE(2)		|
                                IOMUXC_PTB10_PUS(2);

	IOMUXC->SINGLE.PTB11 = IOMUXC_PTB11_MUX_MODE(1) 	| /* rx, alt1*/
                                IOMUXC_PTB11_IBE_MASK 	|
                                IOMUXC_PTB11_SPEED(1) 	|
                                IOMUXC_PTB11_DSE(2) 	|
                                IOMUXC_PTB11_PUS(2);

	CCM->CCGR0 |= CCM_CCGR0_CG8(CG_CLK_ON_RUN_OFF_WAIT_STOP_MODES);     /*Ungate UART1 clock*/
	IOMUXC->SINGLE.PTB23 = IOMUXC_PTB23_MUX_MODE(2)		| /* tx, alt2*/
                                IOMUXC_PTB23_OBE_MASK	|
                                IOMUXC_PTB23_SPEED(1) 	|
                                IOMUXC_PTB23_DSE(2) 	|
                                IOMUXC_PTB23_PUS(2);

	IOMUXC->SINGLE.PTB24 = IOMUXC_PTB24_MUX_MODE(2)		| /* rx, alt2*/
                                IOMUXC_PTB24_IBE_MASK	|
                                IOMUXC_PTB24_SPEED(1)	|
                                IOMUXC_PTB24_DSE(2) 	|
                                IOMUXC_PTB24_PUS(2);

	IOMUXC->SCI_FLX1_IPP_IND_SCI_TX_SELECT_INPUT = 2; /*select PTB23*/
	IOMUXC->SCI_FLX1_IPP_IND_SCI_RX_SELECT_INPUT = 2; /*select PTB24*/

// 	CCM->CCGR0 |= CCM_CCGR0_CG7(CG_CLK_ON_RUN_OFF_WAIT_STOP_MODES);     //Ungate UART0 clock
// 	IOMUXC->SINGLE.PTB10 = 0x001011A2;  //TX
// 	IOMUXC->SINGLE.PTB11 = 0x001011A1;  //RX
// 
// 	CCM->CCGR0 |= CCM_CCGR0_CG8(CG_CLK_ON_RUN_OFF_WAIT_STOP_MODES);     //Ungate UART1 clock
// 	IOMUXC->SINGLE.PTB4 = 0x002011A2;   //TX, mux mode ALT2, speed 100MHz, drive strength 75R, pull-up 100KR, output
// 	IOMUXC->SINGLE.PTB5 = 0x002011A1;   //RX, mux mode ALT2, speed 100MHz, drive strength 75R, pull-up 100KR, input
// 	/*RTS and RCS not configured*/
// 	IOMUXC->SCI_FLX1_IPP_IND_SCI_RX_SELECT_INPUT=0;  //Select PTB5 as RX input
// 
// 	CCM->CCGR0 |= CCM_CCGR0_CG9(CG_CLK_ON_RUN_OFF_WAIT_STOP_MODES);   //Ungate UART2 clock
// 	IOMUXC->SINGLE.PTB6 = 0x007011A2; //TX
// 	IOMUXC->SINGLE.PTB7 = 0x007011A1; //RX
	/*RTS and RCS not configured*/
// 	IOMUXC->SCI_FLX2_IPP_IND_SCI_RX_SELECT_INPUT=0;  //Select PTB7 as RX input

	/* Console init */
	u16 sbr, brfa;
    u32 sbr64;

	if (CONSOLE_UART_PORT == NULL)
		return;

	/* Enable the pins for the selected UART */
	CONSOLE_UART_PORT->MODEM=0; //Need to clear MODEM register in case BootROM sets it  

	/* Make sure that the transmitter and receiver are disabled while we 
	 * change settings->
	 */
	CONSOLE_UART_PORT->C2 &= ~UART_C2_RE_MASK;
	CONSOLE_UART_PORT->C2 &= ~UART_C2_TE_MASK;

	/* Configure the UART for 8-bit mode, no parity */
	CONSOLE_UART_PORT->C1 = 0x00;  /* We need all default settings, so entire register is cleared */

	/* enable the hw tx and rxFIFO */
	CONSOLE_UART_PORT->PFIFO |= UART_PFIFO_TXFE_MASK | UART_PFIFO_RXFE_MASK;
	CONSOLE_UART_PORT->CFIFO |= UART_CFIFO_TXFLUSH_MASK | UART_CFIFO_RXFLUSH_MASK;

	/* Calculate baud settings
     * From docs: "UART baud rate = UART module clock / (16 × (SBR[12:0] + BRFD))"
     * We need 5-bit precision for BFRA, with one extra bit for proper rounding, hence 2*32
     */
	sbr64 = ((u32)BUS_CLK_KHZ * 1000) * 4 / CONSOLE_BAUD; /* == 2 * 32 * bus_clock / (baud * 16) */
    sbr64 += 1;		/* round-up */
    sbr = (u16)(sbr64 / 64);
    brfa = (u16)(sbr64 / 2) & 0x1f;

	CONSOLE_UART_PORT->BDH |= UART_BDH_SBR((sbr & 0x1F00) >> 8);
	CONSOLE_UART_PORT->BDL = UART_BDL_SBR((u8)(sbr & 0x00FF));  
	CONSOLE_UART_PORT->C4 &= ~UART_C4_BRFA_MASK;
	CONSOLE_UART_PORT->C4 |= UART_C4_BRFA(brfa);

	/* Enable transmitter */
	CONSOLE_UART_PORT->C2 |= UART_C2_TE_MASK;
	/* Enable receiver */
	CONSOLE_UART_PORT->C2 |= UART_C2_RE_MASK;

#ifndef CONFIG_CONSOLE_RAW
	plostd_puts(ATTR_LOADER, ESC "c" CSI "7h" CSI "2J" CSI "3;1000r" CSI "1;1H");
#endif
}

void low_watchdogInit(void)
{
	WDOG_Type *wdog = (WDOG_Type *) WDOG_BASE;
	u16 wdog_timeout = (WATCHDOG_TIMEOUT_SEC * 2) - 1;
	if (WATCHDOG_TIMEOUT_SEC > 128)
		wdog_timeout = 0xff;

	/* Disable power-down counter */
	wdog->WMCR = 0;

	wdog->WCR = WDOG_WCR_WT(wdog_timeout)	| 	/* Watchdog timeout */
				0 << WDOG_WCR_WDW_SHIFT 	|	/* Suspend wdog on low power WAIT */
				0 << WDOG_WCR_SRE_SHIFT		|	/* Use original way of generating software reset */
				1 << WDOG_WCR_WDA_SHIFT		|	/* Disable WDOG_B assertion */
				1 << WDOG_WCR_SRS_SHIFT		|	/* Disable software reset signal */
				1 << WDOG_WCR_WDT_SHIFT		|	/* Assert WDOG_B together with wdog timeout */
				1 << WDOG_WCR_WDE_SHIFT		|	/* Enable watchdog */
				1 << WDOG_WCR_WDBG_SHIFT	|	/* Suspend wdog when in DEBUG mode */
				0 << WDOG_WCR_WDZST_SHIFT;		/* Keep watchdog working in low power modes */

	wdog->WSR = 0x5555;
	wdog->WSR = 0xAAAA;
}

void low_putc(const char attr, const char ch)
{
	/* Preceed LF with CR for pretty serial console output */
	if (ch == '\n')
		low_putc(attr, '\r');

	/* Wait until space is available in the FIFO */
	while(!(CONSOLE_UART_PORT->S1 & UART_S1_TDRE_MASK));

	/* Send the character */
	CONSOLE_UART_PORT->D = (u8)ch;
}


void low_getc(char *c, char *sc)
{
	while (CONSOLE_UART_PORT->SFIFO & UART_SFIFO_RXEMPT_MASK);
	*c = CONSOLE_UART_PORT->D;
	if(*c == 0x1b){
		while (CONSOLE_UART_PORT->SFIFO & UART_SFIFO_RXEMPT_MASK);
		*c = CONSOLE_UART_PORT->D;
		if(*c != '[' && *c != 'O')
			return;
		*c = 0;
		while (CONSOLE_UART_PORT->SFIFO & UART_SFIFO_RXEMPT_MASK);
		*sc = CONSOLE_UART_PORT->D;
	}
}


int low_keypressed()
{
	return !(CONSOLE_UART_PORT->SFIFO & UART_SFIFO_RXEMPT_MASK);
}

#ifndef STATUS_LEDS
#define STATUS_LEDS { }
#endif
#ifndef STATUS_LEVEL
#define STATUS_LEVEL { }
#endif

static const int led_num[] = STATUS_LEDS;
static const int led_lev[] = STATUS_LEVEL;

void low_init()
{

	GPIO_Type *GPIOs[] = GPIO_BASES;

	int i;
	if(sizeof(led_num) > 0) {
		for(i = 0; i < sizeof(led_num)/sizeof(led_num[0]); ++i)
		{

			IOMUXC->RGPIO[led_num[i]] = IOMUXC_RGPIO_MUX_MODE(0)|
										IOMUXC_RGPIO_OBE_MASK	|
										IOMUXC_RGPIO_SPEED(1)	|
										IOMUXC_RGPIO_DSE(7) 	|
										IOMUXC_RGPIO_PUS(3)		|
										IOMUXC_RGPIO_PKE_MASK	|
										IOMUXC_RGPIO_PUE_MASK;
			if(led_lev[i])
				GPIOs[led_num[i] / 32]->PSOR |= (1 << (led_num[i] % 32));
			else
				GPIOs[led_num[i] / 32]->PCOR |= (1 << (led_num[i] % 32));
		}
	}


	low_consoleInit();
	low_dbg(0x00);

#if WATCHDOG_ENABLED
	low_watchdogInit();
#endif
}


void low_irqinst(u8 src, u16 irq, int (*isr)(u16, void*), void* data)
{
	//TODO
// 	if(src > 31 || irq > 15) return;
}


void low_irqen(u16 irq, u8 enable)
{
	//TODO
}


void low_irquninst(u16 irq)
{
	//TODO
}


void low_done()
{
	low_cli();
	low_sti();
}


void low_sti(){
	irq_enable();
}


void low_cli(){
	irq_disable();
}


void low_dbg(u8 state)
{
}


#define PHY_DQ_TIMING   0x00002613
#define PHY_DQS_TIMING  0x00002615
#define PHY_CTRL        0x00210000
#define PHY_MASTER_CTRL 0x0001012a
#define PHY_SLAVE_CTRL  0x00002400

void low_ddrInit(void)
{
	/* These settings were taken from Freescale MQX 4.2.0 BSP for TWR-VF65GS10. */

	/* Enable clock gate to DDRCM. */
	CCM->CCGR6 |= CCM_CCGR6_CG14(0x3);

	if (DDRMC->CR00 & DDRMC_CR00_START_MASK) {
		/* DDRC was already initialized. */
		return;
	}

	/* Configure DDR in IOMUXC. */
	volatile uint32_t *ptr;
	for (ptr = &(IOMUXC->DDR_RESETB); &(IOMUXC->DUMMY_DDRBYTE2) >= ptr; ++ptr)
		*ptr = IOMUXC_DDR_RESETB_DDR_INPUT_MASK | IOMUXC_DDR_RESETB_DSE(5) | IOMUXC_DDR_RESETB_DDR_TRIM(0);

	IOMUXC->DDR_CLK_0 |= IOMUXC_DDR_CLK_0_DDR_INPUT_MASK;
	IOMUXC->DDR_DQS_0 |= IOMUXC_DDR_DQS_0_DDR_INPUT_MASK;
	IOMUXC->DDR_DQS_1 |= IOMUXC_DDR_DQS_1_DDR_INPUT_MASK;

	/* Dram device parameters. */
	DDRMC->CR00 = 0x00000600; /* LPDDR2 or DDR3 */
	DDRMC->CR02 = 0x00000020; /* TINIT F0 */
	/* cold boot - 1ms??? 0x61a80 */
	DDRMC->CR10 = 0x00013880; /* reset during power on */
	/* warm boot - 200ns */
	DDRMC->CR11 = 0x00030D40; /* 500us - 10ns */
	DDRMC->CR12 = 0x0000050c; /* CASLAT_LIN, WRLAT */
	DDRMC->CR13 = 0x15040400; /* trc, trrd, tccd, tbst_int_interval */
	DDRMC->CR14 = 0x1406040F; /* tfaw, trp, twtr, tras_min */
	DDRMC->CR16 = 0x04040000; /* tmrd, trtp */
	DDRMC->CR17 = 0x006DB00C; /* tras_max, tmod */
	DDRMC->CR18 = 0x00000403; /* tckesr, tcke */

	DDRMC->CR20 = 0x01000000; /* ap, writeinterp, */
	DDRMC->CR21 = 0x00060001; /* trcd_int, tras_lockout, */
	DDRMC->CR22 = 0x000C0000; /* tdal */
	DDRMC->CR23 = 0x03000200; /* bstlen, tmrr - lpddr2, tdll */
	DDRMC->CR24 = 0x00000006; /* addr_mirror, reg_dimm, trp_ab */
	DDRMC->CR25 = 0x00010000; /* tref_enable, auto_refresh, arefresh */
	DDRMC->CR26 = 0x0C30002C; /* tref, trfc */
	DDRMC->CR28 = 0x00000000;
	DDRMC->CR29 = 0x00000003; /* tpdex_f0 */

	DDRMC->CR30 = 0x0000000A; /* txpdll */
	DDRMC->CR31 = 0x003001D4; /* txsnr, txsr */
	DDRMC->CR33 = 0x00010000; /* cke_dly, en_quick_srefresh, */
							  /* srefresh_exit_no_refresh, */
							  /* pwr, srefresh_exit */
	DDRMC->CR34 = 0x00050500; /* cksrx_f0, */

	/* Frequency change. */
	DDRMC->CR38 = 0x00000000; /* freq change... */
	DDRMC->CR39 = 0x04001002; /* PHY_INI: com, sta, freq_ch_dll_off */
	DDRMC->CR41 = 0x00000001; /* 15.02 - allow dfi_init_start */
	DDRMC->CR45 = 0x00000000; /* wrmd */
	DDRMC->CR46 = 0x00000000; /* rmd */
	DDRMC->CR47 = 0x00000000; /* REF_PER_AUTO_TEMPCHK: LPDDR2 set to 2, else 0 */

	/* DRAM device Mode registers */
	DDRMC->CR48 = 0x00460420; /* mr0, ddr3 burst of 8 only */
							  /* mr1, if freq < 125, dll_dis = 1, rtt = 0 */
							  /* if freq > 125, dll_dis = 0, rtt = 3 */
	DDRMC->CR49 = 0x00000000; /* mr0_f1_0 & mr2_f0_0 */

	DDRMC->CR51 = 0x00000000; /* mr3 & mrsingle_data */

	/* ECC */
	DDRMC->CR57 = 0x00000000; /* ctrl_raw */
							  /* ctrl_raw, if DDR3, set 3, else 0 */

	/* ZQ stuff */
	DDRMC->CR66 = 0x01000200; /* zqcl, zqinit */
	DDRMC->CR67 = 0x00000040; /* zqcs */
	DDRMC->CR69 = 0x00000200; /* zq_on_sref_exit, qz_req */

	DDRMC->CR70 = 0x00000040; /* ref_per_zq */
	DDRMC->CR71 = 0x00000000; /* zqreset, ddr3 set to 0 */
	DDRMC->CR72 = 0x00000000; /* zqcs_rotate, no_zq_init */

	/* DRAM controller misc */
	DDRMC->CR73 = 0x0a010300; /* arebit, col_diff, row_diff, bank_diff */
	DDRMC->CR74 = 0x01014040; /* bank_split, addr_cmp_en, cmd/age cnt */
	DDRMC->CR75 = 0x01010101; /* rw same pg, rw same en, pri en, plen */
	DDRMC->CR76 = 0x03030100; /* #q_entries_act_dis, (#cmdqueues */
							  /* dis_rw_grp_w_bnk_conflict */
							  /* w2r_split_en, cs_same_en */
	DDRMC->CR77 = 0x01000101; /* cs_map, inhibit_dram_cmd, dis_interleave, swen //by devendra */
	DDRMC->CR78 = 0x0700000C; /* qfull, lpddr2_s4, reduc, burst_on_fly */
	DDRMC->CR79 = 0x00000000; /* ctrlupd_req_per aref en, ctrlupd_req */
							  /* ctrller busy, in_ord_accept */
	DDRMC->CR82 = 0x10000000;
	/* ODT */
	DDRMC->CR87 = 0x01000000; /* odt: wr_map_cs, rd_map_cs, port_data_err_id */
	DDRMC->CR88 = 0x00040000; /* todtl_2cmd */
							  /* todtl_2cmd = odtl_off = CWL + AL - 2ck */
	DDRMC->CR89 = 0x00000002; /* add_odt stuff */
	DDRMC->CR91 = 0x00020000;
	DDRMC->CR92 = 0x00000000; /* tdqsck_min, _max, w2w_smcsdl */
	DDRMC->CR96 = 0x00002819; /* wlmrd, wldqsen */
	DDRMC->CR97 = 0x01000000;
	DDRMC->CR98 = 0x00000000;
	DDRMC->CR99 = 0x00000000;
	DDRMC->CR102 = 0x00000000;

	DDRMC->CR105 = 0x00001800; /* rdlvl, Byte0 */
	DDRMC->CR106 = 0x00000000; /* rdlvl_gate, Byte0 */
	DDRMC->CR110 = 0x00000018; /* rdlvl_gate, Byte1; rdlvl, Byte1 */
	DDRMC->CR114 = 0x00000000;
	DDRMC->CR115 = 0x00000000;

	/* AXI ports */
	DDRMC->CR117 = 0x00000000; /* FIFO type (0-async, 1-2:1, 2-1:2, 3- sync, w_pri, r_pri */
	DDRMC->CR118 = 0x01010000; /* w_pri, rpri, en */
	DDRMC->CR119 = 0x00000000; /* fifo_type */

	DDRMC->CR120 = 0x02020000;
	DDRMC->CR121 = 0x00000202; /* round robin port ordering */
	DDRMC->CR122 = 0x01010064;
	DDRMC->CR123 = 0x00010101;
	DDRMC->CR124 = 0x00000064;

	/* TDFI */
	DDRMC->CR125 = 0x00000000; /* dll_rst_adj_dly, dll_rst_delay */
	DDRMC->CR126 = 0x00000800; /* phy_rdlat */
	DDRMC->CR127 = 0x00000000; /* dram_ck_dis */
	DDRMC->CR131 = 0x00000000; /* tdfi_ctrlupd_interval_f0 */

	DDRMC->CR132 = 0x00000506; /* wrlat, rdlat 15.02 */
	DDRMC->CR137 = 0x00020000; /* Phyctl_dl */
	DDRMC->CR138 = 0x01000100; /* wrlvl_dl parameter */
	DDRMC->CR139 = 0x04070303;

	DDRMC->CR140 = 0x00000040; /* phy_wrlv_ww */
	DDRMC->CR143 = 0x06000080; /* phy_wrlv_resp */
	DDRMC->CR144 = 0x04070303; /* phy_wrlv_max */
	DDRMC->CR145 = 0x00000040; /* rdlv_gat_mxdl, rdlv_mxdl */
	DDRMC->CR146 = 0x00000040; /* rdlvl_res, rdlv_load, rdlv_dll, rdlv_en */
	DDRMC->CR147 = 0x000F0000; /* relv_resp_mask */
	DDRMC->CR148 = 0x000F0000; /* relv_en, redlv_gate_resp_mask, rdlv_gate_resp_mask */
	DDRMC->CR151 = 0x00000101;

	DDRMC->CR154 = 0x682C4000; /* pad_zq: _early_cmp_en_timer, _mode, _hw_for, _cmp_out_smp */
	DDRMC->CR155 = 0x00000012; /* ReadOnly pad_ibe, _sel. pad_odt, axi_awcache, axi_cobuf, pad_odt: bate0, byte1 */
	DDRMC->CR158 = 0x00000006; /* twr */
	DDRMC->CR161 = 0x00010202; /* todth */

	/* Initialize DDR PHY. */

	/* phy_dq_timing_reg freq set 0 */
	DDRMC->PHY00 = PHY_DQ_TIMING;
	DDRMC->PHY16 = PHY_DQ_TIMING;

	/* phy_dqs_timing_reg freq set 0 */
	DDRMC->PHY01 = PHY_DQS_TIMING;
	DDRMC->PHY17 = PHY_DQS_TIMING;

	/* phy_gate_lpbk_ctrl_reg freq set 0 */
	DDRMC->PHY02 = PHY_CTRL;	/* read delay bit21:19 */
	DDRMC->PHY18 = PHY_CTRL;	/* phase_detect_sel bit18:16 */
	DDRMC->PHY34 = PHY_CTRL;	/* bit lpbk_ctrl bit12 */


	/* phy_dll_master_ctrl_reg freq set 0 */
	DDRMC->PHY03 = PHY_MASTER_CTRL;
	DDRMC->PHY19 = PHY_MASTER_CTRL;
	DDRMC->PHY35 = PHY_MASTER_CTRL;


	/* phy_dll_slave_ctrl_reg freq set 0 */
	DDRMC->PHY04 = PHY_SLAVE_CTRL;
	DDRMC->PHY20 = PHY_SLAVE_CTRL;
	DDRMC->PHY36 = PHY_SLAVE_CTRL;

	DDRMC->PHY49 = 0x00000000;
	DDRMC->PHY50 = 0x00001100;
	DDRMC->PHY52 = 0x00010101;

	DDRMC->CR00 = 0x00000601; /* LPDDR2 or DDR3, start */

	{
		int i = 5000;
		while (i--);
	}

	DDRMC->CR69 = 0x00000200; /* zq_on_sref_exit, qz_req */

	{
		int i = 5000;
		while (i--);
	}
}

void low_clocksInit(void)
{
  /*
   * Enable CCM, SRC, GPC and more main modules. BootROM has already done most of these
   */
  CCM->CCGR4 |= CCM_CCGR4_CG10(CG_CLK_ON_ALL_MODES); // WKPU ungate
  CCM->CCGR4 |= CCM_CCGR4_CG11(CG_CLK_ON_ALL_MODES); // CCM ungate
  CCM->CCGR4 |= CCM_CCGR4_CG12(CG_CLK_ON_ALL_MODES); // GPC ungate
  CCM->CCGR4 |= CCM_CCGR4_CG14(CG_CLK_ON_ALL_MODES); // SRC ungate
  CCM->CCGR3 |= CCM_CCGR4_CG0(CG_CLK_ON_ALL_MODES); // ANADIG ungate
  CCM->CCGR3 |= CCM_CCGR4_CG2(CG_CLK_ON_ALL_MODES); // SCSC ungate
  CCM->CCGR1 |= CCM_CCGR1_CG7(CG_CLK_ON_RUN_OFF_WAIT_STOP_MODES);		/* Enable PIT clock */
  CCM->CCGR1 |= CCM_CCGR1_CG3(CG_CLK_ON_RUN_OFF_WAIT_STOP_MODES);		/* Enable CRC clock */

	  
  /*
   * Enable IOMUX modules
   */
  CCM->CCGR2 |= CCM_CCGR2_CG8(CG_CLK_ON_ALL_MODES); // IOMUX Controller ungate
  CCM->CCGR2 |= CCM_CCGR2_CG9(CG_CLK_ON_ALL_MODES_EXCEPT_STOP); // Port A Controller ungate
  CCM->CCGR2 |= CCM_CCGR2_CG10(CG_CLK_ON_ALL_MODES); // Port B Controller ungate - Tower buttons
  CCM->CCGR2 |= CCM_CCGR2_CG11(CG_CLK_ON_ALL_MODES_EXCEPT_STOP); // Port C Controller ungate
  CCM->CCGR2 |= CCM_CCGR2_CG12(CG_CLK_ON_ALL_MODES_EXCEPT_STOP); // Port D Controller ungate
  CCM->CCGR2 |= CCM_CCGR2_CG13(CG_CLK_ON_ALL_MODES_EXCEPT_STOP); // Port E Controller ungate

  /* Enable all modules */
/*
  CCM->CCGR0 = 0xFFFFFFFF;
  CCM->CCGR1 = 0xFFFFFFFF;
  CCM->CCGR2 = 0xFFFFFFFF;
  CCM->CCGR3 = 0xFFFFFFFF;
  CCM->CCGR4 = 0xFFFFFFFF;
  CCM->CCGR5 = 0xFFFFFFFF;
  CCM->CCGR6 = 0xFFFFFFFF;
  CCM->CCGR7 = 0xFFFFFFFF;
  CCM->CCGR8 = 0x3FFFFFFF;	//Turn off GPU since causes immediate interrupt
  CCM->CCGR9 = 0xFFFFFFFF; 
  CCM->CCGR10 = 0xFFFFFFFF;
  CCM->CCGR11 = 0xFFFFFFFF;
*/
  
  //Enable external 32kHz clock
  SCSC->SOSC_CTR|=SCSC_SOSC_CTR_SOSC_EN_MASK;


//Use 396MHz CA5, 396 MHz DDR, 132MHz CM4, and 66MHz bus
#if (CLOCK_SETUP == 0)

  /*******************************************************
   * Setup the clocks to run in synchronous mode
   * using PLL1 PFD3
   ********************************************************/
  //PLL1 uses PLL1_PFD3, enable all PLL1 and PLL2, enable PLL3_PFD4 for QSPI, select Fast Clock, and sys_clock_sel use PLL1  
  CCM->CCSR = (CCM_CCSR_PLL3_PFD4_EN_MASK | CCM_CCSR_PLL1_PFD_CLK_SEL(3) | \
		  	   0x0000FF00 | CCM_CCSR_FAST_CLK_SEL_MASK | (CCM_CCSR_SYS_CLK_SEL(4)));   
  CCM->CACRR=0x00000810; //ARM_DIV=0 (div by 1), BUS_DIV=2 (div by 3), ipg_div value is 1 (div by 2)

  /* enable all the PLLs in Anadig */
  ANADIG->PLL1_CTRL=0x00002001;	//PLL1 (System PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=1 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  ANADIG->PLL2_CTRL=0x00002001;	//PLL2 (PLL 528) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=1 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  /* PLL3 --> 480 MHz PLL for USB0 - leave as default */
  ANADIG->PLL4_CTRL=0x00002031; //PLL4 (Audio PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x31 (24M*49=1176MHz, range 600M-1300M)
  ANADIG->PLL5_CTRL=0x00002001; //PLL5 (ENET PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0b01 (50MHz)
  ANADIG->PLL6_CTRL=0x00002028; //PLL6 (Video PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x28 (24M*40=960M, range 600M-1300M)

//500 MHz A5   
#elif (CLOCK_SETUP == 1)
  
  /*******************************************************
   * Setup the clocks to run in asynchronous mode
   *
   * PLL1 (24M*22 = 528 MHz setting):
   * 	PFD1 = 500 MHz	*** selected ***
   * 	PFD2 = 452 MHz
   * 	PFD3 = 396 MHz
   * 	PFD4 = 528 MHz
   *
   * PLL2 (24M*22 = 528 MHz setting):
   * 	PFD1 = 500 MHz
   * 	PFD2 = 396 MHz
   * 	PFD3 = 339 MHz
   * 	PFD4 = 413 MHz
   *
   ********************************************************/

  /*****************************************************************************
   * PLL1 PFD1 (500MHz CA5), PLL2 PFD2 (396MHz DDR), 24MHz XOSC clock source
   *****************************************************************************/
  
  //Work-around for errata e6235
  CCM->CCSR&=~CCM_CCSR_SYS_CLK_SEL_MASK; //Select 24MHz external crystal as clock source instead of PLL   
  CCM->CCSR = (CCM_CCSR_PLL3_PFD4_EN_MASK | CCM_CCSR_PLL1_PFD_CLK_SEL(0) | \
		  	   0x0000FF00 | CCM_CCSR_FAST_CLK_SEL_MASK | (CCM_CCSR_SYS_CLK_SEL(0)));  
  ANADIG->PLL1_CTRL&=~ANADIG_PLL1_CTRL_DIV_SELECT_MASK; //Change PLL multiplier (MFI) to 20 
  ANADIG->PLL1_DENOM=100; //Set MFD to 100
  ANADIG->PLL1_NUM=83;    //Set MFN to 83
  CCM->CCSR|=CCM_CCSR_SYS_CLK_SEL(0x4); //Select PLL1 as clock source again

  /******************************************************************************
   * 500MHz CA5, 167MHz CM4, and 83.5MHz bus
   * ARM_DIV=0 (div by 1), BUS_DIV=2 (div by 3), ipg_div value is 1 (div by 2)
   ******************************************************************************/
  CCM->CACRR = (CCM_CACRR_FLEX_CLK_DIV(1) | CCM_CACRR_PLL6_CLK_DIV_MASK | \
		  	  	CCM_CACRR_IPG_CLK_DIV(1) | CCM_CACRR_BUS_CLK_DIV(2) | CCM_CACRR_ARM_CLK_DIV(0));

  /****************************************************************************************
   * enable PLLs in Anadig
   ****************************************************************************************/
  ANADIG->PLL1_CTRL=0x00002000;	//PLL1 (System PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  ANADIG->PLL2_CTRL=0x00002001;	//PLL2 (PLL 528) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=1 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  /* PLL3 --> 480 MHz PLL for USB0 - leave as default */
  ANADIG->PLL4_CTRL=0x00002031; //PLL4 (Audio PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x31 (24M*49=1176MHz, range 600M-1300M)
  ANADIG->PLL5_CTRL=0x00002001; //PLL5 (ENET PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0b01 (50MHz)
  ANADIG->PLL6_CTRL=0x00002028; //PLL6 (Video PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x28 (24M*40=960M, range 600M-1300M)
  
//264 MHz A5 - No clock init after BootROM
#elif (CLOCK_SETUP == 2)
  //Do nothing

//264 MHz A5 - Debug case where BootROM did not run and thus PLLs not turned on and clock is 24MHz
//  Need to turn on the PLL to emulate what the BootROM does, and then further configure the PLLs like normal

  // b46365: ...well, no. For debugging, assume BootROM did not run and copy from case 3 below, changing freq. to 264 MHz
  // refer to https://community.freescale.com/thread/315695
  // and to the clock distribution block diagram in the RM

  /* Mask Crystal Oscillator Ready Interrupt Bit */
  CCM->CIMR |= CCM_CIMR_M_FXOSC_READY_MASK;

  /* Enable CCM Reference Clock Enable */
  CCM->CCR |= CCM_CCR_FXOSC_EN_MASK;

  /* PLL lock depends on this bit, so make sure it is enabled (is enabled by default) */
  //SCSC->SIRC_CTR |= SCSC_SIRC_CTR_SIRC_EN_MASK;

  /* Diable Crystal Power Down Bit in LPCR register */
  CCM->CLPCR &= ~CCM_CLPCR_FXOSC_PWRDWN_MASK;

  /* Wait for Oscillator timeout to occur */
  while((CCM->CSR & CCM_CSR_FXOSC_RDY_MASK)  == 0)
  {}

  // b46365:
  // In order to get 264 MHz:
  // - PLL1 gets 24MHz as input from XTAL
  // - PLL1_out = PLL1_in * (MFI+(MFN/MFD)) = 24 * 22 = 528 MHz
  //   MFN is 0 (default), MFI is ANADIG_PLL1_CTRL[DIV_SELECT] (bits 1..2), set to 01b - 22
  // - PLL1_PFD3_out = PLL1_out * (18 / PLL1_PFD3_DIV) = 528 * 18/18 = 528
  // - ARM_CLK_DIV in CCM_CACRR is set to :2, so 528 : 2 = 264 MHz CA5 core clock

  // We can't just set PLL1_PFD3 to 36 as suggested in the Community post, because it is out of range - this is why ARM_CLK_DIV is changed to :2 instead of :1.

  ANADIG->PLL1_CTRL=0x00002003;	//PLL1 (System PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=1 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  ANADIG->PLL1_PFD = (ANADIG->PLL1_PFD &~ 0x3F3F0000) | (18 << 16) | (31 << 24); // can't just put 36 here, 12..35 is allowed
  ANADIG->PLL3_CTRL=0x00003040; //PLL3 (USB PLL)


    /*******************************************************
   * Setup the clocks to run in synchronous mode
   * using PLL1 PFD3
   * 264MHz CA5
   ********************************************************/



  CCM->CCGR4 |= CCM_CCGR4_CG11(CG_CLK_ON_ALL_MODES); // CCM ungate
  CCM->CCSR = 0xF003FF64;  // b46365: PLL1_PFD_CLK_SEL (18..16) and SYS_CLK_SEL (2..0) select PLL1_PFD3 for CA5 and bus
  CCM->CACRR = CCM_CACRR_ARM_CLK_DIV(ARM_CLOCK_DIV-1)
             | CCM_CACRR_BUS_CLK_DIV(BUS_CLOCK_DIV-1)
             | CCM_CACRR_IPG_CLK_DIV(IPG_CLOCK_DIV-1);

  // b46365: I have disabled PLL4 and PLL6 as I assume you will not be using SPDIF and Video ADC (which is not even present in VF3)
  //         You can also disable PLL5 if not using ethernet.
  //         You may need to re-enable some PLLs for peripherals that need them (e.g. some QSPI modes)
  //         Refer to RM 9.10 Peripheral Clocks for details

  /* enable all the PLLs in Anadig */
  /* PLL1 --> System PLL already turned on */
  ANADIG->PLL2_CTRL=0x00002002; //PLL2 (PLL 528) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=1 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  /* PLL3 --> 480 MHz PLL for USB0 - leave as default */
  // leave disabled: ANADIG->PLL4_CTRL=0x00002031; //PLL4 (Audio PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x31 (24M*49=1176MHz, range 600M-1300M)
  ANADIG->PLL5_CTRL=0x00002001; //PLL5 (ENET PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0b01 (50MHz)
  // leave disabled: ANADIG->PLL6_CTRL=0x00002028; //PLL6 (Video PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x28 (24M*40=960M, range 600M-1300M)


#elif (CLOCK_SETUP == 3)

  /* Mask Crystal Oscillator Ready Interrupt Bit */
  CCM->CIMR |= CCM_CIMR_M_FXOSC_READY_MASK;

  /* Enable CCM Reference Clock Enable */
  CCM->CCR |= CCM_CCR_FXOSC_EN_MASK;
  
  /* PLL lock depends on this bit, so make sure it is enabled (is enabled by default) */
  //SCSC->SIRC_CTR |= SCSC_SIRC_CTR_SIRC_EN_MASK;  

  /* Diable Crystal Power Down Bit in LPCR register */
  CCM->CLPCR &= ~CCM_CLPCR_FXOSC_PWRDWN_MASK;
  
  /* Wait for Oscillator timeout to occur */
  while((CCM->CSR & CCM_CSR_FXOSC_RDY_MASK)  == 0)
  {}
  
  ANADIG->PLL1_CTRL=0x00002001;	//PLL1 (System PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=1 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  ANADIG->PLL3_CTRL=0x00003040; //PLL3 (USB PLL)

    /*******************************************************
   * Setup the clocks to run in synchronous mode
   * using PLL1 PFD3 
   * 396MHz CA5, 396 MHz DDR, 132MHz CM4, and 66MHz bus
   ********************************************************/   
  
  CCM->CCGR4 |= CCM_CCGR4_CG11(CG_CLK_ON_ALL_MODES); // CCM ungate
  CCM->CCSR=0xF003FF64;
  CCM->CACRR=0x00000810;

  /* enable all the PLLs in Anadig */
  /* PLL1 --> System PLL already turned on */
  ANADIG->PLL2_CTRL=0x00002001;	//PLL2 (PLL 528) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=1 (1->Fout=Fref*22, 0->Fout=Fref*22=>24M*22=528MHz)
  /* PLL3 --> 480 MHz PLL for USB0 - leave as default */
  ANADIG->PLL4_CTRL=0x00002031; //PLL4 (Audio PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x31 (24M*49=1176MHz, range 600M-1300M)
  ANADIG->PLL5_CTRL=0x00002001; //PLL5 (ENET PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0b01 (50MHz)
  ANADIG->PLL6_CTRL=0x00002028; //PLL6 (Video PLL) --> POWERDOWN=0, BYPASS=0, ENABLE=1, DIV_SELECT=0x28 (24M*40=960M, range 600M-1300M)
#endif 
}

u32 low_persistGet()
{
	return ((SRC_Type *)SRC_BASE)->GPR[4];
}


void low_persistSet(const u32 val)
{
	((SRC_Type *)SRC_BASE)->GPR[4] = val;
}


/* CCITT CRC-16 with 0xffff seed */
u16 low_crc16(u16 *data, unsigned count)
{
	CRC_Type *crc = (CRC_Type *)CRC_BASE;

	crc->CTRL = 0;
	crc->GPOLY_ACCESS16BIT.GPOLYL = 0x1021;
	crc->CTRL = CRC_CTRL_WAS_MASK;
	crc->ACCESS16BIT.DATAL = 0xffff;
	crc->CTRL = 0;

	while (count--)
		crc->ACCESS16BIT.DATAL = *data++;
	return crc->ACCESS16BIT.DATAL;
}


u32 low_crc32(u32 data[], unsigned count)
{
	CRC_Type *crc = (CRC_Type *)CRC_BASE;

	crc->CTRL = CRC_CTRL_TCRC_MASK;
	crc->GPOLY = 0x04c11db7;
	crc->CTRL = CRC_CTRL_TCRC_MASK | CRC_CTRL_WAS_MASK;
	crc->DATA = 0;
	crc->CTRL = CRC_CTRL_TCRC_MASK;

	while (count--)
		crc->DATA = *data++;
	return crc->DATA;
}


void low_reset(void)
{
	((SRC_Type *)SRC_BASE)->SCR |= 1 << SRC_SCR_SW_RST_SHIFT;
}
