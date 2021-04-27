/*
 * 525.742 SOC FPGA Design Lab
 * Fall
 * Student:
 * Lab 5
 * Tuner Controller.c MicroBlaze code for controlling the SDR. The output of the DDS is sent to the CODEC
 * on the Zybo-z7 development board for playback over a loudspeaker.
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   115200
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "sleep.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "xil_cache.h"
#include <stdbool.h>
#include "xgpio.h"
#include "xiic_l.h"
#include "xuartps_hw.h"

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
extern volatile int dhcp_timoutcntr;
#endif
extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

#define DEFAULT_IP_ADDRESS	"192.168.1.10"
#define DEFAULT_IP_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDRESS	"192.168.1.1"

#define TIMER_REG (0)
#define IO_REG (1)
#define FIFO_DATA_REG (2)
#define FIFO_COUNT_REG (3)

void platform_enable_interrupts(void);
void start_application(void);
void transfer_data(u16 frame_count, u32* data);
void print_app_header(void);
u8 in_byte(struct netif *netif);

#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || \
		 XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
int IicPhyReset(void);
#endif
#endif

struct netif server_netif;

static void print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	print_ip("Board IP:       ", ip);
	print_ip("Netmask :       ", mask);
	print_ip("Gateway :       ", gw);
}

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if (!err)
		xil_printf("Invalid default IP address: %d\r\n", err);

	err = inet_aton(DEFAULT_IP_MASK, mask);
	if (!err)
		xil_printf("Invalid default IP MASK: %d\r\n", err);

	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if (!err)
		xil_printf("Invalid default gateway address: %d\r\n", err);
}

#define ADmute 0x01;
#define DDS_CHANNEL	1
#define TUNER_CHANNEL	1
#define GPIO_CHANNEL 2

// Function Prototypes
void write_codec_register(u8 regnum, u8 regval);
void configure_codec();
void test_dds();
int  mygets(char *string,struct netif *netif);
void setDDS_freq(int dds_phase_inc);
void setTuner_freq(int tuner_phase_inc);

int main()
{
	u8  rcvd_char;
	u8  get_char;
	float phase_inc;
	float Tuner_phase_inc;
	int DDS_frequency;
	int Tuner_frequency;
	char char_string[20];
	int Status;
	int CODEC;
	u8  volume;
	u8  volume_level;
	bool stream_enable_flag = true;
	u32 payload[256];
	u16 frame_counter=0;
	volatile u32 *periph_base = (volatile u32 *)XPAR_CUSTOMPERIPHERAL_0_S00_AXI_BASEADDR;
	int avail_in_fifo = 0;
	u32 RecievedByte;

	struct netif *netif;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] = {
		0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	netif = &server_netif;
#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || \
		XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif
#endif

	/* Define this board specific macro in order perform PHY reset
	 * on ZCU102
	 */
#ifdef XPS_BOARD_ZCU102
	IicPhyReset();
#endif

	init_platform();

	lwip_init();

