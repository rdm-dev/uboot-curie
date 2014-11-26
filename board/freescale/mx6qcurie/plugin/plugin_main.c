#include "sdk.h"
#include "timer.h"
#include "registers/regsuart.h"
#include "registers/regsusdhc.h"
#include "usdhc_ifc.h"
#include "usdhc_mmc.h"
#include "xprintf.h"
#include "plugin_data.h"

#define UART_INST    HW_UART1
#define USDHC_INST   HW_USDHC4

void xputc(char c)
{
	uart_putchar(UART_INST, c);
}

char data[PLUGIN_DATA_SIZE_MAX];

static plugin_data_value_ddr_param_t default_ddr_param = {
	.m_phy0 = {
		.m_mpdgctrl0 = 0x032C033C,
		.m_mpdgctrl1 = 0x03200314,
		.m_mprddlctl = 0x40303436,
		.m_mpwrdlctl = 0x30303A34,
	},
	.m_phy1 = {
		.m_mpdgctrl0 = 0x03400354,
		.m_mpdgctrl1 = 0x03240268,
		.m_mprddlctl = 0x36342C44,
		.m_mpwrdlctl = 0x3E2E3E36,
	},
};

void dump_mem(char *p, int len)
{
	int i;
	xprintf("\n");
	for(i = 0; i < len; i++)
	{
		if(i % 16 == 0)
			xprintf("%04x", i & ~0xf);
		if(i % 8 == 0)
			xprintf(" ");
		xprintf(" %02x", p[i]);
		if(i % 16 == 15)
			xprintf("\n");
	};
	xprintf("\n");
}

void fill_ddr_param(const plugin_data_value_ddr_param_t *p);
void PluginMain(void)
{
	int ret;
	plugin_data_itr_t itr;
	int ddr_calibrated = 0;

	// clock init
	ccm_init();

	// timer
	system_time_init();

	// uart init
	uart_init(UART_INST, 115200, PARITY_NONE, STOPBITS_ONE, EIGHTBITS, FLOWCTRL_OFF);

	// banner
	xprintf("\n\ni.MX6 Plugin for Pre-Configuration\n\n");

	// emmc init
	set_card_access_mode(0, 0);
	if(FAIL == card_emmc_init(USDHC_INST))
	{
		xprintf("Fail to init eMMC.\n");
		goto CLEANUP;
	};
	xprintf("eMMC initialized.\n");
	// dump info
	emmc_print_cfg_info(USDHC_INST);

	// access boot #2
	if(FAIL == mmc_set_partition_access(USDHC_INST, PA_BT2))
	{
		xprintf("Fail to set access to boot partition #2.\n");
		goto CLEANUP;
	};

	// try to read plugin data
	if(FAIL == card_data_read(USDHC_INST, (int *) data, sizeof(data), 0 /*offset*/) )
	{
		xprintf("Fail to read eMMC data.\n");
		goto CLEANUP;
	};

	// access user
	if(FAIL == mmc_set_partition_access(USDHC_INST, PA_USER))
	{
		xprintf("Fail to set access to user partition.\n");
		goto CLEANUP;
	};

	dump_mem(data, 512);
	// decode plugin data
	if( (ret = plugin_data_itr_init(&itr, data)) )
	{
		xprintf("No plugin data found: code=%d.\n", ret);
		goto CLEANUP;
	};

	// process each segment in plugin data
	while(itr.m_seg)
	{
		// check segment tags
		switch(itr.m_seg->m_tag)
		{
			case PLUGIN_DATA_TAG_DDR_PARAM:
				if(itr.m_seg->m_length != sizeof(plugin_data_value_ddr_param_t))
				{
					xprintf("Bad DDR Calibration Parameters length: %d.", itr.m_seg->m_length);
					break;
				};
				xprintf("DDR Calibration Parameters...\n");
				fill_ddr_param((plugin_data_value_ddr_param_t *) itr.m_seg->m_value);
				ddr_calibrated = 1;
				break;
			default:
				xprintf("Unknown Tags: %d. Ignored\n", itr.m_seg->m_tag);
				break;
		};

		// move to next tag
		if( plugin_data_itr_next(&itr) )
			break;
	};

CLEANUP:
	if(!ddr_calibrated)
	{
		xprintf("Calibrating DDR with default parameters...");
		fill_ddr_param(&default_ddr_param);
	};
	return;
}

