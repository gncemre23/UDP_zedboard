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
#include <stdlib.h>
#include <stdbool.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xiic_l.h"
#include "xil_io.h"
#include "sleep.h"
#include "xgpio.h"
#include "string.h"

#define ADmute 0x01;
#define DDS_CHANNEL	1
#define TUNER_CHANNEL	1
#define GPIO_CHANNEL 2

// Function Prototypes
void write_codec_register(u8 regnum, u8 regval);
void configure_codec();
void test_dds();
int  mygets(char *string);
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
	bool dma_enable_flag = false;

    init_platform();
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

// User loop
    while(1)
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
    	print("Press 'e' to enable/disable DMA transfer");
    	get_char = 0;
    	while(get_char == 0)
    	{
    		rcvd_char = inbyte();
    			if(rcvd_char == 'f')
					{
    					xil_printf("\r\nEnter DDS frequency in Hz:");
    				    Status =  mygets(char_string);
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
						rcvd_char = inbyte();
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
						Status =  mygets(char_string);
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
    				if(dma_enable_flag)
    				{
    					dma_enable_flag = false;
    					print("\r\n DMA disabled\n\r");
    				}
    				else
    				{
    					dma_enable_flag = true;
    					print("\r\n DMA enabled\n\r");
    				}
    				get_char = 1;
				}
    			else
    				{
    					print("\r\n Incorrect entry. Try again\n\r");
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
int mygets(char *string)
{
	int x=0;
	int ret = -1;

	while(x != '\r' && x != '\n'){
	   ret++;
	   x = inbyte();
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