/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address,
				PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\r\n");
		return -1;
	}
	netif_set_default(netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(netif);


#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	dhcp_start(netif);
	dhcp_timoutcntr = 24;
	while (((netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		xemacif_input(netif);

	if (dhcp_timoutcntr <= 0) {
		if ((netif->ip_addr.addr) == 0) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			assign_default_ip(&(netif->ip_addr),
					&(netif->netmask), &(netif->gw));
		}
	}

	/* print IP address, netmask and gateway */
#else
	assign_default_ip(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
#endif
	//print_ip_settings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));


	/* start the application*/


// Print program title

    print("***********************************\n\r");
    print("*  SDR Tuner Controller  *\n\r");
    print("***********************************\n\r");
    print("\n\r");

 // Initialize CODEC
    configure_codec();
    xil_printf("CODEC Initialization Complete\r\n");
    CODEC = 0x2; // CODEC on MUTE, DDS active

// Initialize General purpose GPIO outputs
    XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_TRI_OFFSET, 0x0);
	xil_printf("GPIO Initialized\r\n");

// Initialize DDS GPIO outputs
	XGpio_WriteReg(XPAR_DDS_CONFIG_BASEADDR,((DDS_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_TRI_OFFSET, 0x00000000);
	xil_printf("DDS GPIO Initialized\r\n");

// Initialize Tuner GPIO outputs
	XGpio_WriteReg(XPAR_TUNER_BASEADDR,((TUNER_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_TRI_OFFSET, 0x00000000);
	xil_printf("Tuner GPIO Initialized\r\n");

// Initialize DDS to 0Hz.
	DDS_frequency = 0;
	phase_inc = 1.07374*DDS_frequency;
	setDDS_freq(phase_inc);
	xil_printf("\r\n\r\nDDS frequency   = %d Hz\r\n",DDS_frequency);
	xil_printf("Phase Increment = %d\r\n",(int)phase_inc);

// Initialize Tuner to 0Hz.
	Tuner_frequency = 0;
	Tuner_phase_inc = 1.07374*Tuner_frequency;
	setTuner_freq(Tuner_phase_inc);
	xil_printf("\r\n\r\nTuner frequency   = %d Hz\r\n",Tuner_frequency);
	xil_printf("Tuner Phase Increment = %d\r\n",(int)Tuner_phase_inc);

// Initialize CODEC volume to 0dB (default) Done by configure_CODEC().
	volume       = 121;
	volume_level = 8;
	start_application();
// User loop
    while(1)
    {
    	if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(netif);
    	if(stream_enable_flag)
    	{

			// axi reading will be done(1024 byte = 256 word)

			avail_in_fifo = *(periph_base+FIFO_COUNT_REG);
			if(avail_in_fifo > 256)
			{
				xil_printf("Streaming data is ready -- %d\r\n", frame_counter);
				for(int i=0;i<256;i++)
				{
					payload[i] = *(periph_base+FIFO_DATA_REG);
				}
				transfer_data(frame_counter, payload);
				frame_counter ++;
			}

    	}
    	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
    	if(RecievedByte == ' ')
    	{
			print("\r\nPress 'f' to load new frequency\r\n");
			print("Press 'u' to increment frequency by 100Hz\r\n");
			print("Press 'U' to increment frequency by 1KHz\r\n");
			print("Press 'd' to decrement frequency by 100Hz\r\n");
			print("Press 'D' to decrement frequency by 1KHz\r\n");
			print("Press 't' for 50KHz ILA data capture\r\n");
			print("Press 'o' to turn CODEC mute on and off\r\n");
			print("Press 'z' to reset DDS phase\r\n");
			print("Press 'T' to set tuner frequency\r\n");
			print("Press '+' to increase audio volume\r\n");
			print("Press '-' to decrease audio volume\r\n");
			print("Press 'e' to enable/disable streaming");
			get_char = 0;
			while(get_char == 0)
			{


				rcvd_char = in_byte(netif);
					if(rcvd_char == 'f')
						{
							xil_printf("\r\nEnter DDS frequency in Hz:");
							Status =  mygets(char_string,netif);
								if(Status == -1){xil_printf("Error reading string\r\n");}
							DDS_frequency = atoi(char_string);
							phase_inc = 1.07374*DDS_frequency;
							setDDS_freq(phase_inc);
							xil_printf("\r\n\r\nDDS frequency   = %d Hz\r\n",DDS_frequency);
							xil_printf("Phase Increment = %d\r\n",(int)phase_inc);
							get_char = 1;
						}

					else if(rcvd_char == 'u')
						{
							DDS_frequency = DDS_frequency + 100;
							if(DDS_frequency >= 100000000)
								{
									DDS_frequency = 100000000;
								}
							phase_inc = 1.07374*DDS_frequency;
							setDDS_freq(phase_inc);
							xil_printf("\r\n\r\nDDS frequency = %d Hz\r\n",DDS_frequency);
							xil_printf("Phase Increment = %d\r\n",(int)phase_inc);
							get_char = 1;
						}
					else if(rcvd_char == 'U')
						{
							DDS_frequency = DDS_frequency + 1000;
							if(DDS_frequency >= 100000000)
								{
									DDS_frequency = 100000000;
								}
							phase_inc = 1.07374*DDS_frequency;
							setDDS_freq(phase_inc);
							xil_printf("\r\n\r\nDDS frequency = %d Hz\r\n",DDS_frequency);
							xil_printf("Phase Increment = %d\r\n",(int)phase_inc);
							get_char = 1;
						}
					else if(rcvd_char == 'd')
						{
							DDS_frequency = DDS_frequency - 100;
							if(DDS_frequency <= 0)
								{
									DDS_frequency = 0;
								}
							phase_inc = 1.07374*DDS_frequency;
							setDDS_freq(phase_inc);
							xil_printf("\r\n\r\nDDS frequency = %d Hz\r\n",DDS_frequency);
							xil_printf("Phase Increment = %d\r\n",(int)phase_inc);
							get_char = 1;
						}
					else if(rcvd_char == 'D')
						{
							DDS_frequency = DDS_frequency - 1000;
							if(DDS_frequency <= 0)
								{
									DDS_frequency = 0;
								}
							phase_inc = 1.07374*DDS_frequency;
							setDDS_freq(phase_inc);
							xil_printf("\r\n\r\nDDS frequency = %d Hz\r\n",DDS_frequency);
							xil_printf("Phase Increment = %d\r\n",(int)phase_inc);
							get_char = 1;
						}
					else if(rcvd_char == 't')
						{
							xil_printf("\r\n Set ILA trigger signal = 'write_to_file', rising edge. Arm Trigger.\r\n");
							xil_printf("\r\n Hit any key to run ILA data capture.\r\n");
							rcvd_char = in_byte(netif);
							test_dds();

							XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, CODEC);
							phase_inc = 1.07374*DDS_frequency;
							setDDS_freq(phase_inc);
							xil_printf("\r\n\r\nDDS frequency restored to = %d Hz\r\n",DDS_frequency);
							xil_printf("Phase Increment = %d\r\n",(int)phase_inc);
							get_char = 1;
						}
					else if(rcvd_char == 'o')
						{
							if (CODEC == 0x3)
								{
									xil_printf("\r\n Codec Mute on\r\n");
									XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x2);
									CODEC = 0x2;
									break;

								}
							if (CODEC == 0x2)
								{
									xil_printf("\r\n Codec Mute off\r\n");
									XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x3);
									CODEC = 0x3;
									break;
								}
							get_char = 1;
						}
					else if(rcvd_char == 'z')
						{
							XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x0);
							XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, CODEC);
							xil_printf("\r\n DDS Phase Reset.\r\n");
							get_char = 1;
						}

					else if(rcvd_char == 'T')
						{
							xil_printf("\r\nEnter Tuner frequency in Hz:");
							Status =  mygets(char_string,netif);
								if(Status == -1){xil_printf("Error reading string\r\n");}
							Tuner_frequency = atoi(char_string);
							Tuner_phase_inc = 1.07374*Tuner_frequency;
							setTuner_freq(Tuner_phase_inc);
							xil_printf("\r\n\r\nTuner frequency   = %d Hz\r\n",Tuner_frequency);
							xil_printf("Tuner Phase Increment = %d\r\n",(int)Tuner_phase_inc);
							get_char = 1;
						}

					else if(rcvd_char == '+')
						{
							if(volume >= 127)
								{
									volume       = 127;
									volume_level = 9;
								}
							else
								{
									volume       = volume + 6;
									volume_level = volume_level + 1;
								}

							write_codec_register(2,volume);
							write_codec_register(3,volume);
							xil_printf("\r\n\r\nCODEC Volume level = %d\r\n",volume_level);
							get_char = 1;
						}

					else if(rcvd_char == '-')
						{
							if(volume <= 73)
								{
									volume       = 73;
									volume_level = 0;
								}
							else
								{
									volume = volume - 6;
									volume_level = volume_level - 1;
								}

							write_codec_register(2,volume);
							write_codec_register(3,volume);
							xil_printf("\r\n\r\nCODEC Volume level = %d\r\n",volume_level);
							get_char = 1;
						}
					else if (rcvd_char == 'e')
					{
						if(stream_enable_flag)
						{
							stream_enable_flag = false;
							print("\r\n Streaming disabled\n\r");
						}
						else
						{
							stream_enable_flag = true;
						}
						get_char = 1;
					}
					else
						{
							print("\r\n Incorrect entry. Try again\n\r");
						}
				}
			}
    }

    cleanup_platform();
    return 0;
}


// Functions
void write_codec_register(u8 regnum, u8 regval)
{
	u8 IIC_buffer[2];

	IIC_buffer[0] = regnum <<1;
	IIC_buffer[1] = regval;

	XIic_Send(XPAR_IIC_BASEADDR, 26, IIC_buffer, 2, XIIC_STOP);
}

void configure_codec()
{
     write_codec_register(0xf,0x00);
     usleep(1000);
     write_codec_register(6,0x30);
     write_codec_register(0,0x17);
     write_codec_register(1,0x17);
     write_codec_register(2,0x79);
     write_codec_register(3,0x79);
     write_codec_register(4,0x10);
     write_codec_register(5,0x00);
     write_codec_register(7,0x02);
     write_codec_register(8,0x00);
     usleep(75000);
     write_codec_register(9,0x01);
     write_codec_register(6,0x00);
}

void test_dds()
{
	XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x2);              // Take out of reset
	XGpio_WriteReg(XPAR_DDS_CONFIG_BASEADDR,((DDS_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x0000d1b7);
	XGpio_WriteReg(XPAR_DDS_CONFIG_BASEADDR,((DDS_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x8000d1b7);  // config for 50KHz
	XGpio_WriteReg(XPAR_DDS_CONFIG_BASEADDR,((DDS_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x0000d1b7);
	XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x0);              // reset DDS
	XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x7);              // WTF = '1, RST = '1'
	usleep(1000);
	XGpio_WriteReg(XPAR_GPIO_BASEADDR,((GPIO_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, 0x2);              // WTF = '1, RST = '1'
	xil_printf("\r\n programmed DDS for 50 KHz\r\n");
}

// This function was found at the following link and was modified to suit the purpose of this lab.
//https://forums.xilinx.com/t5/Processor-System-Design-and-AXI/Getchar-problem-with-Microblaze-getting-NULs-after-each/m-p/187236#M5323
int mygets(char *string, struct netif *netif)
{
	int x=0;
	int ret = -1;

	while(x != '\r' && x != '\n'){
	   ret++;
	   x = in_byte(netif);
	   outbyte(x);
	   *string++ = x;
	}
	*string = '\0'; // optional to add end of string symbol

	return (ret);
}

void setDDS_freq(int dds_phase_inc)
{
	XGpio_WriteReg(XPAR_DDS_CONFIG_BASEADDR,((DDS_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, dds_phase_inc);
    XGpio_WriteReg(XPAR_DDS_CONFIG_BASEADDR,((DDS_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, (0x80000000 | dds_phase_inc));
    XGpio_WriteReg(XPAR_DDS_CONFIG_BASEADDR,((DDS_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, dds_phase_inc);
}

void setTuner_freq(int tuner_phase_inc)
{
	XGpio_WriteReg(XPAR_TUNER_BASEADDR,((TUNER_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, tuner_phase_inc);
    XGpio_WriteReg(XPAR_TUNER_BASEADDR,((TUNER_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, (0x80000000 | tuner_phase_inc));
    XGpio_WriteReg(XPAR_TUNER_BASEADDR,((TUNER_CHANNEL - 1) * XGPIO_CHAN_OFFSET) + XGPIO_DATA_OFFSET, tuner_phase_inc);
}

u8 in_byte(struct netif *netif)
{
	u32 RecievedByte;
	while (!XUartPs_IsReceiveData(STDIN_BASEADDRESS)) {
		if(TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(netif);
	}

	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
	return (u8) RecievedByte;
}