void fill_ddr_param(const plugin_data_value_ddr_param_t *p)
{
	//DDR IO TYPE:			
	reg32_write(0x020e0798, 0x000C0000);	// IOMUXC_SW_PAD_CTL_GRP_DDR_TYPE 
	reg32_write(0x020e0758, 0x00000000);	// IOMUXC_SW_PAD_CTL_GRP_DDRPKE 
				
	//CLOCK:			
	reg32_write(0x020e0588, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_0
	reg32_write(0x020e0594, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_1
				
	//ADDRESS:			
	reg32_write(0x020e056c, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_CAS
	reg32_write(0x020e0578, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_RAS
	reg32_write(0x020e074c, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_ADDDS 
				
	//Control:			
	reg32_write(0x020e057c, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_RESET
	reg32_write(0x020e058c, 0x00000000);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDBA2 - DSE can be configured using Group Control Register: IOMUXC_SW_PAD_CTL_GRP_CTLDS
	reg32_write(0x020e059c, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT0
	reg32_write(0x020e05a0, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT1
	reg32_write(0x020e078c, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_CTLDS 
				
	//Data Strobes:			
	reg32_write(0x020e0750, 0x00020000);	// IOMUXC_SW_PAD_CTL_GRP_DDRMODE_CTL 
	reg32_write(0x020e05a8, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS0 
	reg32_write(0x020e05b0, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS1 
	reg32_write(0x020e0524, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS2 
	reg32_write(0x020e051c, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS3 
	reg32_write(0x020e0518, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS4 
	reg32_write(0x020e050c, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS5 
	reg32_write(0x020e05b8, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS6 
	reg32_write(0x020e05c0, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS7 
				
	//Data:			
	reg32_write(0x020e0774, 0x00020000);	// IOMUXC_SW_PAD_CTL_GRP_DDRMODE
	reg32_write(0x020e0784, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B0DS 
	reg32_write(0x020e0788, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B1DS 
	reg32_write(0x020e0794, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B2DS 
	reg32_write(0x020e079c, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B3DS 
	reg32_write(0x020e07a0, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B4DS 
	reg32_write(0x020e07a4, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B5DS 
	reg32_write(0x020e07a8, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B6DS 
	reg32_write(0x020e0748, 0x00000028);	// IOMUXC_SW_PAD_CTL_GRP_B7DS 

	reg32_write(0x020e05ac, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM0
	reg32_write(0x020e05b4, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM1
	reg32_write(0x020e0528, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM2
	reg32_write(0x020e0520, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM3
	reg32_write(0x020e0514, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM4
	reg32_write(0x020e0510, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM5
	reg32_write(0x020e05bc, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM6
	reg32_write(0x020e05c4, 0x00000028);	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM7
				
				
	//=============================================================================			
	// DDR Controller Registers			
	//=============================================================================			
	// Manufacturer:	Hynix		
	// Device Part Number:	H5TQ2G63BFR		
	// Clock Freq.:		528MHz		
	// Density per CS in Gb:	8		
	// Chip Selects used:	1		
	// Number of Banks:	8		
	// Row address:		14		
	// Column address:	10		
	// Data bus width	64		
	//=============================================================================			
				
	//=============================================================================			
	// Calibration setup.			
	//=============================================================================			
	reg32_write(0x021b0800, 0xA1390003);	// DDR_PHY_P0_MPZQHWCTRL, enable both one-time & periodic HW ZQ calibration.
				
	// For target board, may need to run write leveling calibration to fine tune these settings.			
	reg32_write(0x021b080c, 0x001F001F);
	reg32_write(0x021b0810, 0x001F001F);
	reg32_write(0x021b480c, 0x001F001F);
	reg32_write(0x021b4810, 0x001F001F);
				
	////Read DQS Gating calibration			
	reg32_write(0x021b083c, p->m_phy0.m_mpdgctrl0);	// MPDGCTRL0 PHY0
	reg32_write(0x021b0840, p->m_phy0.m_mpdgctrl1);	// MPDGCTRL1 PHY0
	reg32_write(0x021b483c, p->m_phy1.m_mpdgctrl0);	// MPDGCTRL0 PHY1
	reg32_write(0x021b4840, p->m_phy1.m_mpdgctrl1);	// MPDGCTRL1 PHY1
				
	//Read calibration			
	reg32_write(0x021b0848, p->m_phy0.m_mprddlctl);	// MPRDDLCTL PHY0
	reg32_write(0x021b4848, p->m_phy1.m_mprddlctl);	// MPRDDLCTL PHY1
				
	//Write calibration									
	reg32_write(0x021b0850, p->m_phy0.m_mpwrdlctl);	// MPWRDLCTL PHY0
	reg32_write(0x021b4850, p->m_phy1.m_mpwrdlctl);	// MPWRDLCTL PHY1
				
	//read data bit delay: (3 is the reccommended default value, although out of reset value is 0)			
	reg32_write(0x021b081c, 0x33333333);	// DDR_PHY_P0_MPREDQBY0DL3
	reg32_write(0x021b0820, 0x33333333);	// DDR_PHY_P0_MPREDQBY1DL3
	reg32_write(0x021b0824, 0x33333333);	// DDR_PHY_P0_MPREDQBY2DL3
	reg32_write(0x021b0828, 0x33333333);	// DDR_PHY_P0_MPREDQBY3DL3
	reg32_write(0x021b481c, 0x33333333);	// DDR_PHY_P1_MPREDQBY0DL3
	reg32_write(0x021b4820, 0x33333333);	// DDR_PHY_P1_MPREDQBY1DL3
	reg32_write(0x021b4824, 0x33333333);	// DDR_PHY_P1_MPREDQBY2DL3
	reg32_write(0x021b4828, 0x33333333);	// DDR_PHY_P1_MPREDQBY3DL3
				
	//For i.mx6qd parts of versions A & B (v1.0, v1.1), uncomment the following lines. For version C (v1.2), keep commented			
	//0x021b08c0 =	0x24911492	// fine tune SDCLK duty cyc to low - seen to improve measured duty cycle of i.mx6
	//0x021b48c0 =	0x24911492	
				
	// Complete calibration by forced measurement:								
	reg32_write(0x021b08b8, 0x00000800);	// DDR_PHY_P0_MPMUR0, frc_msr
	reg32_write(0x021b48b8, 0x00000800);	// DDR_PHY_P0_MPMUR0, frc_msr
	//=============================================================================			
	// Calibration setup end			
	//=============================================================================			
				
	//MMDC init:			
	reg32_write(0x021b0004, 0x00020036);	// MMDC0_MDPDC
	reg32_write(0x021b0008, 0x09444040);	// MMDC0_MDOTC
	reg32_write(0x021b000c, 0x54597975);	// MMDC0_MDCFG0
	reg32_write(0x021b0010, 0xFF538F64);	// MMDC0_MDCFG1
	reg32_write(0x021b0014, 0x01FF00DB);	// MMDC0_MDCFG2
				
	//MDMISC: RALAT kept to the high level of 5.			
	//MDMISC: consider reducing RALAT if your 528MHz board design allow that. Lower RALAT benefits:				
	//a. better operation at low frequency, for LPDDR2 freq < 100MHz, change RALAT to 3			
	//b. Small performence improvment			
	reg32_write(0x021b0018, 0x00011740);	// MMDC0_MDMISC
	reg32_write(0x021b001c, 0x00008000);	// MMDC0_MDSCR, set the Configuration request bit during MMDC set up
	reg32_write(0x021b002c, 0x000026D2);	// MMDC0_MDRWD
	reg32_write(0x021b0030, 0x00591023);	// MMDC0_MDOR
	reg32_write(0x021b0040, 0x00000027);	// Chan0 CS0_END 
	reg32_write(0x021b0000, 0x831A0000);	// MMDC0_MDCTL
				
	//Mode register writes							
	reg32_write(0x021b001c, 0x02088032);	// MMDC0_MDSCR, MR2 write, CS0
	reg32_write(0x021b001c, 0x00008033);	// MMDC0_MDSCR, MR3 write, CS0
	reg32_write(0x021b001c, 0x00048031);	// MMDC0_MDSCR, MR1 write, CS0
	reg32_write(0x021b001c, 0x09408030);	// MMDC0_MDSCR, MR0write, CS0
	reg32_write(0x021b001c, 0x04008040);	// MMDC0_MDSCR, ZQ calibration command sent to device on CS0
				
	//0x021b001c =	0x0208803A	// MMDC0_MDSCR, MR2 write, CS1
	//0x021b001c =	0x0000803B	// MMDC0_MDSCR, MR3 write, CS1
	//0x021b001c =	0x00048039	// MMDC0_MDSCR, MR1 write, CS1
	//0x021b001c =	0x09408038	// MMDC0_MDSCR, MR0write, CS1
	//0x021b001c =	0x04008048	// MMDC0_MDSCR, ZQ calibration command sent to device on CS1
				
	reg32_write(0x021b0020, 0x00007800);	// MMDC0_MDREF
				
	reg32_write(0x021b0818, 0x00022227);	// DDR_PHY_P0_MPODTCTRL
	reg32_write(0x021b4818, 0x00022227);	// DDR_PHY_P1_MPODTCTRL
				
	reg32_write(0x021b0004, 0x00025576);	// MMDC0_MDPDC now SDCTL power down enabled
				
	reg32_write(0x021b0404, 0x00011006);	// MMDC0_MAPSR ADOPT power down enabled, MMDC will enter automatically to self-refresh while the number of idle cycle reached.
				
	reg32_write(0x021b001c, 0x00000000);	// MMDC0_MDSCR, clear this register (especially the configuration bit as initialization is complete)
}

