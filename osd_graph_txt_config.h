/*
 * osd_graph_gpio_config.h
 *
 *  Created on: 2022��8��29��
 *      Author: FYC
 */

#ifndef SRC_OSD_GRAPH_TXT_CONFIG_H_
#define SRC_OSD_GRAPH_TXT_CONFIG_H_



//*************ACSII CHARACTERS LIB INFO***************//

#define ASCII_24_ARROW_RIGHT_BAR 			22 //	up arrow
#define ASCII_24_ARROW_LEFT_BAR 			23 //	up arrow
#define ASCII_24_ARROW_UP 					24 //	up arrow
#define ASCII_25_ARROW_DOWN 				25 //	down arrow
#define ASCII_26_ARROW_RIGHT				26 //   right arrow ->
#define ASCII_27_ARROW_LEFT 				27 //   left  arrow <-
#define ASCII_28_PENTA_STAR 				28 //   five point star
#define ASCII_29_LAZER_FORBID_LAUNCH 		29 //ref comm agreement 4.1.2.7
#define ASCII_32_BLANK 						32

#define ASCII_CHAR_COLS 					16
#define ASCII_CHAR_ROWS 					32
#define ASCII_CHAR_NUM 						128



#define U_CHANNEL 			2
#define V_CHANNEL 			1
#define Y_CHANNEL 			0
#define A_CHANNEL 			3

#define OSD_COLOR_BLACK 	1
#define OSD_COLOR_WHTIE 	2
#define OSD_COLOR_RED 		3
#define OSD_COLOR_YELLOW 	4
#define OSD_COLOR_BLUE 		5
#define OSD_COLOR_GREEN 	6

#define AXI_GPIO_TAR1_XYWH_ADDR 		0x41220000
#define AXI_GPIO_TAR2_XYWH_ADDR 		0x41230000
#define AXI_GPIO_TAR3_XYWH_ADDR 		0x41210000
#define AXI_GPIO_TAR4_XYWH_ADDR 		0x41240000
#define AXI_GPIO_TAR5_XYWH_ADDR 		0x41250000
#define AXI_GPIO_TAR6_XYWH_ADDR 		0x41260000
#define AXI_GPIO_TAR7_XYWH_ADDR 		0x41290000
#define AXI_GPIO_TAR8_XYWH_ADDR 		0x412A0000
#define AXI_GPIO_TAR9_XYWH_ADDR 		0x412B0000
#define AXI_GPIO_TAR10_XYWH_ADDR 		0x412C0000




#define Tar_Gate_Addr_offset 			0x08
#define Tar_Cross_Addr_offset 			0x00
#define Tar_Graph_Color_Addr_offset 	0x00




typedef struct{
	unsigned int tar_idx;
	union gate
	{
		unsigned int Send;
		struct{
			unsigned int height:		12;		//corresponding PL GPIO bit 	11:0
			unsigned int width:			12;		//corresponding PL GPIO bit 	23:12
			unsigned int Id:			6;		//corresponding PL GPIO bit 	29:24
			unsigned int IdEn:          1;		//corresponding PL GPIO bit 	30
			unsigned int Enable:		1;		//corresponding PL GPIO bit 	31
		}info;
	}gate;
	union cross
	{
		unsigned int Send;
		struct{
			unsigned int rows_ver_y:	12;		//corresponding PL GPIO bit 	11:0
			unsigned int cols_hor_x:	12;		//corresponding PL GPIO bit 	23:12
			unsigned int line_width:	7;		//corresponding PL GPIO bit 	30:24
			unsigned int Enable:		1;		//corresponding PL GPIO bit 	31
		}info;
	}cross;
}TarGraphInfo;

typedef struct{
	union ColorConfig{
		unsigned int send;
		struct{
			unsigned int Color_B_V:		8;		//corresponding PL GPIO bit 	7:0
			unsigned int Color_G_U:		8;		//corresponding PL GPIO bit 	15:8
			unsigned int Color_R_Y:		8;		//corresponding PL GPIO bit 	23:16
			unsigned int reserved:		8;		//corresponding PL GPIO bit 	31:24
		}color;
	}ColorConfig;
}TarGraphColor;


#endif /* SRC_OSD_GRAPH_TXT_CONFIG_H_ */
