#include <iostream>
#include <queue>
#include "device.h"
#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include "utils.h"
#ifdef OS_UNIX
#include "osd_graph_txt_app.h"
#include "video_app.h"
#include <pthread.h>
#include <thread>
#endif

static double laser_stop_time = -1;  // -1表示未在倒计时
static bool laser_five_sec_showing = false;
static bool laser_had_worked = false;  // 是否曾经进行过测距/照射

#define CHINESE_OSD_IN_PL
#ifdef CHINESE_OSD_IN_PL
	#include "osd_graph_txt_chineseChar_app.h"
#endif

#ifdef OS_UNIX
#define UART422_HOST_RX_ADDR 0x42C00000
#define UART422_HOST_TX_ADDR 0x42C10000
#define UART422_XJ3_RX_ADDR  0x42C20000  //0x42C20000  0x43C80000
#define UART422_XJ3_TX_ADDR  0x42C30000  //0x42C30000  0x43C70000

#define AXI_UART_RXFIFO_REG  0x00
#define AXI_UART_TXFIFO_REG  0x04
#define AXI_UART_STATE_REG  0x08
#define AXI_UART_CTRL_REG  0x0c

#define UART_RX_FIFO_VALID_DATA 0x01
#define UART_TX_FIFO_isFULL 0x08
#define UART_RX_FIFO_FULL   0x02
#define UART_RX_OVERRUN_ERR  0x20

#define UART_RST    0x13

volatile unsigned int *Uart_HOST_RX_HANDLE = (unsigned int *) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t) (UART422_HOST_RX_ADDR + 0x0));
volatile unsigned int *Uart_HOST_TX_HANDLE = (unsigned int *) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t) (UART422_HOST_TX_ADDR + 0x0));
volatile unsigned int *Uart_XJ3_RX_HANDLE = (unsigned int *) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t) (UART422_XJ3_RX_ADDR + 0x0));
volatile unsigned int *Uart_XJ3_TX_HANDLE = (unsigned int *) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t) (UART422_XJ3_TX_ADDR + 0x0));

//2023.02.06  fengyachun
typedef struct{
        union
        {
            unsigned char union_body;
            struct{
                unsigned char temp_abs:		 7;//bit 6~0  	abs value of temperature
                unsigned char pos_neg:		 1;//bit 7 		pos=0   neg=1
            }para;
        }config;
    }TEMP_BYTE;
    TEMP_BYTE temp_byte_reprot;

    typedef struct{
            union
            {
                unsigned char union_body;
                struct{
                    unsigned char breakdown_TV_video:		1;//bit 0
                    unsigned char breakdown_IR_video:		1;//bit 1
                    unsigned char breakdown_Reserved:		1;//bit 2
                    unsigned char TBD:		 				5;//bit 7~3
                }info_detail;
            }info_body;
        }Self_Check;
        Self_Check self_check;

// #define TRACKER_TEST_MODE
#define SERVO_SPEED_LIMIT
// #define DEBUG_KEYBOARD
	static int key_enter_num = 0;
	static int key_EOF_num = 0;
//	bool break_proc_q;
#ifndef DEBUG_KEYBOARD
	struct termios orig_termios;
	void reset_terminal_mode();
	void set_conio_terminal_mode();
	int kbhit();
	int getch();
	// registered exit func, will be called when current proc exit
    void reset_terminal_mode()
    {
    	printf("reset_terminal_mode happen\n");
    	tcsetattr(0, TCSANOW, &orig_termios); //back to default terminal mode
    }
    void set_conio_terminal_mode()
    {
    	struct termios new_settings;
    	/* take two copies - one for now, one for later */
    	tcgetattr(0, &orig_termios);// get the describe of the current general terminal interface.
    	memcpy(&new_settings, &orig_termios, sizeof(new_settings));

    	/* register cleanup handler, and set the new terminal mode */
    	atexit(reset_terminal_mode); // registered exit func for current proc

    	new_settings.c_lflag &= (~ICANON);//change the describe of the current terminal interface.
    	new_settings.c_lflag |= ECHO;
    	new_settings.c_lflag |= ISIG;
    	new_settings.c_cc[VMIN] = 1;
    	new_settings.c_cc[VTIME] = 0;
    	tcsetattr(0, TCSANOW, &new_settings);//using the new describe to the current terminal interface.
    }
    int kbhit()
    {
    	struct timeval tv = { 0L, 0L };
    	fd_set fds;
    	FD_ZERO(&fds);
    	FD_SET(0, &fds);
    	return select(1, &fds, NULL, NULL, &tv);
    }
    int getch()
    {
    	int r;
    	unsigned char c;

    	if ((r = read(0, &c, sizeof(c))) < 0)
    	{
    		return r;
    	}
    	else
    	{
    		return c;
    	}
    }
#endif
#ifdef DEBUG_KEYBOARD
    void keyboard_fun(int n); //keyboard thread func
    void simulate_command(); //keyboard process func
    //fyc 20221107
    // void init_sigwinch_intr();
    // void sigwinch_callback(int signum, siginfo_t *info,void *myact);
	// #define TEST_GS2972_RST
	// #define TEST_LT9211_XJ3MIPI_RST
    void keyboard_fun(int n)
    {
        static struct termios oldt;
        tcgetattr(0, &oldt);//save terminal settings
        oldt.c_lflag &= ~(ICANON | ECHO);//change settings
        tcsetattr(0, TCSANOW, &oldt);    //apply settings
        printf("enter key thread..\n");//<<""<<endl;
        static bool fEnter_Keyboard_Read = 0;
        while(true)
        {
            if(fEnter_Keyboard_Read == 0)
            {
                fEnter_Keyboard_Read = 1;
                // simulate_command();
                // system("stty raw");
                int get_key;
                get_key = getchar();
                if(get_key == 27)// ascii=27: ESC key
                {
//                	break_proc_q = true;
                	exit(0);
                }
                // system("stty cooked");
                // printf("key: %d\n",get_key);
                fEnter_Keyboard_Read=0;
                key_enter_num++;
            }
            // std::this_thread::sleep_for(std::chrono::milliseconds(30));//sleep_thread_us(1);
        }
    }
    void simulate_command()
    {
        char get_key;
        get_key = getchar();
        switch(get_key)
        {
//            case 'q':
//                break_proc_q = true;
//                printf("q exit..\n");
//                break;
    #ifdef MEASURE_LOOP_TIME
            case 'a':
            {
                bias_ctrl_left_mipi = true;
                bias_hapened = true;
            // bias_ctrl_left_disp = true;
                printf("key a \n");
            }
                break;
            case 's':
            {
                bias_ctrl_left_mipi = false;
                bias_rst_hapened = true;
            // bias_ctrl_rst_disp = true;
                printf("key s \n");
            }
                break;
    #endif
#ifdef TEST_GS2972_RST
//            case 'b':
//            {
//            	switch_clk.config.para.rst_gs2972_mannul = 1;
//            	AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[SWITCH_VIDEO_CLK_SRC>>2] = switch_clk.config.union_body;asm("nop");
//            	printf("enable gs2972\n");
//            }
//                break;
//            case 'n':
//            {
//            	switch_clk.config.para.rst_gs2972_mannul = 0;
//            	AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[SWITCH_VIDEO_CLK_SRC>>2] = switch_clk.config.union_body;asm("nop");
//            	printf("disable gs2972\n");
//            }
//                break;
#endif
#ifdef RST_VID_IN_PS_PROC
//            case 'r':
//               	AXI_GPIO_RST_VID_HANDLE[RST_VIDEO_INTO_AXI4S_OFFSET>>2] = 0xf;
//               	for(int i = 0; i<1000000;i++)
//               		asm("nop");
//               	AXI_GPIO_RST_VID_HANDLE[RST_VIDEO_INTO_AXI4S_OFFSET>>2] = 0;asm("nop");
//               	printf("manual rst vid\n");
//            	break;
//            case 's':
//            	AXI_GPIO_RST_VID_HANDLE[0x00>>2] = 0;asm("nop");
//            	printf("init vid\n");
//            	break;
#endif
//        #ifdef XADC
//                    case 'p':
//                        requst_temper = true;
//                        break;
//        #endif
#ifdef TRACKER_TEST_MODE
        case 't':
        {
            HostUARTDevice::instance()->track_command = 2;
            HostUARTDevice::instance()->track_command_changed = true;
            XJ3UARTDevice::instance()->write_command_control(0x2F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
            printf("track start.\n");
        }
            break;
        case 'c':
        {
            HostUARTDevice::instance()->track_command = 1;
            HostUARTDevice::instance()->track_command_changed = true;
            XJ3UARTDevice::instance()->write_command_control(0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
            printf("track stop.\n");
        }
            break;
        case 'd':
        {
            HostUARTDevice::instance()->multi_target_prompt = 1;
            XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
            printf("detect start.\n");
        }
            break;
        case 'f':
        {
            HostUARTDevice::instance()->multi_target_prompt = 2;
            XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF2, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);                
            printf("detect stop.\n");
        }
            break;
#endif
#ifdef BLACK_WHITE_SWITCH_IN_AVT
//        	case 'w':
//        		ir_black_white = !ir_black_white;
//        		ir_black_white_switch_happened = true;
            	//printf("switch black & white status");
//        		break;
#endif
#ifdef TEST_LT9211_XJ3MIPI_RST
       	case 'v':
       		VTC_MIPI_HANDLE[0x00]	= 0xC0000000;	asm("NOP");	//      0xC0000000
       		wait_moment();		//important when reset video into axis stream IP core, must wait a moment
       		VTC_MIPI_HANDLE[0x00]	= 0x00000026;	asm("NOP");	// enable generate"0x0000002F fync_in 0x00000026
       		rst_lt9211 = true;//step-1: prepare to reset LT9211;
            pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst 				= 0;	//step-2: setp LT9211 rst sig
            GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");//step-2: write LT9211 rst sig;
       		printf("manually reset mipi VTC.\n");
       		break;
        // reset XJ3 mipi
        // case 'c':
        //     XJ3UARTDevice::instance()->write_command_control(0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
        //     printf("manually reset XJ3 mipi_cam.\n");
        //     break;
#endif


#ifdef DETECT_VIDEO_INPUT
//        	case 'm':
//        		CLK_INFO switch_clk_tmp;
//        		switch_clk_tmp.config.union_body = (unsigned int)AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[SWITCH_VIDEO_CLK_SRC>>2];
//        		if(switch_clk_tmp.config.para.video_clk_switch == true)
//        			printf("clk src from TV 74.25MHz..\n");
//        		else
//        			printf("clk src from Crystal oscillator 50MHz..\n");
//        		//printf("clk_status: %u  bit: %u union: %u \n",switch_clk_tmp.config.union_body, switch_clk_tmp.config.para.video_clk_switch,switch_clk.config.union_body);
//        		break;
//
//        	case 'x':
//        		video_status.config.union_body = AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[DETECT_VIDEO_INPUT_STATUS_OFFSET>>2];
//        		printf("video status: %u  TV: %d IR: %u \n",video_status.config.union_body, video_status.config.para.status_TV,video_status.config.para.status_IR);
//        	    break;
//        	case 'k':
//        		HostUARTDevice::instance()->ir_cool_finish = true;//testcode-20230207 delete
//        		break;

#endif
        	default:
                break;
        }
    }
#endif

#define BLOCK_SIGBUS
int count_sigbus = 0;
#ifdef BLOCK_SIGBUS  //20221108--fyc
    void init_interrupt_SIGBUS();
    void interrupt_SIGBUS_callback(int signum, siginfo_t *info,void *myact);

    void init_interrupt_SIGBUS()
    {
        struct sigaction sigio_act;
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGBUS);
        sigio_act.sa_sigaction = interrupt_SIGBUS_callback;
        sigio_act.sa_flags = SA_SIGINFO;
        if (sigaction(SIGBUS, &sigio_act, NULL) < 0)
            printf("Failed to init SIGBUS sigaction\n");
//        printf("block sigbus\n");
        return;
    }
    void interrupt_SIGBUS_callback(int signum, siginfo_t *info,void *myact)
    {
        asm("nop");
        printf("self_def bus error proc\n");
    }
#endif

//#define ENABLE_Ctrl_C_kill_AutoRun

#ifdef ENABLE_Ctrl_C_kill_AutoRun
    void init_interrupt_CtrlC();
    void Get_CtrlC_callback(int signum, siginfo_t *info,void *myact);

    void init_interrupt_CtrlC()
	{
        struct sigaction sigio_act;
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigio_act.sa_sigaction = Get_CtrlC_callback;
        sigio_act.sa_flags = SA_SIGINFO;
        if (sigaction(SIGINT, &sigio_act, NULL) < 0)
            printf("Failed to selfdef Ctrl + C intr action\n");
        printf("Enable selfdef Ctrl + C intr action\n");
	}
	void Get_CtrlC_callback(int signum, siginfo_t *info,void *myact)
	{
		printf("capture Ctrl + C, quit app..\n");
		exit(0);
	}
#endif


std::map<std::string, int> interrupt_uart_fds;

#define DEBUG_STD_LENGTH_ERR
#ifdef DEBUG_STD_LENGTH_ERR
	static unsigned int intr_cnt_uart_host = 0;
	static unsigned int intr_cnt_uart_xj3 = 0;
#endif

//HOST UART
// #define UART_PRINT_EVERY
#ifdef UART_PRINT_EVERY
static unsigned short int uart_interrupt_pos;

unsigned char uart_host_byte_arr_1[1024] = {0};

    int host_read_idx = 0;
    void uart_decode_lxl(int nn);

    static unsigned char cmd_data[256] = {0}; //cmd_data[255]
    //call before intr_init_uart
    //long int decode_try_cnt = 0;
    void uart_decode_lxl(int nn)
    {
        //uart_host_byte_arr_1;
        //uart_host_byte_arr_2;
        int off_set = 0;
        int off_end = 0;// 0xcc 0x06 0x06 0x01 0x01 jiaoyan
        while(1)
        {
            int _pay_load = 0;
            //decode_try_cnt++;
            if(off_set>0 || off_end>0)
            {
                int swich_max = 0;
                int _next = off_end;
                if (_next < uart_interrupt_pos){
                    swich_max = uart_interrupt_pos - _next;
                } else if (uart_interrupt_pos < _next) {
                    swich_max = uart_interrupt_pos+1024 - _next;
                }
                // New data should have been more than 3!
                if (swich_max > 3){
                    for (int _t=0;_t<swich_max-3;_t++){
                        unsigned char _check_sum = 0;
                        int _tmp_start_1 = (_next+_t)%1024;
                        int _tmp_start_2 = (_tmp_start_1+1)%1024;
                        int _tmp_start_3 = (_tmp_start_1+2)%1024;
                        if((uart_host_byte_arr_1[_tmp_start_1] == 0xcc) && (uart_host_byte_arr_1[_tmp_start_2] == 0x06) )
                        {
                        // printf("We found the new head 0xcc 0x06, payload being %02x\n",uart_host_byte_arr_1[_tmp_start_3]);
                            _pay_load = uart_host_byte_arr_1[_tmp_start_3];
                            // judge _pay_load + _tmp_start_1 > uart_interrupt_pos
                            int _temp_end = ( _pay_load + _tmp_start_1) % 1024;
                            if ((_temp_end > uart_interrupt_pos && _temp_end - uart_interrupt_pos < 150) || (uart_interrupt_pos > 850 && _temp_end<200))
                            {
                            continue;
                            }
                            for(int j = 3; j < _pay_load - 1;j++)
                            {
                                _check_sum += uart_host_byte_arr_1[(_tmp_start_1 + j)%1024];
                            }

                            if(_check_sum  == uart_host_byte_arr_1[(_tmp_start_1 + _pay_load - 1) % 1024])
                            {
                                off_set = _tmp_start_1;
                                off_end = (_tmp_start_1 + _pay_load) % 1024;
                            //    printf("cmd: ");
                                for(int k = 0; k<_pay_load; k++)
                                {
                                    cmd_data[k] = uart_host_byte_arr_1[(off_set + k) % 1024];
                            //      printf("%02x ",cmd_data[k]);
                                }
                                cmd_data[255] = 0xfd;
                            //   printf("\n ");
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                for(int i = 0; i<1023;i++)
                {
                    if(uart_host_byte_arr_1[i] == 0xcc)
                    {
                        if(uart_host_byte_arr_1[i+1] == 0x06)
                        {
                            unsigned char _check_sum = 0;
                            _pay_load = uart_host_byte_arr_1[i+2];
                        // printf("First payload:%d.\n", _pay_load);
                            for(int j = 3; j < _pay_load - 1;j++)
                            {
                                _check_sum += uart_host_byte_arr_1[i+j];
                            }
                            if(_check_sum == uart_host_byte_arr_1[i + _pay_load - 1])
                            {
                                off_set = i;
                                off_end = i + _pay_load;
                                for(int k = 0; k<_pay_load; k++)
                                {
                                    cmd_data[k] = uart_host_byte_arr_1[(off_set+k)%1024];
                                    //printf("%02x ",cmd_data[k]);
                                }
                                cmd_data[255] = 0xfd;
                                //printf("\n ");
                            // printf("The next byte:%02x.\n",uart_host_byte_arr_1[(off_end) % 1024]);
                                break;
                            }
                        }
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(1000));//sleep_thread_us(1);
        }
    }
#endif

static int loop_nop_num = 2;
int HOST_uart_write_callback(const vector<uint8_t> &data, int len)
{   
    int write_len = 0;
    for (int i = 0; i < len; ++i)
    {
        while (Uart_HOST_TX_HANDLE[AXI_UART_STATE_REG >> 2] & UART_TX_FIFO_isFULL)
        {
        //    std::this_thread::sleep_for(std::chrono::microseconds(10)); //asm("nop"); //changed zzy 2022/12/22
		   for(int i = 0; i<loop_nop_num; i++)  //changed zzy 2023/02/24
		   {
			   asm("nop");
		   }
        }
        Uart_HOST_TX_HANDLE[1] = data[i];
        asm("nop");
    }
    write_len = len;
    return write_len;
}

void interrupt_HOST_uart_callback(int signum, siginfo_t *info,void *myact)
{
	if(info->si_code == 1)  ////irq2_drv  ----HOST_uart			----SIGWINCH     ---- POLL_IN   ---1
	{
		while((Uart_HOST_RX_HANDLE[AXI_UART_STATE_REG>>2] & UART_RX_FIFO_VALID_DATA) == UART_RX_FIFO_VALID_DATA)
		{
			uint8_t data_byte = Uart_HOST_RX_HANDLE[AXI_UART_RXFIFO_REG>>2];
#ifdef		DEBUG_STD_LENGTH_ERR
			intr_cnt_uart_host++;
#endif
#ifdef UART_PRINT_EVERY
            uart_host_byte_arr_1[uart_interrupt_pos] = data_byte;
            uart_interrupt_pos = (uart_interrupt_pos + 1) % 1024;
#endif
			HostUARTDevice::instance()->read_byte(data_byte);        
		}

	}
	if(info->si_code == 2) ////irq1_drv  ----NIR_CL_VS_IN		----SIGIO     ---- POLL_OUT   ---2
	{
	   return;
	}
	if(info->si_code == 3) ////irq2_drv  -----HOST  			----SIGIO     ---- POLL_MSG   ---3
	{
		return;
	}
}
//interrupt config info
//irq2_drv  ----AXI_UART_RS422_RX_HOST	----SIGWINCH  ---- POLL_MSG   ---1
//irq5_drv  ----AXI_UART_RS422_RX_XJ3	----SIGUSR2   ---- POLL_MSG   ---1

bool init_interrupt_HOST_uart()
{
    static bool done_host = false;
    if (done_host)
    {
        return true;
    }
    done_host = true;

    struct sigaction sigio_act;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGWINCH);
    sigio_act.sa_sigaction = interrupt_HOST_uart_callback;
    sigio_act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGWINCH, &sigio_act, NULL) < 0)
    {
        log_critical("Failed to init SIGIO sigaction.");
        return false;
    }

    {
        auto file_name = parse_arg("host_uart_file", "/dev/irq2_drv");  // HOST UART
        int fd = ::open(file_name.c_str(), O_RDWR);
        interrupt_uart_fds[file_name] = fd;
        if (fd < 0)
        {
            log_critical("Failed to open: '%s'", file_name.c_str());
            return false;
        }
        fcntl(fd, F_SETOWN, getpid());
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | FASYNC);
        fcntl(fd, F_SETSIG, SIGWINCH);     // -D_GNU_SOURCE

        Uart_HOST_RX_HANDLE[3] = 0x13;  // init reset host uart ctlr reg to enable (0x10) interrupt and reset rx & tx fifo (0x03)

        log_info("Succeeded open: '%s'", "/dev/irq2_drv");
    }
    return true;
}
// end Host uart 

void interrupt_XJ3_uart_callback(int signum, siginfo_t *info, void *data)
{

    if (info->si_code == 1)  // XJ3
    {              
        while((Uart_XJ3_RX_HANDLE[AXI_UART_STATE_REG>>2] & UART_RX_FIFO_VALID_DATA) == UART_RX_FIFO_VALID_DATA)
        {
            uint8_t data_byte = Uart_XJ3_RX_HANDLE[0x00>>2];
#ifdef		DEBUG_STD_LENGTH_ERR
            intr_cnt_uart_xj3++;//
#endif
            XJ3UARTDevice::instance()->read_byte(data_byte);
			//xj3_buffer.produce(data_byte);
        }   
    }
}

bool init_interrupt_XJ3_uart()
{
    static bool done_xj3 = false;
    if (done_xj3)
    {
        return true;
    }
    done_xj3 = true;

    struct sigaction sigio_act;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);

    sigio_act.sa_sigaction = interrupt_XJ3_uart_callback;
    sigio_act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR2, &sigio_act, NULL) < 0)
    {
        log_critical("Failed to init SIGIO sigaction.");
        return false;
    }

    {
        auto file_name = parse_arg("xj3_uart_file", "/dev/irq5_drv");  // XJ3 UART
        int fd = ::open(file_name.c_str(), O_RDWR);
        interrupt_uart_fds[file_name] = fd;
        if (fd < 0)
        {
            log_critical("Failed to open: '%s'", file_name.c_str());
            return false;
        }
        fcntl(fd, F_SETOWN, getpid());
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | FASYNC);
        fcntl(fd, F_SETSIG, SIGUSR2);  // -D_GNU_SOURCE
        Uart_XJ3_RX_HANDLE[0x04>>2] = 0x01;
        Uart_XJ3_RX_HANDLE[0x0c>>2] = 0x12;  // init reset xj3 uart ctlr reg to enable (0x10) interrupt and reset rx & tx fifo (0x03)

        log_info("Succeeded open: '%s'", file_name.c_str());
    }
    return true;
}


void sigrtmin_5_intr_callback(int signum, siginfo_t *info, void *data)
{
    if (info->si_code == 1)  // video_out_DISP SDI 74.25MHz  30fps
    {
    	video_in_TV();
    }
}
bool sigrtmin_5_intr_init()
{
	struct sigaction sigio_act;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN+5);
	int Oflags;
	int fd_tv_in;

	sigio_act.sa_sigaction = sigrtmin_5_intr_callback;
	sigio_act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGRTMIN+5, &sigio_act, NULL) < 0)
	{
		log_critical("Failed to init SIGRTMIN+5 sigaction.");
		return false;
	}
	{
		fd_tv_in =  open("/dev/irq0_drv", O_RDWR);
		if (fd_tv_in < 0)
		{
			printf("can't open irq0 intr!\n");
		}
		fcntl(fd_tv_in, F_SETOWN, getpid());
		Oflags = fcntl(fd_tv_in, F_GETFL);
		fcntl(fd_tv_in, F_SETFL, Oflags | FASYNC);
		fcntl(fd_tv_in, F_SETSIG, SIGRTMIN+5);
	}
}
void sigrtmin_6_intr_callback(int signum, siginfo_t *info, void *data)
{
    if (info->si_code == 1)  // video_out_DISP SDI 74.25MHz  30fps
    {
    	video_in_IR();
    }
}
bool sigrtmin_6_intr_init()
{
	struct sigaction sigio_act;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN+6);
	int Oflags;
	int fd_ir_in;
	sigio_act.sa_sigaction = sigrtmin_6_intr_callback;
	sigio_act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGRTMIN+6, &sigio_act, NULL) < 0)
	{
		log_critical("Failed to init SIGRTMIN+6 sigaction.");
		return false;
	}
	{
		fd_ir_in =  open("/dev/irq1_drv", O_RDWR);
		if (fd_ir_in < 0)
		{
			printf("can't open irq1 intr!\n");
		}
		fcntl(fd_ir_in, F_SETOWN, getpid());
		Oflags = fcntl(fd_ir_in, F_GETFL);
		fcntl(fd_ir_in, F_SETFL, Oflags | FASYNC);
		fcntl(fd_ir_in, F_SETSIG, SIGRTMIN+6);
	}
}
void sigrtmin_7_intr_callback(int signum, siginfo_t *info, void *data)
{
    if (info->si_code == 1)  // video_out_DISP SDI 74.25MHz  30fps
    {
        video_out_DISP();
    }
}
bool sigrtmin_7_intr_init()
{
	struct sigaction sigio_act;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN+7);
	int Oflags;
	int fd_display_out;
	sigio_act.sa_sigaction = sigrtmin_7_intr_callback;
	sigio_act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGRTMIN+7, &sigio_act, NULL) < 0)
	{
		log_critical("Failed to init SIGRTMIN+7 sigaction.");
		return false;
	}
	{
		fd_display_out =  open("/dev/irq3_drv", O_RDWR);
		if (fd_display_out < 0)
		{
			printf("can't open irq3 intr!\n");
		}
		fcntl(fd_display_out, F_SETOWN, getpid());
		Oflags = fcntl(fd_display_out, F_GETFL);
		fcntl(fd_display_out, F_SETFL, Oflags | FASYNC);
		fcntl(fd_display_out, F_SETSIG, SIGRTMIN+7);
	}
}
void sigsur1_intr_callback(int signum, siginfo_t *info, void *data)
{
    if (info->si_code == 1)  // video_out_DISP SDI 74.25MHz  30fps
    {
    }
    if (info->si_code == 2)  // video_out_SDI_MIPI BT1120 148.5MHz 60fps
    {
        video_out_SDI_MIPI();
    }
    if(info->si_code == 3)	//tv vs in	Cameralink 74.25MHz 30fps
    {
    	//video_in_TV();
    }
}
bool sigsur1_intr_init()
{
	video_info_MIPI_OUT.ready = false;
	video_info_MIPI_OUT.img_buf = 0;
	video_info_DISP_OUT.ready = false;
	video_info_DISP_OUT.img_buf = 0;
	video_info_TV_IN.ready = false;
	video_info_TV_IN.img_buf = 0;

    struct sigaction sigio_act;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    int Oflags;
    int fd_tv_in, fd_display_out, fd_mipi_out;

    sigio_act.sa_sigaction = sigsur1_intr_callback;
    sigio_act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &sigio_act, NULL) < 0)
    {
        log_critical("Failed to init SIGIO sigaction.");
        return false;

        //TV in vs signal detected

        //display video out vs signal detected
    }

    {
        //mipi video out vs signal detected
        fd_mipi_out =  open("/dev/irq4_drv", O_RDWR);
        if (fd_mipi_out < 0)
        {
            printf("can't open irq4 intr!\n");
        }
        fcntl(fd_mipi_out, F_SETOWN, getpid());
        Oflags = fcntl(fd_mipi_out, F_GETFL);
        fcntl(fd_mipi_out, F_SETFL, Oflags | FASYNC);
        fcntl(fd_mipi_out, F_SETSIG, SIGUSR1);
    }
    return true;
}

void sigio_intr_callback(int signum, siginfo_t *info,void *myact)
{
	if(info->si_code == 1) //POLL IN
	{
#ifdef PHOTO_IN_DVP_OUT
		PHOTO_in();
#endif
	}
	if(info->si_code == 2) ////irq1_drv  ----IR_CL_VS_IN		----SIGIO     ---- POLL_OUT   ---2
	{
        //video_in_IR();
	}
	if(info->si_code == 3)//POLL MSG
	{

	}
}
bool sigio_intr_init()
{
	video_info_IR_IN.ready = false;
	video_info_IR_IN.img_buf = 0;
	//init video interrupt
	int Oflags;
	struct sigaction sigio_act;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGIO);
	sigio_act.sa_sigaction = sigio_intr_callback;// set SIGIO interrupt callback func to video_intr_callback()
	sigio_act.sa_flags = SA_SIGINFO;

	int fd_ir_in,fd_photo_in,fd_dvp_out;

	if(sigaction(SIGIO, &sigio_act, NULL) < 0)
	{
		printf("install SIGIO sigaction error");
		return false;
	}

		//ir video in vs signal detected
#ifdef PHOTO_IN_DVP_OUT
		{
			//ir video in vs signal detected
			fd_photo_in =  open("/dev/irq6_drv", O_RDWR);
			if (fd_photo_in < 0)
			{
				printf("can't open irq6 intr!\n");
			}
			fcntl(fd_photo_in, F_SETOWN, getpid());
			Oflags = fcntl(fd_photo_in, F_GETFL);
			fcntl(fd_photo_in, F_SETFL, Oflags | FASYNC);
			fcntl(fd_photo_in, F_SETSIG, SIGIO);
		}
				//ir video in vs signal detected
#endif
	return true;
}

#endif

#ifdef PHOTO_IN_DVP_OUT
	void sigpipe_intr_callback(int signum, siginfo_t *info,void *myact)
	{
	   if (info->si_code == 1)  // video_out_DISP SDI 74.25MHz  30fps
		{
			DVP_out_rk3588();
		}
		if (info->si_code == 2)  // video_out_SDI_MIPI BT1120 148.5MHz 60fps
		{

		}
		if(info->si_code == 3)	//tv vs in	Cameralink 74.25MHz 30fps
		{

		}
	}

	bool sigpipe_intr_init()
	{
		//init video interrupt
		int Oflags;
		struct sigaction sigpipe_act;
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask,SIGPIPE);
		sigpipe_act.sa_sigaction = sigpipe_intr_callback;// set SIGIO interrupt callback func to video_intr_callback()
		sigpipe_act.sa_flags = SA_SIGINFO;

		int fd_dvp_out;

		if(sigaction(SIGPIPE, &sigpipe_act, NULL) < 0)
		{
			printf("install SIGPIPE sigaction error");
			return false;
		}

		{
			//ir video in vs signal detected
			fd_dvp_out =  open("/dev/irq7_drv", O_RDWR);
			if (fd_dvp_out < 0)
			{
				printf("can't open irq7 intr!\n");
			}
			fcntl(fd_dvp_out, F_SETOWN, getpid());
			Oflags = fcntl(fd_dvp_out, F_GETFL);
			fcntl(fd_dvp_out, F_SETFL, Oflags | FASYNC);
			fcntl(fd_dvp_out, F_SETSIG, SIGPIPE);
		}

		return true;
	}
#endif

int cnt_photo_in = 0;
int cnt_dvp_out = 0;
#ifdef TV_ALIGN_ON_FMQL
	void fly_axis_crc(bool &res, unsigned char *ptrImg_buf, float &center_hor, float &center_ver);
#endif


//#define ADD_dot_line
#ifdef ADD_dot_line

	#define AXI_BRAM_CTRL_DOTLINE_00 0x44000000
	#define AXI_BRAM_CTRL_DOTLINE_01 0x46000000
	#define AXI_BRAM_CTRL_DOTLINE_10 0x48000000
	#define AXI_BRAM_CTRL_DOTLINE_11 0x4A000000
	unsigned int* AXI_BRAM_CTRL_DOTLINE_00_HANDLE = (unsigned int*)mmap(NULL, 4*1024, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)AXI_BRAM_CTRL_DOTLINE_00);
	unsigned int* AXI_BRAM_CTRL_DOTLINE_01_HANDLE = (unsigned int*)mmap(NULL, 4*1024, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)AXI_BRAM_CTRL_DOTLINE_01);
	unsigned int* AXI_BRAM_CTRL_DOTLINE_10_HANDLE = (unsigned int*)mmap(NULL, 4*1024, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)AXI_BRAM_CTRL_DOTLINE_10);
	unsigned int* AXI_BRAM_CTRL_DOTLINE_11_HANDLE = (unsigned int*)mmap(NULL, 4*1024, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)AXI_BRAM_CTRL_DOTLINE_11);

	unsigned short pt_arr[1024];
	static int start_rows = 0; //img_rows_start
	static int start_cols = 0; //img_cols_start
	static int end_rows = 0;//img_rows_end 270+540
	static int end_cols = 0; //img_cols_end 480+960

	void gen_line(int x0, int y0, int x1, int y1, int width, unsigned short *pt_arr);
	void init_dot_line_bram_buf();
	void update_dot_arr(unsigned int* bram_handle, int loc, unsigned short *pt_arr);
	void clear_dot_line_buf();

	void gen_line(int x0, int y0, int x1, int y1, int width, unsigned short *pt_arr)
	{
		bool steep = false;
		if (std::abs(x1 - x0) < std::abs(y1 - y0))
		{
			std::swap(x0, y0);
			std::swap(x1, y1);
			steep = true;
		}
		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		int dx = std::abs(x1 - x0);
		int dy = std::abs(y1 - y0);
		int deltaY = (dy << 1);//deltaY * 2
		int middle = dx;
		int y = y0;
		int valid = 0;
		int forward_cnt = 0;
		bool b_valid = 0;
		for (int x = x0; x <= x1; ++x)
		{
			if (steep)
			{
				for (int i = -width / 2; i <= width / 2; ++i)
				{
					if (b_valid)
					{
						pt_arr[forward_cnt + 1] = 2 * x;
						pt_arr[forward_cnt] = 2 * (y + i);
						forward_cnt += 2;
					}
					valid++;
					if ((valid >> 1) % 2 == 1)
					{
						b_valid = true;
					}
					else
					{
						b_valid = false;
					}

				}
			}
			else
			{
				for (int i = -width / 2; i <= width / 2; ++i)
				{
					if (b_valid)
					{
						pt_arr[forward_cnt + 1] = 2 * (y + i);
						pt_arr[forward_cnt] = 2 * x;
						forward_cnt += 2;
					}

					valid++;
					if ((valid >> 1) % 2 == 1)
					{
						b_valid = true;
					}
					else
					{
						b_valid = false;
					}
				}
			}
			deltaY += (dy << 1);
			if (deltaY >= middle)
			{
				y += (y1 > y0 ? 1 : -1);
				middle += ((dx << 1));
			}
		}
		//printf("forward_cnt: %d\n",forward_cnt);
	}

	void init_dot_line_bram_buf()
	{
		for(int i = 0; i < 1024; i++)
		{
			AXI_BRAM_CTRL_DOTLINE_00_HANDLE[i] = 0	; asm("nop");
			AXI_BRAM_CTRL_DOTLINE_01_HANDLE[i] = 0	; asm("nop");
			AXI_BRAM_CTRL_DOTLINE_10_HANDLE[i] = 0	; asm("nop");
			AXI_BRAM_CTRL_DOTLINE_11_HANDLE[i] = 0	; asm("nop");
		}
	}

	void update_dot_arr(unsigned int* bram_handle, int loc, unsigned short *pt_arr)
	{

		switch(loc)
		{
		case 0:
				{
					for(int i = 0; i < 512; i++)
					{
						bram_handle[(4*(i+1))>> 2] = ((pt_arr[2*i+1])	<<16) 	+ pt_arr[2*i]	; asm("nop");
					}
				}
			break;
		case 1:
				{
					for(int i = 0; i < 512; i++)
					{
						bram_handle[(4*(i+1))>> 2] = ((pt_arr[2*i+1]+1 )<<16) 	+ pt_arr[2*i]	; asm("nop");
					}
				}
			break;
		case 2:
				{
					for(int i = 0; i < 512; i++)
					{
						bram_handle[(4*(i+1))>> 2] = ((pt_arr[2*i+1] )	<<16) 	+ pt_arr[2*i] + 1; asm("nop");
					}
				}
			break;
		case 3:
				{
					for(int i = 0; i < 512; i++)
					{
						bram_handle[(4*(i+1))>> 2] = ((pt_arr[2*i+1]+1)	<<16) 	+ pt_arr[2*i] + 1; asm("nop");
					}
				}
			break;
		default:
			break;
		}
	}


#endif

#define ADD_dot_line_mixer
#ifdef ADD_dot_line_mixer
	void gen_line(int x0, int y0, int x1, int y1, int width, unsigned short *pt_arr);
	void clear_pre_dot(unsigned char *img_layer_buf,unsigned short *pt_pre);
	void draw_cur_dot(unsigned char *img_layer_buf,unsigned short *pt_cur,unsigned char color_y,unsigned char  color_u,unsigned char  color_v);
	void clear_dot_line();
	void draw_dot_line(int start_rows, int start_cols, int end_rows, int end_cols,unsigned char  color_y,unsigned char  color_u,unsigned char  color_v);
	#define max_dot_num 512
	unsigned short pt_arr_pre[2*max_dot_num];
	unsigned short pt_arr_cur[2*max_dot_num];

	void clear_dot_line()
	{
		clear_pre_dot(ptr_MIXER_LAYER_TXT_BUF,pt_arr_pre);
		memset(pt_arr_pre,0,1024*2);
	}
	void draw_dot_line(int start_rows, int start_cols, int end_rows, int end_cols,unsigned char  color_y,unsigned char  color_u,unsigned char  color_v)
	{
		if((start_rows>1077 || start_rows<0) || (start_cols>1917 || start_cols<0)  ||  (end_rows>1077 || end_rows<0) || (end_cols>1917 || end_cols<0) )
		{
			//printf("illegal input para start_rows: %d start_cols: %d end_rows: %d end_cols: %d\n");
			return;
		}
		memset(pt_arr_cur,0,1024*2);
		clear_pre_dot(ptr_MIXER_LAYER_TXT_BUF,pt_arr_pre);
		gen_line(start_rows/2, start_cols/2, end_rows/2, end_cols/2, 1, pt_arr_cur);
		draw_cur_dot(ptr_MIXER_LAYER_TXT_BUF,pt_arr_cur,color_y,color_u,color_v);
		memcpy(pt_arr_pre,pt_arr_cur,1024*2);
	}

	void gen_line(int x0, int y0, int x1, int y1, int width, unsigned short *pt_arr)
	{
		bool steep = false;
		if (std::abs(x1 - x0) < std::abs(y1 - y0))
		{
			std::swap(x0, y0);
			std::swap(x1, y1);
			steep = true;
		}
		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		int dx = std::abs(x1 - x0);
		int dy = std::abs(y1 - y0);
		int deltaY = (dy << 1);//deltaY * 2
		int middle = dx;
		int y = y0;
		int valid = 0;
		int forward_cnt = 0;
		bool b_valid = 0;
		for (int x = x0; x <= x1; ++x)
		{
			if (steep)
			{
				for (int i = -width / 2; i <= width / 2; ++i)
				{
					if (b_valid)
					{
						pt_arr[forward_cnt + 1] = 2 * x;
						pt_arr[forward_cnt] = 2 * (y + i);
						forward_cnt += 2;
					}
					valid++;
					if ((valid >> 1) % 2 == 1)
					{
						b_valid = true;
					}
					else
					{
						b_valid = false;
					}

				}
			}
			else
			{
				for (int i = -width / 2; i <= width / 2; ++i)
				{
					if (b_valid)
					{
						pt_arr[forward_cnt + 1] = 2 * (y + i);
						pt_arr[forward_cnt] = 2 * x;
						forward_cnt += 2;
					}

					valid++;
					if ((valid >> 1) % 2 == 1)
					{
						b_valid = true;
					}
					else
					{
						b_valid = false;
					}
				}
			}
			deltaY += (dy << 1);
			if (deltaY >= middle)
			{
				y += (y1 > y0 ? 1 : -1);
				middle += ((dx << 1));
			}
		}
//		printf()
		//printf("x0: %d y0: %d x1: %d y1: %d seg: %d\n",x0, y0, x1, y1,forward_cnt);
	}

	void clear_pre_dot(unsigned char *img_layer_buf,unsigned short *pt_pre)
	{
		int tmp_ver = 0;
		int tmp_hor = 0;
		for(int i = 0;i<max_dot_num;i++)
		{
			tmp_ver = pt_pre[2*i];		//pt_pre[2*i]=ver;
			tmp_hor = pt_pre[2*i+1];	//pt_pre[2*i+1]=hor
			if(tmp_ver>1077 || tmp_hor>1917 )
				continue;
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 0] = 0;
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 1] = 0;
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 2] = 0;
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 3] = 0;
			//01
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 0] = 0;
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 1] = 0;
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 2] = 0;
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 3] = 0;
			//10
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 0] = 0;
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 1] = 0;
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 2] = 0;
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 3] = 0;
			//11
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 0] = 0;
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 1] = 0;
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 2] = 0;
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 3] = 0;
		}
	}

	void draw_cur_dot(unsigned char *img_layer_buf,unsigned short *pt_cur,unsigned char  color_y,unsigned char  color_u,unsigned char  color_v)
	{
		int tmp_ver = 0;
		int tmp_hor = 0;
		for(int i = 0;i<max_dot_num;i++)
		{
			tmp_ver = pt_cur[2*i];		//pt_pre[2*i]=ver;
			tmp_hor = pt_cur[2*i+1];	//pt_pre[2*i+1]=hor
			if(tmp_ver>1077 || tmp_hor>1917 || tmp_ver==0 || tmp_hor==0)
				continue;
			//00
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 0] = color_y;
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 1] = color_v;
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 2] = color_u;
			img_layer_buf[tmp_ver*1920*4 + tmp_hor*4 + 3] = 255;
			//01
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 0] = color_y;
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 1] = color_v;
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 2] = color_u;
			img_layer_buf[tmp_ver*1920*4 + (tmp_hor+1)*4 + 3] = 255;
			//10
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 0] = color_y;
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 1] = color_v;
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 2] = color_u;
			img_layer_buf[(tmp_ver+1)*1920*4 + tmp_hor*4 + 3] = 255;
			//11
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 0] = color_y;
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 1] = color_v;
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 2] = color_u;
			img_layer_buf[(tmp_ver+1)*1920*4 + (tmp_hor+1)*4 + 3] = 255;
		}
	}

#endif
int main(int argc, char *argv[])
{
    set_args(argc, argv);

    const auto app_name = "Image Processing System (BUAA) - ProcessorZYNQ";
    bool allow_invalid_device = std::stoi(parse_arg("allow_invalid_device", "0"));
    bool use_osd = std::stoi(parse_arg("use_osd", "1"));

//RESET UART
#ifdef RST_UART_IN_PS_PROC
     AXI_GPIO_RST_UART_HANDLE[RST_GPIO_RST_UART_OFFSET>>2] = 0;asm("nop");
     wait_moment();
     wait_moment();
     wait_moment();
     AXI_GPIO_RST_UART_HANDLE[RST_GPIO_RST_UART_OFFSET>>2] = 0xf;asm("nop");
     //AXI_GPIO_RST_UART_HANDLE[0x08>>2] = 1;                           //uart recive 8 byte out one interupt
     std::this_thread::sleep_for(std::chrono::milliseconds(20));		//sleep_thread_us(1);
     log_info("UART reset OK");
#endif

#ifdef UART_PRINT_EVERY
    std::thread thread_uart_host_decode(uart_decode_lxl, 0);     //start
    std::this_thread::sleep_for(std::chrono::microseconds(1000));//sle1ep_thread_us(1);
#endif
    logger_init();
    log_info("%s (%s %s)", app_name, __DATE__, __TIME__);
    log_info("Command line: %s", join(argc, argv, " ").c_str());

    const auto software_version = "Version 0.1.0.2, 2025-01-15";
    log_info("%s", software_version);
#ifdef OS_UNIX
    // video_init();
    sigio_intr_init();			    //init IR vs in interrupt callback
    sigsur1_intr_init();			//init [TV vs in]/[display vs out]/[mipi vs out] interrupt callback
#endif

#ifdef PHOTO_IN_DVP_OUT
    sigpipe_intr_init();
#endif
	sigrtmin_5_intr_init();
	sigrtmin_6_intr_init();
	sigrtmin_7_intr_init();

//  static struct termios oldt;
//	tcgetattr(0, &oldt);				//save terminal settings
//	oldt.c_lflag &= ~(ICANON | ECHO);	//change settings
//	tcsetattr(0, TCSANOW, &oldt);    	//apply settings

#ifdef DEBUG_KEYBOARD
    std::thread thread_keyboard(keyboard_fun, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));		//sleep_thread_us(1);
#endif
#ifndef DEBUG_KEYBOARD
    std::this_thread::sleep_for(std::chrono::milliseconds(10));		//sleep_thread_us(1);
	int ch = 0;
	int ch_cur = 0;
	int ch_pre = 0;
	set_conio_terminal_mode();
//  printf("keyboard non blocking mode setting ok\n");
#endif

#ifdef BLOCK_SIGBUS
    init_interrupt_SIGBUS();    //20221108--fyc
#endif
#ifdef ENABLE_Ctrl_C_kill_AutoRun
    init_interrupt_CtrlC();
#endif

    auto open_device = [&](Device *device) {
        device->open();
        if (!device->is_open())
        {
            log_critical("Failed to open device %s.", device->get_name().c_str());
            if (!allow_invalid_device)
            {
                exit(1);
            }
        }
        else
        {
            log_info("Device %s ready.", device->get_name().c_str());
        }
        return device;
    };

    auto close_device = [&](Device *device) {
        device->close();
    };
    open_device(HostUARTDevice::instance());
    open_device(XJ3UARTDevice::instance());
    auto quit = [&]() {
        close_device(HostUARTDevice::instance());
        close_device(XJ3UARTDevice::instance());
        exit(0);
    };
    printf("uart ready..\n");
#ifdef OS_UNIX
    cOSD_MAPPING osd;

    disp_ctrl.SWITCH_HAPPENED = false;
    disp_ctrl.DISP_MODE = TV_MAIN_MODE;
    disp_ctrl.SCALING_RATE_TV = SCALING_X1;
    disp_ctrl.SCALING_RATE_IR = SCALING_X1;
    disp_ctrl.PIP_WIDTH = 640;
    disp_ctrl.PIP_HEIGHT = 360;
    disp_ctrl.PIP_START_X = 0;
    disp_ctrl.PIP_START_Y = 0;
    disp_ctrl.layer_ctrl.layer_TV_EN = 1;
    disp_ctrl.layer_ctrl.layer_IR_EN = 0;
    disp_ctrl.layer_ctrl.layer_PIP_EN = 0;
    disp_ctrl.layer_ctrl.layer_OSD_EN = 1;
    disp_ctrl.layer_ctrl.reserved = 0;

//    osd.init();
    init_tar_graph();
//    printf("enter init video\n");
    video_init(); //2023.01.31 modified by fyc , delete VTC_reset partion in func video_init() only config VDMA & resizeIP core & Video Mxier & GammaLut

#ifdef CHINESE_OSD_IN_PL
    osd_init_chinese();
#endif

    cv::FileStorage fs;
    static uint8_t show_color = 2;
    static uint8_t picture_in_picture_position;
    static int cross_x_main = 960;
    static int cross_y_main = 540;
    static int offset_x_pip = 0;
    static int offset_y_pip = 0;

    static int cross_x_main_tv_l = 960;
    static int cross_y_main_tv_l = 540;
    static int offset_x_pip_tv_l = 0;
    static int offset_y_pip_tv_l = 0;

    static int cross_x_main_tv_m = 960;
    static int cross_y_main_tv_m = 540;
    static int offset_x_pip_tv_m = 0;
    static int offset_y_pip_tv_m = 0;

    static int cross_x_main_tv_s = 960;
    static int cross_y_main_tv_s = 540;
    static int offset_x_pip_tv_s = 0;
    static int offset_y_pip_tv_s = 0;

    static int cross_x_main_tv_z = 960;
    static int cross_y_main_tv_z = 540;
    static int offset_x_pip_tv_z = 0;
    static int offset_y_pip_tv_z = 0;

    static int cross_x_main_ir_l = 960;
    static int cross_y_main_ir_l = 540;
    static int offset_x_pip_ir_l = 0;
    static int offset_y_pip_ir_l = 0;

    static int cross_x_main_ir_m = 960;
    static int cross_y_main_ir_m = 540;
    static int offset_x_pip_ir_m = 0;
    static int offset_y_pip_ir_m = 0;

    static int cross_x_main_ir_s = 960;
    static int cross_y_main_ir_s = 540;
    static int offset_x_pip_ir_s = 0;
    static int offset_y_pip_ir_s = 0;

    static int cross_x_main_ir_z = 960;
    static int cross_y_main_ir_z = 540;
    static int offset_x_pip_ir_z = 0;
    static int offset_y_pip_ir_z = 0;
#define ENABLE_XML_FILE_SAVED_FUNC
//#define TEST_SYNC		//20230209 fengyachun

    //2023.02.04 modified by fengyachun
//    printf("reading xml file\n");
    try
    {
    	fs.open("/root/color.xml", cv::FileStorage::READ);
    }
    catch(cv::Exception)
    {
    	printf("read color faild..\n");
    }

    if(!fs.isOpened())
    {
    	HostUARTDevice::instance()->show_color = 2;
    	printf("using defalt color\n");

    	// recover xml file using default setup
        cv::FileStorage fs_tmp("/root/color.xml", cv::FileStorage::WRITE);
        fs_tmp << "color" << HostUARTDevice::instance()->show_color;
        fs_tmp.release();
        system("sync");//2023.02.03 added by fyc---sync
        printf("recover cfg file using defalt color\n");
    }
    else
    {
    	fs["color"] >> HostUARTDevice::instance()->show_color;
    	printf("using saved color\n");
    }
    osd.switch_osd_color(HostUARTDevice::instance()->show_color);
    fs.release();//2023.02.03 added by fyc---sync
    //system("sync");//2023.02.03 added by fyc---sync
    				//annotated by fengyachun 20230209

    try
    {
    	fs.open("/root/pip.xml", cv::FileStorage::READ);
    }
    catch(cv::Exception)
    {
    	printf("read pip xml faild..\n");
    }

    if(!fs.isOpened())
    {
    	printf("using defalt pip loc\n");
    	HostUARTDevice::instance()->picture_in_picture_position = 0;
     	// recover xml file using default setup
         cv::FileStorage fs_tmp("/root/pip.xml", cv::FileStorage::WRITE);
         fs_tmp << "pic_in_pic" << picture_in_picture_position;
         fs_tmp.release();
         system("sync");//2023.02.03 added by fyc---sync
         printf("recover cfg file using defalt pip loc\n");

    }
    else
    {
    	fs["pic_in_pic"] >> HostUARTDevice::instance()->picture_in_picture_position;
    	printf("using saved pip loc\n");
    }
    //HostUARTDevice::instance()->picture_in_picture_position = picture_in_picture_position;
    fs.release();//2023.02.03 added by fyc---sync
   // system("sync");//2023.02.03 added by fyc---sync
    				//annotated by fengyachun 20230209

    try
	{
//    	fs.open("/root/cross_tv_l.xml", cv::FileStorage::READ);
    	fs.open("/root/cross.xml", cv::FileStorage::READ);//2023.02.09 modified by fengyachun
	}
	catch(cv::Exception)
	{
		printf("read cross xml faild..\n");
	}

    if(!fs.isOpened())
    {
        HostUARTDevice::instance()->cross_x_main = 960;
        HostUARTDevice::instance()->cross_y_main = 540;
        HostUARTDevice::instance()->cross_x_pip  = 960;
        HostUARTDevice::instance()->cross_y_pip  = 540;
        printf("using defalt cross loc\n");
    }
    else
    {
        fs["cross_x_main_tv_l"] >> cross_x_main_tv_l;
        fs["cross_y_main_tv_l"] >> cross_y_main_tv_l;
        fs["offset_x_pip_tv_l"] >> offset_x_pip_tv_l;
        fs["offset_y_pip_tv_l"] >> offset_y_pip_tv_l;

        fs["cross_x_main_tv_m"] >> cross_x_main_tv_m;
        fs["cross_y_main_tv_m"] >> cross_y_main_tv_m;
        fs["offset_x_pip_tv_m"] >> offset_x_pip_tv_m;
        fs["offset_y_pip_tv_m"] >> offset_y_pip_tv_m;

        fs["cross_x_main_tv_s"] >> cross_x_main_tv_s;
        fs["cross_y_main_tv_s"] >> cross_y_main_tv_s;
        fs["offset_x_pip_tv_s"] >> offset_x_pip_tv_s;
        fs["offset_y_pip_tv_s"] >> offset_y_pip_tv_s;

        fs["cross_x_main_tv_z"] >> cross_x_main_tv_z;
        fs["cross_y_main_tv_z"] >> cross_y_main_tv_z;
        fs["offset_x_pip_tv_z"] >> offset_x_pip_tv_z;
        fs["offset_y_pip_tv_z"] >> offset_y_pip_tv_z;

        fs["cross_x_main_ir_l"] >> cross_x_main_ir_l;
        fs["cross_y_main_ir_l"] >> cross_y_main_ir_l;
        fs["offset_x_pip_ir_l"] >> offset_x_pip_ir_l;
        fs["offset_y_pip_ir_l"] >> offset_y_pip_ir_l;

        fs["cross_x_main_ir_m"] >> cross_x_main_ir_m;
        fs["cross_y_main_ir_m"] >> cross_y_main_ir_m;
        fs["offset_x_pip_ir_m"] >> offset_x_pip_ir_m;
        fs["offset_y_pip_ir_m"] >> offset_y_pip_ir_m;

        fs["cross_x_main_ir_s"] >> cross_x_main_ir_s;
        fs["cross_y_main_ir_s"] >> cross_y_main_ir_s;
        fs["offset_x_pip_ir_s"] >> offset_x_pip_ir_s;
        fs["offset_y_pip_ir_s"] >> offset_y_pip_ir_s; 

        fs["cross_x_main_ir_z"] >> cross_x_main_ir_z;
        fs["cross_y_main_ir_z"] >> cross_y_main_ir_z;
        fs["offset_x_pip_ir_z"] >> offset_x_pip_ir_z;
        fs["offset_y_pip_ir_z"] >> offset_y_pip_ir_z;       

        HostUARTDevice::instance()->cross_x_main = cross_x_main_tv_l;
        HostUARTDevice::instance()->cross_y_main = cross_y_main_tv_l;
        HostUARTDevice::instance()->cross_x_pip = 960 + offset_x_pip_tv_l;
        HostUARTDevice::instance()->cross_y_pip = 540 + offset_y_pip_tv_l;

        printf("using saved cross loc\n");
    }
    fs.release();
    //system("sync");//2023.02.03 added by fyc---sync
    					//annotated by fengyachun 20230209

#endif

    log_info("Ready!");
    static uint8_t track_command = 1;
    static uint8_t track_status  = 1;
    static uint8_t slide_status  = 0; // 0: 默认状态 1:进入滑行，未上报 2: 滑行多帧，上报
    static uint8_t multi_target_prompt = 2;
    static uint8_t track_ID      = 1;
    static int16_t report_x      = 9600;
    static int16_t report_y      = 5400;
    static int16_t offset_x      = 0;
    static int16_t offset_y      = 0;
    static int16_t delta_x       = 0;
    static int16_t delta_y       = 0;
    static int16_t yaw_v_stable    = 0;
    static int16_t pitch_v_stable  = 0; 
    static uint8_t reached_x     = 2;
    static uint8_t reached_y     = 2;
    static uint8_t reached_x_servo   = 2;
    static uint8_t reached_y_servo   = 2;
    static int16_t display_x     = 960;
    static int16_t display_y     = 540;
    static int16_t cross_symbol_x = -1;
    static int16_t cross_symbol_y = -1;
    static uint8_t tv_wave_gate_size = 2;
    static uint8_t ir_wave_gate_size = 1;
    static uint8_t target_type = 1; 
    static uint8_t resend_delay = 5;
    static uint8_t multi_type = 8; 
    static int count_multi_type = -10;
    static int count = -1;
    static int watchdog_mipi_count = -1;
    static int count_mipi_ok = 0;
    static int normal_track_start = -100;
    static int count_track_start = -100;
    static int count_tv_mode = -100;
    static int count_ch_switch = -10;
    static int count_electron = -10;
    static int count_ir_enhance = -100;
    static int count_clear_ir_buffer = -30;
    static int count_self_check_error = -100;
    static int count_prompt_start = -1;
    static bool ir_align = false;
    static bool tv_align = false;
    static int count_ir_align = -10;
    static int count_tv_align = -10;
    static int count_photo_start = -100;
    static uint8_t send_photo_status = 0;
    static bool is_tv = true;  
    static uint8_t self_check_status = 1;
    self_check.info_body.union_body = 0;
    unsigned char pre_self_check_status = 0;
    static uint8_t status_TV_pre = 1;
    static uint8_t status_IR_pre = 0;
    static uint8_t view_scale = 2; 
    static uint8_t tv_scale = 2;
    static uint8_t ir_scale = 2;
    static uint8_t tv_enhance_on = 2;
    static uint8_t work_mode = 0;
    static uint8_t disp_mode = TV_MAIN_MODE;
    static int init_count = 0;
    static unsigned char sys_init_str_cache[6][32];
    static uint8_t pic_in_pic = 2;
    static uint8_t show_level = 1;
    static bool xj3_send_command = false;
    static bool servo_start = false;
    static bool visible = true;
    static bool target_size_visible = false;
    static bool wave_gate_visible = true;
    static uint8_t second_capture = 2;
    static bool channel_changed = false;
    static bool enhance_level_changed = false;
    static bool cross_main_changed = false;
    static bool cross_main_send = false;
    static bool cross_main_init = true; 
    static uint8_t tv_zoom_status = 0;
    static uint8_t ir_zoom_status = 0;
    static uint8_t sync_422_speed = 3;

    /* sensor size, view angle, enhancement, view status, brightness and contrastness, ir status */
    static uint8_t channel = 99;
    static uint8_t channel_disp = 99;
    static uint8_t enhance_level = 'f';
    static uint8_t enhance_level_tv = 'f';
    static uint8_t enhance_level_ir = 'f';
    static uint8_t enhance_level_zoom = 'f';
    static uint8_t sensor_view_size = 9;
    static uint8_t tv_view_size = 9;
    static uint8_t ir_view_size = 9;
    static uint8_t brigheness_contrast_modify = 'f';
    static uint8_t ir_pola = 0xff;
    static std::string yaw_view_angle_str = "---";
    static float north_angle = 0;
    static std::string north_angle_str = "---";
    static bool ir_power = false;
    static int16_t init_altitude = -1;
    static uint16_t aircraft_altitude = 0;

#ifdef IR_CLOSE_VDMA_S2MM_20240125
    static bool ir_s2mm_stat = false;
#endif

    static bool laser_power = false;
    bool tv_mode = false;
    bool tv_mode_change_start = false;
    bool tv_mode_change_end = false;
    bool level_changed = false;            
    int timer1 = 0;
    int timer2 = 0;
    //cv::Mat img_TV(240,320,CV_8UC3);
    //int img_count = 0;
    static uint8_t sdi_first_line[256] = {48};
    unsigned char cmd_update_data[256] = {0};
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
#ifdef XADC							// 2023.01.31 added by fengyachun to report the temperature of FMQL-7045
	//init XADC
	XADC_HANDLE[0x00>>2] = 0xA;
	XADC_HANDLE[0x10>>2] = 0x1;
	XADC_HANDLE[0x10>>2] = 0x0;
#endif


#ifdef RST_VID_IN_PS_PROC
    //2023.01.05  to handle randomly happened gray screen--------------fengyachun & yangyifan
    // enforece reset video in to axis-stream IPcore in init step
	AXI_GPIO_RST_VID_HANDLE[RST_VIDEO_INTO_AXI4S_OFFSET>>2] = 0xf;//4bit 1111
	AXI_GPIO_RST_VID_HANDLE[RST_IR_VIDEO_INTO_AXI4S_OFFSET>>2] = 0xf;//4bit 1111
	wait_moment();					//important when reset video into axis stream IP core, must wait a moment
	AXI_GPIO_RST_VID_HANDLE[RST_VIDEO_INTO_AXI4S_OFFSET>>2] = 0;asm("nop");
	AXI_GPIO_RST_VID_HANDLE[RST_IR_VIDEO_INTO_AXI4S_OFFSET>>2] = 0;asm("nop");
//	printf("init rst vid\n");
#endif

#ifdef DETECT_VIDEO_INPUT	// 2023.01.31 added by fengyachun to detect video input & switch video output clk src
	//2023.01.30

	switch_clk.config.para.reserved 	= 0;
    switch_clk.config.para.RK3399_I2C_SDA = 0;
	switch_clk.config.para.reserved_bit = 0;
	switch_clk.config.para.rst_gs2972_mannul = 1;
	switch_clk.config.para.video_clk_switch = EXTERN_REF_CLK;
	AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[SWITCH_VIDEO_CLK_SRC>>2] = switch_clk.config.union_body;
	asm("nop");
    //TV
    static bool video_not_setup		= true;
    static bool VTC_IN_ASYNC_MODE	= false;
    //IR
    static bool video_not_setup_ir = true;
    static bool video_setup_ir  = false;


    pre_self_check_status = 0;//20230208

#endif

#define DEBUG_PRINTF

#ifdef SUM_DELAY
    cv::Mat img_for_delay(cv::Size(640,640),CV_8UC1);
    static int sum_for_delay = 0;
#endif

#ifdef ADD_dot_line

    memset(pt_arr, 0, 1024*sizeof(unsigned short));
	start_rows = 540; //img_rows_start
	start_cols = 1920; //img_cols_start
	end_rows = 540;//img_rows_end 270+540
	end_cols = 960; //img_cols_end 480+960
	int width = 1;
	std::chrono::system_clock::time_point start_draw;
	std::chrono::system_clock::time_point end_draw;
	init_dot_line_bram_buf();
//#define EXAMPLE_CODE_DRAW
//#define EXAMPLE_CODE_CLEAR_DRAW
#ifdef EXAMPLE_CODE_DRAW
	memset(pt_arr, 0, 1024*2);
	gen_line(start_rows/2, start_cols/2, end_rows/2, end_cols/2, width, pt_arr);
	update_dot_arr(AXI_BRAM_CTRL_DOTLINE_00_HANDLE, 0,pt_arr);//00
	update_dot_arr(AXI_BRAM_CTRL_DOTLINE_01_HANDLE, 1,pt_arr);//01
	update_dot_arr(AXI_BRAM_CTRL_DOTLINE_10_HANDLE, 2,pt_arr);//10
	update_dot_arr(AXI_BRAM_CTRL_DOTLINE_11_HANDLE, 3,pt_arr);//11
#endif

#ifdef EXAMPLE_CODE_CLEAR_DRAW
	memset(pt_arr, 0, 1024*2);
	init_dot_line_bram_buf();
#endif


#endif
#ifdef ADD_dot_line_mixer
	memset(pt_arr_pre, 0, 1024*2);
	memset(pt_arr_cur, 0, 1024*2);

	//std::chrono::system_clock::time_point start_draw;
	//std::chrono::system_clock::time_point end_draw;
	//example...
	//int start_rows = 540; //img_rows_start
	//int start_cols = 1920; //img_cols_start
	//int end_rows = 540;//img_rows_end 270+540
	//int end_cols = 960; //img_cols_end 480+960

	//draw_dot_line(start_rows, start_cols, end_rows, end_cols,targraphColor.ColorConfig.color.Color_R_Y,targraphColor.ColorConfig.color.Color_G_U,targraphColor.ColorConfig.color.Color_B_V);

	//clear_dot_line();

#endif

// #define open_main_loop_time_monitor
    bool OPEN_CLC = false;
    bool CLOSE_CLC = false;
    while (true)
    {  
#ifdef open_main_loop_time_monitor
        start = std::chrono::system_clock::now();
#endif
        #ifdef UART_PRINT_EVERY
            if(cmd_data[255] == 0xfd)
            {
                int cmd_length = cmd_data[2];
                memcpy(cmd_update_data, cmd_data, cmd_length);
                cmd_data[255] = 0x00;
                //0x11 0x19 0x21 0x23 0x25 0x1b
			if( (cmd_update_data[3] != 0x21) &&
					(cmd_update_data[3] != 0x23) &&
					(cmd_update_data[3] != 0x25) &&
					(cmd_update_data[3] != 0x26) &&
					(cmd_update_data[3] != 0x27))
                {
                    printf("cmd pld=%d: ",cmd_length);
                    for(int i = 0; i<cmd_length; i++)
                    {
                        printf("%02x ",cmd_update_data[i]);
                    }
                    printf("\n ");
                }
            }
        #endif
#ifndef DEBUG_KEYBOARD
		if(kbhit())
		{
			ch_cur = getch(); /* consume the character */
			//if(ch_cur==-1)
			if(ch_cur!=ch_pre)
			{
				key_enter_num++;
			}
			if(ch_cur==-1)
			{
				key_EOF_num++;
			}
//			printf("get char 0x%x\n", ch);
			if(ch_cur==27)
			{
                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF2, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                std::this_thread::sleep_for(std::chrono::milliseconds(4));
//				break_proc_q = true;
				exit(0);
			}
			ch_pre = ch_cur;
		}
		if(OPEN_CLC==true)
		{
			printf("open CLC\n");
			OPEN_CLC = false;
		}
		if(CLOSE_CLC==true)
		{
			printf("close CLC\n");
			CLOSE_CLC = false;
		}


//		else
//		{
//			key_wait_num++;
//		}
#endif
        if(new_mipi_stream_setup==true && mipi_out_cnt > 60)
        {
        	//xj3_mipi_rst_time_cur = omp_get_wtime();
            XJ3UARTDevice::instance()->write_command_control(0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
            new_mipi_stream_setup = false;
            watchdog_mipi_count = 0;
            count_mipi_ok = 0;
            count_ch_switch = -10;
            printf("manually reset XJ3 mipi_cam.\n");
        }

        if ((Uart_XJ3_RX_HANDLE[0x08>>2] & 0x20) == 0x20)  // check status of RX FIFO full
        {
            Uart_XJ3_RX_HANDLE[0x0c>>2] |= 0x13;           // reset RX FIFO
        }    

        if ((Uart_HOST_RX_HANDLE[0x08>>2] & 0x20) == 0x20)  // check status of RX FIFO full
        {
            Uart_HOST_RX_HANDLE[0x0c>>2] |= 0x13;           // reset RX FIFO
        }

//        if (break_proc_q )//&& break_proc_w && break_proc_e && break_proc_r && break_proc_t
//        {
//        	printf("exit ps app using key_board mode..\n");
//            return 0;
//        }
        // 2023.07.14 added by zhangzeyu to detect gpio130 from rk3399 to detect take_photo status and 
        // if(video_status.config.para.status_send_photo == 1)
        // {
        //     if(send_photo_status == 0)
        //     {
        //         uint8_t params[1] = {HostUARTDevice::instance()->photo_stat};
        //         HostUARTDevice::instance()->write_command_new(0x52, SIZEOF(params), params, HostUARTDevice::instance()->write_data, &HostUARTDevice::instance()->write_data_length);
        //     }
        // }
        // else
        if(video_status.config.para.status_send_photo == 0)
        {
            if(send_photo_status == 1)
            {
                uint8_t params[1] = {0x03};
                HostUARTDevice::instance()->write_command_new(0x52, SIZEOF(params), params, HostUARTDevice::instance()->write_data, &HostUARTDevice::instance()->write_data_length);
            }
        }
        send_photo_status = video_status.config.para.status_send_photo;

#ifdef DETECT_VIDEO_INPUT			// 2023.01.31 added by fengyachun to detect video input & switch video output clk src
        // status_IR_pre = video_status.config.para.status_IR;
        // status_TV_pre = video_status.config.para.status_TV;
//        pre_self_check_status = self_check.info_body.union_body;
        video_status.config.union_body = AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[DETECT_VIDEO_INPUT_STATUS_OFFSET>>2];
        //add self_check and report zzy 2023.02.07
        // if( ((status_IR_pre != video_status.config.para.status_IR) && HostUARTDevice::instance()->ir_cool_finish ) || ( status_TV_pre != video_status.config.para.status_TV) )
        // {
        self_check_status = 1; 
        if(video_status.config.para.status_TV == 1)  
        {
            self_check.info_body.info_detail.breakdown_TV_video = 0;
        }
        else
        {
            self_check_status = 2; 
            self_check.info_body.info_detail.breakdown_TV_video = 1;
        }

        if(!HostUARTDevice::instance()->ir_cool_finish)
        {
            self_check.info_body.info_detail.breakdown_IR_video = 0;
        }
        else
        {
            if(video_status.config.para.status_IR == 1)
                self_check.info_body.info_detail.breakdown_IR_video = 0;
            else
            { 
                self_check_status = 2; 
                self_check.info_body.info_detail.breakdown_IR_video = 1;
            }
        }
        if(intr_cnt_uart_host > 1500)
        {
            if(pre_self_check_status != self_check.info_body.union_body)
            {
                if(self_check_status == 1)
                {
                    count_self_check_error = -100;
                    pre_self_check_status = self_check.info_body.union_body;
                    uint8_t params[2] = {self_check_status, self_check.info_body.union_body};
                    HostUARTDevice::instance()->write_command(0x03, SIZEOF(params), params);
#ifdef DEBUG_PRINTF
                    printf("AVT reprot self_check status %d IR_POWER: %d TV: %d IR: %d S: %d\n",self_check_status, HostUARTDevice::instance()->ir_cool_finish, self_check.info_body.info_detail.breakdown_TV_video, self_check.info_body.info_detail.breakdown_IR_video,self_check.info_body.info_detail.breakdown_Reserved);
#endif
                }
                else
                {
                    pre_self_check_status = self_check.info_body.union_body;
                    count_self_check_error = count;
                }
            }
        }
        if(count == (count_self_check_error + 10))
        {
            count_self_check_error = -100;
            uint8_t params[2] = {self_check_status, self_check.info_body.union_body};
            HostUARTDevice::instance()->write_command(0x03, SIZEOF(params), params);
#ifdef DEBUG_PRINTF
            printf("AVT reprot self_check status %d IR_POWER: %d TV: %d IR: %d S: %d\n",self_check_status, HostUARTDevice::instance()->ir_cool_finish, self_check.info_body.info_detail.breakdown_TV_video, self_check.info_body.info_detail.breakdown_IR_video,self_check.info_body.info_detail.breakdown_Reserved);
#endif            
        }

        //----- video_status.config.para.status_IR
        //----- video_status.config.para.status_TV

		if(video_status.config.para.status_TV == 0)
		{
			video_not_setup = true;
			switch_clk.config.para.video_clk_switch = AVT_GEN_CLK;
			AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[SWITCH_VIDEO_CLK_SRC>>2] = switch_clk.config.union_body;asm("nop");
			if(VTC_IN_ASYNC_MODE == false)
			{
    			wait_moment();
                wait_moment();
                wait_moment();
                wait_moment();
    			wait_moment();					//important when switch video clk src must wait a moment to reset VTC
				reset_VTC_IN_ASYNC_MODE();
				VTC_IN_ASYNC_MODE = true;
#ifdef DEBUG_PRINTF
				printf("TV video input break & reset vtc async mode\n");
#endif
			}
		}
		else
		{
			if(video_not_setup == true)
			{
				switch_clk.config.para.video_clk_switch = EXTERN_REF_CLK;
				AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE[SWITCH_VIDEO_CLK_SRC>>2] = switch_clk.config.union_body;asm("nop");
				wait_moment();					//important when switch video clk src must wait a moment to reset VTC
				wait_moment();
				#ifdef RST_VID_IN_PS_PROC
					//2023.01.30  to handle randomly happened gray screen--------------fengyachun & yangyifan
					AXI_GPIO_RST_VID_HANDLE[RST_VIDEO_INTO_AXI4S_OFFSET>>2] = 0xf;asm("nop");//4bit 1111
					wait_moment();				//important when reset video into axis stream IP core, must wait a moment
					AXI_GPIO_RST_VID_HANDLE[RST_VIDEO_INTO_AXI4S_OFFSET>>2] = 0;asm("nop");
				#endif
				wait_moment();					//important when switch video clk src must wait a moment to reset VTC
				wait_moment();
    			wait_moment();
				reset_VTC_IN_SYNC_MODE();
				video_not_setup = false;
				VTC_IN_ASYNC_MODE = false;
#ifdef DEBUG_PRINTF
//				TV_RECOVER_CNT++;
				printf("TV video input recover & reset vtc sync mode\n");
#endif
			}
		}

#ifdef IR_CLOSE_VDMA_S2MM_20240125
		if(ir_s2mm_stat != cur_ir_s2mm_stat)
		{
			ir_s2mm_stat = cur_ir_s2mm_stat;
			if(ir_s2mm_stat)
				printf("Enable IR S2MM\n");
			else
				printf("Disable IR S2MM\n");
		}
#endif
        if(video_status.config.para.status_IR == 0)
        {
            video_not_setup_ir = true;
            if(video_setup_ir == false)
            {
                video_setup_ir = true;
#ifdef DEBUG_PRINTF
              //  printf("IR video input break. Current IR_Power %d \n",HostUARTDevice::instance()->ir_cool_finish);
#ifdef IR_CLOSE_VDMA_S2MM_20240125
    			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
    			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
                ir_power_on_stat = false;
                cur_ir_s2mm_stat = false;
                ir_power_on_cnt = 0;
                printf("Detect IR break, Close IR S2MM. Current IR_Power %d \n",HostUARTDevice::instance()->ir_cool_finish);
#else
                printf("Detect IR break, Current IR_Power %d \n",HostUARTDevice::instance()->ir_cool_finish);
#endif
#endif
            }
        }
        else
        {
            if(video_not_setup_ir == true)
            {
                #ifdef RST_VID_IN_PS_PROC
					//2024.02.24  to handle randomly happened ir black screen--------------hanyang
					AXI_GPIO_RST_VID_HANDLE[RST_IR_VIDEO_INTO_AXI4S_OFFSET>>2] = 0xf;asm("nop");//4bit 1111
					wait_moment();				//important when reset video into axis stream IP core, must wait a moment
					AXI_GPIO_RST_VID_HANDLE[RST_IR_VIDEO_INTO_AXI4S_OFFSET>>2] = 0;asm("nop");
				#endif
				wait_moment();
				wait_moment();
				video_setup_ir = false;
                video_not_setup_ir = false;
#ifdef DEBUG_PRINTF
//                IR_RECOVER_CNT++;
             //   printf("IR video input recover. Current IR_Power %d \n",HostUARTDevice::instance()->ir_cool_finish);
#ifdef IR_CLOSE_VDMA_S2MM_20240125
                VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
    			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
                ir_power_on_stat = true;
    			cur_ir_s2mm_stat = false;
    			ir_power_on_cnt = 0;
    			printf("Detect IR input, Close IR S2MM and start counting\n");
#else
    			printf("IR video input recover. Current IR_Power %d \n",HostUARTDevice::instance()->ir_cool_finish);
#endif
#endif
            }
        }
#endif
	
#ifdef XADC		// 2023.01.31 added by fengyachun to report the temperature of FMQL-7045
		if(requst_temper)
		{
			int temp = XADC_HANDLE[0x200>>2];
			float temp_lsb= ((((float)(temp)/65536.0f)/0.00198421639f ) - 273.15f);//XADC_obj.data.sub_data.lsb_temp;
			printf("chip temp: %f\n",temp_lsb);
			requst_temper = false;
		}
		if(HostUARTDevice::instance()->request_temper)
		{
			int temp = XADC_HANDLE[0x200>>2];
			int temp_lsb= (int)((((float)(temp)/65536.0f)/0.00198421639f ) - 273.15f);//XADC_obj.data.sub_data.lsb_temp;
			uint8_t params[1];
			if(temp_lsb >=0)
			{
				//params[0] = temp_lsb & 0xff;
				temp_byte_reprot.config.para.pos_neg = 0;//20230206 fengyachun add
				temp_byte_reprot.config.para.temp_abs = (temp_lsb & 0x7f);//20230206 fengyachun add
			}
			else
			{
				//params[0] = ((temp_lsb & 0xff) | 0x80);
				temp_byte_reprot.config.para.pos_neg = 1;//20230206 fengyachun add
				temp_byte_reprot.config.para.temp_abs = (temp_lsb & 0x7f);//20230923 zzy modify - temp_lsb;//20230206 fengyachun add
			}

			params[0] = temp_byte_reprot.config.union_body;//20230206 fengyachun add
			//printf("report temper: %X\n",params[0]);
			HostUARTDevice::instance()->write_command(0x50, SIZEOF(params), params);
			HostUARTDevice::instance()->request_temper = false;

		}
#endif		
		       
        #ifdef OS_UNIX  
            // is_tv = (HostUARTDevice::instance()->channel == 1);
            // uint8_t disp_mode = (HostUARTDevice::instance()->picture_in_picture == 0x02) ? (is_tv ? TV_MAIN_MODE : IR_MAIN_MODE) : (is_tv ? TV_PIP_MODE : IR_PIP_MODE);
            // disp_mode = (HostUARTDevice::instance()->fusion == 0x01) ? FUSION_MODE : disp_mode;
            // if ((disp_ctrl.DISP_MODE != disp_mode))
            // {
            //     disp_ctrl.SWITCH_HAPPENED = true;
            //     //disp_ctrl.DISP_MODE = (HostUARTDevice::instance()->picture_in_picture == 0x02) ? (is_tv ? TV_MAIN_MODE : IR_MAIN_MODE) : (is_tv ? TV_PIP_MODE : IR_PIP_MODE);
            //     disp_ctrl.DISP_MODE = disp_mode;
            // }          
            if((count == 900) && cross_main_init)
            {
                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, cross_x_main, cross_y_main, 0x02);                  
            }   
            if((count == 901) && cross_main_init)
            {
                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, (cross_x_main-320)/2, (cross_y_main-28)/2, 0x03);
                cross_main_init = false;                  
            } 
            // if((count == 3600) && (init_altitude) == -1) 
            // {
            //     init_altitude = (uint16_t)(HostUARTDevice::instance()->aircraft_altitude / 50);
            //     XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, aircraft_altitude, 0xFFFF, 0x09);
            // }
            // if((HostUARTDevice::instance()->aircraft_altitude / 50 - aircraft_altitude) > 1000)
            // {
            //     aircraft_altitude = HostUARTDevice::instance()->aircraft_altitude / 50;
            //     XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, aircraft_altitude, 0xFFFF, 0x09);
            // }
            
// ********************start: targets tracking and detection **********************************//
            bool status_changed = ((HostUARTDevice::instance()->track_command_changed && HostUARTDevice::instance()->track_command == 1) || 
            (HostUARTDevice::instance()->multi_target_prompt_send && HostUARTDevice::instance()->multi_target_prompt == 2) || 
            HostUARTDevice::instance()->second_capture_changed || HostUARTDevice::instance()->track_ID_send);

            if(wave_gate_visible != HostUARTDevice::instance()->wave_gate_visible)
            {                          
                wave_gate_visible = HostUARTDevice::instance()->wave_gate_visible;
            }

            if(HostUARTDevice::instance()->start_new_track)  
            {
                HostUARTDevice::instance()->start_new_track = false;
                XJ3UARTDevice::instance()->write_command_control(0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                uint8_t params[1] = {0x02};
                HostUARTDevice::instance()->write_command(0x07, SIZEOF(params), params);               
                xj3_send_command = true;       
            }  
            if(HostUARTDevice::instance()->wave_gate_size_changed)
            { 
                if(track_command == 1)
                {
                    timer2 = 30;
                } 
                HostUARTDevice::instance()->wave_gate_size_changed = false; 
                if(is_tv)
                {                        
                    if( (HostUARTDevice::instance()->wave_gate_width_scale <= 0.500) && (tv_wave_gate_size != 0) ) 
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x04); 
                        tv_wave_gate_size = 0;
                    }
                    else if( (HostUARTDevice::instance()->wave_gate_width_scale <= 0.750) && (HostUARTDevice::instance()->wave_gate_width_scale > 0.500) && (tv_wave_gate_size != 1))
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x05);
                        tv_wave_gate_size = 1;
                    }
                    else if( (HostUARTDevice::instance()->wave_gate_width_scale <= 1.000) && (HostUARTDevice::instance()->wave_gate_width_scale > 0.750) && (tv_wave_gate_size != 2))
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x06);
                        tv_wave_gate_size = 2;
                    } 
                    else if(HostUARTDevice::instance()->wave_gate_width_scale > 1.000 && (tv_wave_gate_size != 3))
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x07);
                        tv_wave_gate_size = 3;                                
                    }
                }
                else
                {
                    if((HostUARTDevice::instance()->wave_gate_width_scale <= 0.750) && (ir_wave_gate_size != 0))
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x08); 
                        ir_wave_gate_size = 0;                        
                    }
                    else if( (HostUARTDevice::instance()->wave_gate_width_scale <= 1.000) && (HostUARTDevice::instance()->wave_gate_width_scale > 0.750) && (ir_wave_gate_size != 1) ) 
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x04); 
                        ir_wave_gate_size = 1;
                    }
                    else if( (HostUARTDevice::instance()->wave_gate_width_scale <= 1.250) && (HostUARTDevice::instance()->wave_gate_width_scale > 1.000) && (ir_wave_gate_size != 2))
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x05);
                        ir_wave_gate_size = 2;
                    }
                    else if((HostUARTDevice::instance()->wave_gate_width_scale <= 1.500) && (HostUARTDevice::instance()->wave_gate_width_scale > 1.250) && (ir_wave_gate_size != 3))
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x06);
                        ir_wave_gate_size = 3;
                    } 
                    else if(HostUARTDevice::instance()->wave_gate_width_scale > 1.50 && (ir_wave_gate_size != 4))
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0x07);
                        ir_wave_gate_size = 4;
                    } 
                                           
                }
            } 

            if(HostUARTDevice::instance()->wave_gate_position_changed) 
            {
                HostUARTDevice::instance()->wave_gate_position_changed = false;
                // if(HostUARTDevice::instance()->wave_gate_position_x == 1)
                // {
                //     printf("wave_gate_position_x +1.\n");
                // }
                // else if(HostUARTDevice::instance()->wave_gate_position_x == 2)
                // {
                //     printf("wave_gate_position_x -1.\n");
                // }

                // if(HostUARTDevice::instance()->wave_gate_position_y == 1)
                // {
                //     printf("wave_gate_position_y +1.\n");
                // }
                // else if(HostUARTDevice::instance()->wave_gate_position_y == 2)
                // {
                //     printf("wave_gate_position_y -1.\n");
                // }
                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, (HostUARTDevice::instance()->wave_gate_position_y << 4) | HostUARTDevice::instance()->wave_gate_position_x, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
            }

            static std::vector<TarGraphInfo *> targets_{&TarInfo_1, &TarInfo_2, &TarInfo_3, &TarInfo_4, &TarInfo_5, &TarInfo_6, &TarInfo_7, &TarInfo_8};
            static TarGraphInfo *second_target{&TarInfo_10};
            static std::vector<std::pair<int, int> > targets;
            static std::vector<std::pair<int, int> > targets_sz;
            static int target_count = 0;
            static std::vector<int> targets_ID_mapping(5);
//            static std::queue<int> vacancy_index;
            static bool default_targets_saved = false;
            if (!default_targets_saved)
            {
                for (int i = 0; i < 8; ++i)
                {
                    targets.push_back(std::make_pair((int) targets_[i]->cross.info.cols_hor_x, (int) targets_[i]->cross.info.rows_ver_y));
                    targets_sz.push_back(std::make_pair((int) targets_[i]->gate.info.width, (int) targets_[i]->gate.info.height));
                }
                default_targets_saved = true;
            }

            if(XJ3UARTDevice::instance()->data_changed || status_changed)
            {   
                if(XJ3UARTDevice::instance()->data_changed)
                {
                    watchdog_mipi_count = 0;
                    XJ3UARTDevice::instance()->data_changed = false;
                }
                if(status_changed)
                {
                    status_changed = false;  
                }

                if(HostUARTDevice::instance()->track_command_changed)
                {
                    track_command = HostUARTDevice::instance()->track_command;
                    HostUARTDevice::instance()->track_command_changed = false;  
                    printf("track_command_changed:%d.\n", track_command);
                    //XJ3UARTDevice::instance()->write_command_control((0x0F << 4) | track_command, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);

                    if(track_command == 1)
                    {
                        if(multi_target_prompt != 2)
                            count_prompt_start = count;
                        // HostUARTDevice::instance()->wave_gate_width_scale = 1.0; //when track_command change from track to capture, reinit wage_gate_scale = 1.0
                        // HostUARTDevice::instance()->wave_gate_size = 2.0;
                        TarInfo_9.gate.info.Enable  = false;
                        TarInfo_9.gate.info.IdEn    = false;
                        TarInfo_9.cross.info.Enable = false;
                        if(is_tv) 
                        {
                            if(HostUARTDevice::instance()->recv_multi_prompt_tracked)
                            {
                                HostUARTDevice::instance()->recv_multi_prompt_tracked = false;
                                XJ3UARTDevice::instance()->write_command_control(0xF1, 0xF1, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                            else
                            {
                                XJ3UARTDevice::instance()->write_command_control(0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                            xj3_send_command = true;
                        }
                        else
                        {
                            if(HostUARTDevice::instance()->recv_multi_prompt_tracked)
                            {
                                HostUARTDevice::instance()->recv_multi_prompt_tracked = false;
                                XJ3UARTDevice::instance()->write_command_control(0xF1, 0xF1, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                            else
                            {
                                XJ3UARTDevice::instance()->write_command_control(0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);                                
                            }  
                            xj3_send_command = true;                        
                        }
                        // tv_wave_gate_size = 2;
                        // ir_wave_gate_size = 1;
                        slide_status = 0;
                    }
                    else if((track_command == 3) || (track_command == 4))
                    {
                        servo_start = true;
                        count_track_start = count;
                        reached_x_servo = 1;
                        reached_y_servo = 1;
                        if(track_command == 4)
                        {
                            if(cross_symbol_x > 0)
                                targets_[6]->cross.info.cols_hor_x = cross_symbol_x;
                            if(cross_symbol_y > 0)
                                targets_[6]->cross.info.rows_ver_y = cross_symbol_y;
                        }
                    } 
                    else
                    {
                        count_track_start = count;
                    }
                }
                if( track_status != XJ3UARTDevice::instance()->track_status)
                {
                    track_status = XJ3UARTDevice::instance()->track_status;
                    if(track_status > 1 && track_status < 5)
                    {
                        if(track_command == 2) 
                        {
                            if(track_status == 4)
                                printf("report track status lost.\n");
                            uint8_t params[1] = {(uint8_t) (track_status - 1)};
                            HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                        }
                        else if((track_command == 3) || (track_command == 4))
                        {
                            if(track_status == 3)
                            {
                                slide_status = 1;
                            }
                            else if((track_status == 2) && (slide_status == 1))
                            {
                                slide_status = 0;                              
                            }
                            else
                            {
                                if(track_status == 4)
                                    printf("report track status lost.\n");
                                uint8_t params[1] = {(uint8_t) (track_status - 1)};
                                HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params); 
                            }
                        }

                        if(!XJ3UARTDevice::instance()->channel) //is_tv
                        { 
                            report_x = XJ3UARTDevice::instance()->targets[0].first;
                            report_y = XJ3UARTDevice::instance()->targets[0].second;
                        }
                        else
                        {
                            report_x = 2*XJ3UARTDevice::instance()->targets[0].first + 3200;
                            report_y = 2*XJ3UARTDevice::instance()->targets[0].second + 280;                       
                        }
                    }
                } 
                if(HostUARTDevice::instance()->multi_target_prompt_send)
                // if(multi_target_prompt != HostUARTDevice::instance()->multi_target_prompt)
                {
                    HostUARTDevice::instance()->multi_target_prompt_send = false;
                    multi_target_prompt = HostUARTDevice::instance()->multi_target_prompt;
                    // XJ3UARTDevice::instance()->write_command_control(0xFF, (0xF0 | multi_target_prompt), 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                    uint8_t params[1] = {multi_target_prompt};
                    HostUARTDevice::instance()->write_command(0x0A, SIZEOF(params), params);
                } 
                if(HostUARTDevice::instance()->second_capture_changed)
                {
                    second_capture = HostUARTDevice::instance()->second_capture;
                    HostUARTDevice::instance()->second_capture_changed = false;
                    // if(second_capture == 2)
                    // {
                    uint8_t params[1] = {second_capture};
                    HostUARTDevice::instance()->write_command(0x14, SIZEOF(params), params);
                    // }       
                }
 
                targets = XJ3UARTDevice::instance()->targets;
                targets_sz = XJ3UARTDevice::instance()->targets_sz; 
                {
                    switch( track_command )
                    {
                        case 1: //capture
                        {
                            XJ3UARTDevice::instance()->track_status = 1;
                            reached_x = 2;
                            reached_y = 2;
                            if (targets.empty())
                            {
                                for (int i = 0; i < 8; ++i)
                                {
                                    targets_[i]->gate.info.Enable  = false;
                                    targets_[i]->gate.info.IdEn    = false; 
                                    targets_[i]->cross.info.Enable = false; 
                                }
                            }
                            else
                            {
                                if(!is_tv) //XJ3UARTDevice::instance()->channel
                                {
                                    for (int i = 0; i < targets.size(); ++i)
                                    {
                                        targets[i].first  = 2*targets[i].first + 320;
                                        targets[i].second = 2*targets[i].second + 28;
                                        targets_sz[i].first = 2*targets_sz[i].first;
                                        targets_sz[i].second = 2*targets_sz[i].second;
                                    }
                                }
                                else
                                {
                                    switch(tv_scale)
                                    {
                                        case 1:
                                        {
                                            for (int i = 0; i < targets.size(); ++i)
                                            {
                                                targets[i].first  = std::round(2*targets[i].first  - cross_x_main);
                                                targets[i].second = std::round(2*targets[i].second - cross_y_main);
                                                targets_sz[i].first = 2*targets_sz[i].first;
                                                targets_sz[i].second = 2*targets_sz[i].second;
                                            }
                                        }
                                            break;
                                        // case 3:
                                        // {
                                        //     for (int i = 0; i < targets.size(); ++i)
                                        //     {   
                                        //         targets[i].first = std::round(4*targets[i].first  - 3*cross_x_main);
                                        //         targets[i].second = std::round(4*targets[i].second - 3*cross_y_main);
                                        //         targets_sz[i].first = 4*targets_sz[i].first;
                                        //         targets_sz[i].second = 4*targets_sz[i].second;
                                        //     }
                                        // }   
                                            break;
                                        default:
                                            break;
                                    } 
                                } 
                                switch(multi_target_prompt)
                                {
                                    case 1:
                                    {
                                        target_count = 0;
                                        targets_ID_mapping.clear(); 
                                        if(count < (count_prompt_start + 5))
                                        {
                                            for (int i = 0; i < 8; ++i)
                                            {
                                                targets_[i]->gate.info.Enable  = false;
                                                targets_[i]->gate.info.IdEn    = false;
                                                targets_[i]->cross.info.Enable = false;
                                            }  
                                        }
                                        else if(mipi_out_cnt < (count_ch_switch+8))
                                        {
                                            for (int i = 0; i < 8; ++i)
                                            {
                                                targets_[i]->gate.info.Enable  = false;
                                                targets_[i]->gate.info.IdEn    = false;
                                                targets_[i]->cross.info.Enable = false;
                                            }                                             
                                        }
                                        else
                                        {
                                            for (int i = 0; i < 8; ++i)
                                            {
                                                if(i < 5)
                                                {
                                                    targets_[i]->gate.info.Enable  = false;
                                                    targets_[i]->gate.info.IdEn    = false;
                                                    targets_[i]->cross.info.Enable = false; 
                                                }
                                                if(XJ3UARTDevice::instance()->targets[i].first <= 0 || XJ3UARTDevice::instance()->targets[i].first >= 1920 || XJ3UARTDevice::instance()->targets[i].second <= 0 || XJ3UARTDevice::instance()->targets[i].second >= 1080)
                                                    continue;
                                                else if(target_count == 5)
                                                    break;
                                                else
                                                {
                                                    if(is_tv) //!XJ3UARTDevice::instance()->channel
                                                    {
                                                        visible = (XJ3UARTDevice::instance()->channel == 0);
                                                        if((targets[i].first - targets_sz[i].first/2) < 16)
                                                        {
                                                            if(targets[i].first < 24)
                                                                targets[i].first = 24;
                                                            targets_sz[i].first = 2 * (targets[i].first - 16);
                                                        }
                                                        else if((targets[i].first + targets_sz[i].first/2) > 1919)
                                                        {
                                                            if(targets[i].first > 1911)
                                                                targets[i].first = 1911;
                                                            targets_sz[i].first  = 2 * (1919 - targets[i].first);
                                                        }
                                                        if((targets[i].second - targets_sz[i].second/2) < 32)
                                                        {
                                                            if(targets[i].second < 40)
                                                                targets[i].second = 40;
                                                            targets_sz[i].second = 2 * (targets[i].second - 32);
                                                        }
                                                        else if((targets[i].second + targets_sz[i].second/2) > 1079)  
                                                        {
                                                            if(targets[i].second > 1071)
                                                                targets[i].second = 1071;
                                                            targets_sz[i].second = 2 * (1079 - targets[i].second);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        visible = (XJ3UARTDevice::instance()->channel == 1) && ((targets[i].first - targets_sz[i].first/2) > 320) && ((targets[i].first + targets_sz[i].first/2) < 1600) && ((targets[i].second - targets_sz[i].second/2) > 28) && ((targets[i].second + targets_sz[i].second/2) < 1052);
                                                        if((targets[i].first - targets_sz[i].first/2) < 320)
                                                            targets_sz[i].first  = 2 * (targets[i].first - 320);
                                                        else if((targets[i].first + targets_sz[i].first/2) > 1600)
                                                            targets_sz[i].first  = 2 * (1600 - targets[i].first);
                                                        if((targets[i].second - targets_sz[i].second/2) < 32)
                                                        {
                                                            if(targets[i].second < 40)
                                                                targets[i].second = 40;
                                                            targets_sz[i].second = 2 * (targets[i].second - 32);
                                                        }
                                                        else if((targets[i].second + targets_sz[i].second/2) > 1052)
                                                            targets_sz[i].second = 2 * (1052 - targets[i].second);
                                                    }
                                                    if(i < 5)
                                                    {                                               
                                                        targets_[i]->gate.info.Enable = visible ? wave_gate_visible : false;
                                                        targets_[i]->gate.info.IdEn   = visible ? wave_gate_visible : false;
                                                        targets_[i]->gate.info.width  = std::max(16, targets_sz[i].first);
                                                        targets_[i]->gate.info.height = std::max(16, targets_sz[i].second);
                                                        targets_[i]->cross.info.cols_hor_x = targets[i].first;
                                                        targets_[i]->cross.info.rows_ver_y = targets[i].second;
                                                        targets_[i]->cross.info.Enable = false;
                                                        targets_[i]->tar_idx = i+1;
                                                        targets_ID_mapping[i] = i+1;
                                                    }
                                                    // else
                                                    // {
                                                    //     for(int j = 4; j >= 0; j--)
                                                    //     {
                                                    //         if(!targets_[j]->gate.info.Enable)
                                                    //         {
                                                    //             targets_[j]->gate.info.Enable = visible ? wave_gate_visible : false;
                                                    //             targets_[j]->gate.info.IdEn   = visible ? wave_gate_visible : false;
                                                    //             targets_[j]->gate.info.width  = std::max(16, targets_sz[i].first);
                                                    //             targets_[j]->gate.info.height = std::max(16, targets_sz[i].second);
                                                    //             targets_[j]->cross.info.cols_hor_x = targets[i].first;
                                                    //             targets_[j]->cross.info.rows_ver_y = targets[i].second;
                                                    //             targets_[j]->cross.info.Enable = false;
                                                    //             targets_[j]->tar_idx  = i+1;
                                                    //             targets_ID_mapping[j] = i+1; 
                                                    //             break;                                                               
                                                    //         }
                                                    //     }
                                                    // }
                                                    target_count++;
                                                }
                                            }
                                            uint8_t target_order = HostUARTDevice::instance()->aux_ID;
                                            uint8_t main_target_ID = 0;
                                            bool find_main_target = false;
                                            if(target_count > 0)
                                            {
                                                for(int i = 0; i < 5; i++)
                                                {
                                                    if(find_main_target)
                                                    {
                                                        targets_[target_order]->cross.info.line_width = 1;
                                                    }
                                                    else
                                                    {
                                                        if(targets_[target_order]->gate.info.Enable) 
                                                        {
                                                            main_target_ID = target_order;
                                                            // HostUARTDevice::instance()->aux_ID = main_target_ID;
                                                            targets_[target_order]->cross.info.line_width = 3;
                                                            find_main_target = true;
                                                        }
                                                        else
                                                            targets_[target_order]->cross.info.line_width = 1;
                                                    }
                                                    if(HostUARTDevice::instance()->aux_ID_plus)
                                                    {
                                                        if(target_order == 4)
                                                            target_order = 0;
                                                        else 
                                                            target_order++;
                                                    }
                                                    else
                                                    {
                                                        if(target_order == 0)
                                                            target_order = 4;
                                                        else 
                                                            target_order--;
                                                    }
                                                    target_order = (target_order >= 0) ? ((target_order <= 4) ? target_order : 4) : 0; 
                                                }
                                            }

                                            if(HostUARTDevice::instance()->track_ID_send)
                                            {
                                                HostUARTDevice::instance()->track_ID_send = false;
                                                if(HostUARTDevice::instance()->track_command == 3)
                                                    track_ID = main_target_ID;
                                                else
                                                    track_ID = HostUARTDevice::instance()->track_ID - 1;
                                                track_ID = (track_ID >= 0) ? ((track_ID <= 4) ? track_ID : 4) : 0;
                                                
                                                uint8_t params[1] = {0x02};
                                                if(!targets_[track_ID]->gate.info.Enable || (target_count == 0))
                                                {
                                                    printf("no track_ID:%d, track_command = 2.\n", track_ID);
                                                    HostUARTDevice::instance()->track_command = 2;
                                                    XJ3UARTDevice::instance()->write_command_control(0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                                                    // params[0] = {0x01};
                                                    HostUARTDevice::instance()->write_command(0x07, SIZEOF(params), params);
                                                }
                                                else
                                                {
                                                    printf("send XJ3 track_ID:%d.\n", targets_ID_mapping[track_ID]);
                                                    XJ3UARTDevice::instance()->write_command_control(0xFF, (targets_ID_mapping[track_ID]<<4 | 0x0F), 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                                                    HostUARTDevice::instance()->track_command = 3;
                                                    // params[0] = {0x02};
                                                    HostUARTDevice::instance()->write_command(0x07, SIZEOF(params), params);
                                                }
                                                xj3_send_command = true;
                                                HostUARTDevice::instance()->track_command_changed = true;
                                            }
                                        }
                                    }
                                        break;
                                    case 2:
                                    {   
                                        if(HostUARTDevice::instance()->track_ID_send)
                                        {
                                            HostUARTDevice::instance()->track_ID_send = false;
                                            uint8_t params[1] = {0x01};
                                            HostUARTDevice::instance()->write_command(0x07, SIZEOF(params), params);
                                        }
                                        HostUARTDevice::instance()->aux_ID = 0;
                                        XJ3UARTDevice::instance()->targets.resize(0);
                                        for (int i = 0; i < 8; ++i)
                                        {
                                            targets_[i]->gate.info.Enable = false;
                                            targets_[i]->gate.info.IdEn   = false;
                                            targets_[i]->cross.info.Enable = false;
                                        }                                                                                
                                    }
                                        break; 
                                    default:
                                    {
                                        for (int i = 0; i < 8; ++i)
                                        {
                                            targets_[i]->gate.info.Enable  = false;
                                            targets_[i]->gate.info.IdEn    = false;
                                            targets_[i]->cross.info.Enable = false;
                                        }                                    
                                    }   
                                }
                            }                                                                              
                        }
                            break;
                        case 2: //normal track
                        {
                            if(HostUARTDevice::instance()->track_ID_send)
                            {
                                HostUARTDevice::instance()->track_ID_send = false;
                                uint8_t params[1] = {0x02};
                                HostUARTDevice::instance()->write_command(0x07, SIZEOF(params), params);
                            }
                            if (targets.empty())
                            {
                                for (int i = 0; i < 8; ++i)
                                {
                                    targets_[i]->gate.info.Enable  = false;
                                    targets_[i]->gate.info.IdEn    = false; 
                                    targets_[i]->cross.info.Enable = false; 
                                }
                            }
                            else
                            {
                                if(XJ3UARTDevice::instance()->channel) //!is_tv
                                { 
                                    // targets[0].first  = 2*targets[0].first  + 3200;// + gate_offset_x;
                                    // targets[0].second = 2*targets[0].second + 280; // + gate_offset_y;
                                    if(cross_x_main%2)
                                        targets[0].first = 2*(targets[0].first+5) + 3200;// + gate_offset_x;
                                    else
                                        targets[0].first = 2*targets[0].first + 3200;// + gate_offset_x;
                                    if(cross_y_main%2)
                                        targets[0].second = 2*(targets[0].second+5) + 280; // + gate_offset_y; 
                                    else
                                        targets[0].second = 2*targets[0].second + 280; // + gate_offset_y;                                        
                                }                           
                                for (int i = 0; i < 8; ++i)
                                {
                                    targets_[i]->gate.info.Enable = false;
                                    targets_[i]->gate.info.IdEn   = false;
                                    targets_[i]->cross.info.Enable = false;
                                }                    
                                switch(track_status)
                                {
                                    // case 0:
                                    //     break;
                                    // case 1:
                                    // {
                                    //     targets_[6]->gate.info.Enable = false;
                                    //     targets_[6]->gate.info.IdEn   = false;
                                    //     targets_[6]->cross.info.Enable = false;
                                    //     if(count == (count_track_start + resend_delay))
                                    //     {
                                    //         printf("resend track command = %d.\n", track_command);
                                    //         XJ3UARTDevice::instance()->write_command_control(0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                                    //         xj3_send_command = true;
                                    //     }
                                    //     else if(count == (count_track_start + resend_delay*2))
                                    //     {
                                    //         uint8_t params[1] = {0x03};
                                    //         HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                    //     }
                                    // }    
                                    //     break;
                                    case 2: //track
                                    {
                                        normal_track_start = 0;
                                        int16_t temp_x, temp_y;
                                        if(!XJ3UARTDevice::instance()->channel) //is_tv
                                        {
                                            switch(tv_scale)
                                            {
                                                case 1:
                                                {
                                                    if((targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10) <= 0) 
                                                    {
                                                        temp_x = 0;
                                                    }
                                                    else if((targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10) >= 19200)
                                                    {
                                                        temp_x = 19200;
                                                    }
                                                    else
                                                    {
                                                        temp_x = targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10;
                                                    }

                                                    if((targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10) <= 0) 
                                                    {
                                                        temp_y = 0;
                                                    }
                                                    else if((targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10) >= 10800)
                                                    {
                                                        temp_y = 10800;
                                                    }
                                                    else
                                                    {
                                                        temp_y = targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10;
                                                    }
                                                    display_x = std::round(temp_x * 0.1); 
                                                    display_y = std::round(temp_y * 0.1);  
                                                    // display_x = std::round(targets[0].first * 0.2 - cross_x_main); 
                                                    // display_y = std::round(targets[0].second * 0.2 - cross_y_main);                                                    
                                                }
                                                    break;
                                                case 2:
                                                {
                                                    temp_x = targets[0].first;
                                                    temp_y = targets[0].second;
                                                    display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                                    display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;                               
                                                } 
                                                    break;
                                                // case 3:
                                                // {
                                                //     display_x = std::round(targets[0].first * 0.4  - 3*cross_x_main);
                                                //     display_y = std::round(targets[0].second * 0.4 - 3*cross_y_main);
                                                //     // display_x = std::round((targets[0].first  - 2400)  * 0.4);//  + gate_offset_x;
                                                //     // display_y = std::round((targets[0].second - 1350) * 0.4);// + gate_offset_y;
                                                // }   
                                                //     break;
                                                default:
                                                {
                                                    temp_x = targets[0].first;
                                                    temp_y = targets[0].second;
                                                    display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                                    display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;                               
                                                } 
                                                    break;
                                            }
                                        }
                                        else
                                        {
                                            temp_x = targets[0].first;
                                            temp_y = targets[0].second;
                                            display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                            display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;  
                                        }                                        
                                        // smooth display when track offset is tiny 
                                        float x_coeff, y_coeff;
                                        x_coeff = std::abs(display_x - targets_[6]->cross.info.cols_hor_x) / 5.0;
                                        y_coeff = std::abs(display_y - targets_[6]->cross.info.rows_ver_y) / 5.0;
                                        x_coeff = (x_coeff <= 1.0) ? x_coeff : 1.0;
                                        y_coeff = (y_coeff <= 1.0) ? y_coeff : 1.0;
                                        targets_[6]->cross.info.cols_hor_x = std::round(x_coeff * display_x + (1 - x_coeff) * targets_[6]->cross.info.cols_hor_x);
                                        targets_[6]->cross.info.rows_ver_y = std::round(y_coeff * display_y + (1 - y_coeff) * targets_[6]->cross.info.rows_ver_y);  

                                        // bool visible = ((targets_[6]->cross.info.cols_hor_x > 64) && (targets_[6]->cross.info.cols_hor_x < 1856) && (targets_[6]->cross.info.rows_ver_y > 32) && (targets_[6]->cross.info.rows_ver_y < 1048));
                                        if(is_tv) //!XJ3UARTDevice::instance()->channel
                                            visible = ((targets_[6]->cross.info.cols_hor_x - targets_[6]->gate.info.width/2) > 0) && ((targets_[6]->cross.info.cols_hor_x + targets_[6]->gate.info.width/2) < 1920) && ((targets_[6]->cross.info.rows_ver_y - targets_[6]->gate.info.height/2) > 0) && ((targets_[6]->cross.info.rows_ver_y + targets_[6]->gate.info.height/2) < 1080) && (XJ3UARTDevice::instance()->channel == 0);
                                        else
                                            visible = ((targets_[6]->cross.info.cols_hor_x - targets_[6]->gate.info.width/2) > 320) && ((targets_[6]->cross.info.cols_hor_x + targets_[6]->gate.info.width/2) < 1600) && ((targets_[6]->cross.info.rows_ver_y - targets_[6]->gate.info.height/2) > 28) && ((targets_[6]->cross.info.rows_ver_y + targets_[6]->gate.info.height/2) < 1054) && (XJ3UARTDevice::instance()->channel == 1);
                                        targets_[6]->gate.info.Enable  = visible ? wave_gate_visible : false;
                                        targets_[6]->gate.info.IdEn    = false; 
                                        targets_[6]->cross.info.Enable = false;
                                        TarInfo_9.gate.info.Enable  = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false;
                                        //report tracking offset                           
                                        {   
                                        	if(HostUARTDevice::instance()->channel == (XJ3UARTDevice::instance()->channel+1) && !tv_zoom_status && !ir_zoom_status) //is_tv == (XJ3UARTDevice::instance()->channel==0)
                                        	{
                                                // x_coeff = std::abs(targets[0].first  - report_x) / 10.0 + 0.15;
                                                // y_coeff = std::abs(targets[0].second - report_y) / 10.0 + 0.15;
                                                x_coeff = std::abs(temp_x - report_x) / 10.0 + 0.15;
                                                y_coeff = std::abs(temp_y - report_y) / 10.0 + 0.15;
                                                x_coeff = (x_coeff <= 1.0) ? x_coeff : 1.0;
                                                y_coeff = (y_coeff <= 1.0) ? y_coeff : 1.0;
                                                // float temp_x = x_coeff * targets[0].first  + (1 - x_coeff) * report_x;
                                                // float temp_y = y_coeff * targets[0].second + (1 - y_coeff) * report_y;
                                                temp_x = std::round(x_coeff * temp_x + (1 - x_coeff) * report_x);
                                                temp_y = std::round(y_coeff * temp_y + (1 - y_coeff) * report_y);
                                            #ifdef SERVO_SPEED_LIMIT
                                                offset_x = temp_x - cross_x_main*10;//9600;
                                                offset_y = temp_y - cross_y_main*10;//5400;
                                                if(((offset_x <= 100) && (offset_x >=-100)) || (reached_x == 2))
                                                {
                                                    reached_x = 2; 
                                                    // report_x = std::round(temp_x);
                                                    report_x = temp_x;
                                                }
                                                else if(offset_x > 100)
                                                {
                                                    if(reached_x == 0) reached_x = 1;
                                                    offset_x = std::round(offset_x / 2.0 + 50);
                                                    report_x = cross_x_main*10 + offset_x; //9600
                                                }
                                                else
                                                {
                                                    if(reached_x == 0) reached_x = 1;
                                                    offset_x = std::round(offset_x / 2.0 - 50);
                                                    report_x = cross_x_main*10 + offset_x; //9600
                                                }
                                                if(((offset_y <= 100) && (offset_y >= -100)) || (reached_y == 2))
                                                {            
                                                    reached_y = 2;                               
                                                    // report_y = std::round(temp_y);
                                                    report_y = temp_y;
                                                }
                                                else if(offset_y > 100)
                                                {   
                                                    if(reached_y == 0) reached_y = 1;
                                                    offset_y = std::round(offset_y / 2.0 + 50);
                                                    report_y = cross_y_main*10 + offset_y; //5400
                                                }
                                                else
                                                {
                                                    if(reached_y == 0) reached_y = 1;
                                                    offset_y = std::round(offset_y / 2.0 - 50);
                                                    report_y = cross_y_main*10 + offset_y; //5400
                                                }
                                            #else 
                                                // report_x = std::round(temp_x);
                                                // report_y = std::round(temp_y); 
                                                report_x = temp_x;
                                                report_y = temp_y; 
                                            #endif
                                            }
                                            // else
                                            // {
                                            //     if(HostUARTDevice::instance()->channel == 1)
                                            //     {
                                            //         report_x = HostUARTDevice::instance()->cross_x_tv * 10;
                                            //         report_y = HostUARTDevice::instance()->cross_y_tv * 10;                                                    
                                            //     }
                                            //     else
                                            //     {
                                            //         report_x = HostUARTDevice::instance()->cross_x_ir * 10;
                                            //         report_y = HostUARTDevice::instance()->cross_y_ir * 10;                                                    
                                            //     }
                                            // }
                                            uint8_t params[4] = { uint8_t (report_x), uint8_t (report_x >> 8), uint8_t (report_y), uint8_t (report_y >> 8) };
                                            HostUARTDevice::instance()->write_command(0x09, SIZEOF(params), params);
                                            // printf("normal 1 report x:%d and y:%d. \n", report_x, report_y);
#ifdef MEASURE_LOOP_TIME
                                             end = std::chrono::system_clock::now();
                                             float _during = (float)std::chrono::duration_cast<std::chrono::microseconds> (end - time_MIPI_OUT).count();
                                             printf("Interval: %f ms, targets: %d and %d. \n", (double) _during/1000.0, targets[0].first, targets[0].second);
#endif
                                        }  
                                    }
                                        break;
                                    case 3: //keep slide
                                    {   
                                        if(normal_track_start == 0)
                                            normal_track_start = count;
                                        if(count == (normal_track_start + 5))  //in normal track mode, if continuous lost of target is 5, reduce speed of servo
                                        {
                                            reached_x = 0;
                                            reached_y = 0;
                                        }
                                        if(count < (normal_track_start + 10))
                                        {
                                            targets_[6]->gate.info.Enable = wave_gate_visible ? 1 : 0;
                                            TarInfo_9.gate.info.Enable = false;
                                        }
                                        else
                                        {   
                                            targets_[6]->gate.info.Enable = false;        
                                            TarInfo_9.gate.info.Enable = wave_gate_visible ? (count&0x08 ? 1 : 0) : 0;
                                        }           
                                        targets_[6]->gate.info.IdEn    = false;
                                        targets_[6]->cross.info.Enable = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false;
                                        #ifdef TRACKER_TEST_MODE
                                            TarInfo_9.cross.info.cols_hor_x = std::round(targets[0].first*0.1);//960; //for test
                                            TarInfo_9.cross.info.rows_ver_y = std::round(targets[0].second*0.1);//540;
                                        #else
                                            TarInfo_9.cross.info.cols_hor_x = cross_x_main;//960;
                                            TarInfo_9.cross.info.rows_ver_y = cross_y_main;//540;
                                        #endif

                                        if(HostUARTDevice::instance()->channel == (XJ3UARTDevice::instance()->channel+1)) //is_tv == (XJ3UARTDevice::instance()->channel==0)
                                        {
                                            if(HostUARTDevice::instance()->channel == 1)
                                            {
                                                report_x = HostUARTDevice::instance()->cross_x_tv * 10;
                                                report_y = HostUARTDevice::instance()->cross_y_tv * 10;                                                    
                                            }
                                            else
                                            {
                                                report_x = HostUARTDevice::instance()->cross_x_ir * 10;
                                                report_y = HostUARTDevice::instance()->cross_y_ir * 10;                                                    
                                            }
                                        }
                                        // else
                                        // {
                                        //     if(HostUARTDevice::instance()->channel == 1)
                                        //     {
                                        //         report_x = HostUARTDevice::instance()->cross_x_tv * 10;
                                        //         report_y = HostUARTDevice::instance()->cross_y_tv * 10;                                                    
                                        //     }
                                        //     else
                                        //     {
                                        //         report_x = HostUARTDevice::instance()->cross_x_ir * 10;
                                        //         report_y = HostUARTDevice::instance()->cross_y_ir * 10;                                                    
                                        //     }
                                        // }
                                        uint8_t params[4] = { uint8_t (report_x), uint8_t (report_x >> 8), uint8_t (report_y), uint8_t (report_y >> 8) };
                                        HostUARTDevice::instance()->write_command(0x09, SIZEOF(params), params);
                                        // printf("normal 2 report x:%d and y:%d. \n", report_x, report_y);
                                    }
                                        break;
                                    case 4: //lost
                                    {    
                                        track_command = 1;
                                        track_status  = 1;
                                        // XJ3UARTDevice::instance()->track_status = 1;
                                        targets_[6]->gate.info.Enable  = false;
                                        targets_[6]->gate.info.IdEn    = false;
                                        targets_[6]->cross.info.Enable = false; 
                                        TarInfo_9.gate.info.Enable  = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false;                                                          
                                    }
                                        break;
                                    default:
                                    {
                                        targets_[6]->gate.info.Enable  = false;
                                        targets_[6]->gate.info.IdEn    = false;
                                        targets_[6]->cross.info.Enable = false;
                                        TarInfo_9.gate.info.Enable  = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false;

                                        if(count == (count_track_start + resend_delay))
                                        {
                                            printf("resend track command = %d.\n", track_command);
                                            XJ3UARTDevice::instance()->write_command_control(0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                                            xj3_send_command = true;
                                        }
                                        else if(count == (count_track_start + resend_delay*2))
                                        {
                                            uint8_t params[1] = {0x03};
                                            HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                        } 
                                    }
                                }
                                #ifdef TRACKER_TEST_MODE                                
                                    targets_[6]->gate.info.width  = targets[1].first;  //for test
                                    targets_[6]->gate.info.height = targets[1].second;
                                    TarInfo_9.gate.info.width  = targets_[6]->gate.info.width;
                                    TarInfo_9.gate.info.height = targets_[6]->gate.info.height;
                                    // targets_[2]->gate.info.width  = targets[2].first;  //for test
                                    // targets_[2]->gate.info.height = targets[2].second;                                    
                                #else
                                    targets_[6]->gate.info.width  = std::round(64 * HostUARTDevice::instance()->wave_gate_width_scale);
                                    targets_[6]->gate.info.height = std::round(48 * HostUARTDevice::instance()->wave_gate_width_scale);
                                    TarInfo_9.gate.info.width  = targets_[6]->gate.info.width * 5;
                                    TarInfo_9.gate.info.height = targets_[6]->gate.info.height * 5;
                                #endif

                            }
                            // if(HostUARTDevice::instance()->track_command != 3)
                            //     track_command = HostUARTDevice::instance()->track_command;
                        }   
                            break; 
                        case 3: //aux track
                        {   
                            // HostUARTDevice::instance()->track_ID_send = false;
                            if(targets.empty())
                            {
                                for (int i = 0; i < 8; ++i)
                                {
                                    targets_[i]->gate.info.Enable = false;
                                    targets_[i]->gate.info.IdEn   = false;
                                    targets_[i]->cross.info.Enable = false;
                                }  
                            }   
                            else
                            {                             
                                if(XJ3UARTDevice::instance()->channel) //!is_tv
                                { 
                                    // targets[0].first  = 2*targets[0].first + 3200;// + gate_offset_x;
                                    // targets[0].second = 2*targets[0].second + 280;// + gate_offset_y;
                                    if(cross_x_main%2)
                                        targets[0].first = 2*(targets[0].first+5) + 3200;// + gate_offset_x;
                                    else
                                        targets[0].first = 2*targets[0].first + 3200;// + gate_offset_x;
                                    if(cross_y_main%2)
                                        targets[0].second = 2*(targets[0].second+5) + 280; // + gate_offset_y; 
                                    else
                                        targets[0].second = 2*targets[0].second + 280; // + gate_offset_y; 
                                }                             
                                for (int i = 0; i < 8; ++i)
                                {
                                    targets_[i]->gate.info.Enable = false;
                                    targets_[i]->gate.info.IdEn   = false;
                                    targets_[i]->cross.info.Enable = false;
                                }      
                                switch(track_status)
                                {
                                    // case 0:
                                    //     break;
                                    // case 1: //capture
                                    // {
                                    //     targets_[6]->gate.info.Enable = false;
                                    //     targets_[6]->gate.info.IdEn   = false;
                                    //     targets_[6]->cross.info.Enable = false;
                                    //     if(count == (count_track_start + resend_delay))
                                    //     {
                                    //         printf("resend track command%d.\n", track_command);
                                    //         XJ3UARTDevice::instance()->write_command_control(0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                                    //         xj3_send_command = true;
                                    //     }
                                    //     else if(count == (count_track_start + resend_delay*2))
                                    //     {
                                    //         uint8_t params[1] = {0x03};
                                    //         HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                    //     }
                                    // }    
                                    //     break;
                                    case 2: //track
                                    {
                                        normal_track_start = 0;
                                        int16_t temp_x, temp_y;
                                        if(!XJ3UARTDevice::instance()->channel) //is_tv
                                        {
                                            switch(tv_scale)
                                            {
                                                case 1:
                                                {
                                                    if((targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10) <= 0) 
                                                    {
                                                        temp_x = 0;
                                                    }
                                                    else if((targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10) >= 19200)
                                                    {
                                                        temp_x = 19200;
                                                    }
                                                    else
                                                    {
                                                        temp_x = targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10;
                                                    }

                                                    if((targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10) <= 0) 
                                                    {
                                                        temp_y = 0;
                                                    }
                                                    else if((targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10) >= 10800)
                                                    {
                                                        temp_y = 10800;
                                                    }
                                                    else
                                                    {
                                                        temp_y = targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10;
                                                    }
                                                    display_x = std::round(temp_x * 0.1); 
                                                    display_y = std::round(temp_y * 0.1);  
                                                    // display_x = std::round(targets[0].first * 0.2 - cross_x_main); 
                                                    // display_y = std::round(targets[0].second * 0.2 - cross_y_main);                                                    
                                                }
                                                    break;
                                                case 2:
                                                {
                                                    temp_x = targets[0].first;
                                                    temp_y = targets[0].second;
                                                    display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                                    display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;                               
                                                } 
                                                    break;
                                                // case 3:
                                                // {
                                                //     display_x = std::round(targets[0].first * 0.4  - 3*cross_x_main);
                                                //     display_y = std::round(targets[0].second * 0.4 - 3*cross_y_main);
                                                //     // display_x = std::round((targets[0].first  - 2400)  * 0.4);//  + gate_offset_x;
                                                //     // display_y = std::round((targets[0].second - 1350) * 0.4);// + gate_offset_y;
                                                // }   
                                                //     break;
                                                default:
                                                {
                                                    temp_x = targets[0].first;
                                                    temp_y = targets[0].second;
                                                    display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                                    display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;                               
                                                } 
                                                    break;
                                            }
                                        }
                                        else
                                        {
                                            temp_x = targets[0].first;
                                            temp_y = targets[0].second;
                                            display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                            display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;  
                                        }
                                        float x_coeff, y_coeff;
                                        x_coeff = (float)std::abs(display_x - targets_[6]->cross.info.cols_hor_x) / 5.0;
                                        y_coeff = (float)std::abs(display_y - targets_[6]->cross.info.rows_ver_y) / 5.0;
                                        x_coeff = (x_coeff <= 1.0) ? x_coeff : 1.0;
                                        y_coeff = (y_coeff <= 1.0) ? y_coeff : 1.0;
                                        targets_[6]->cross.info.cols_hor_x = std::round(x_coeff * display_x + (1 - x_coeff) * targets_[6]->cross.info.cols_hor_x);
                                        targets_[6]->cross.info.rows_ver_y = std::round(y_coeff * display_y + (1 - y_coeff) * targets_[6]->cross.info.rows_ver_y); 
                                                   
                                        // bool visible = ((targets_[6]->cross.info.cols_hor_x > 64) && (targets_[6]->cross.info.cols_hor_x < 1856) && (targets_[6]->cross.info.rows_ver_y > 32) && (targets_[6]->cross.info.rows_ver_y < 1048));
                                        if(is_tv) //is_tv
                                            visible = ((targets_[6]->cross.info.cols_hor_x - targets_[6]->gate.info.width/2) > 0) && ((targets_[6]->cross.info.cols_hor_x + targets_[6]->gate.info.width/2) < 1920) && ((targets_[6]->cross.info.rows_ver_y - targets_[6]->gate.info.height/2) > 0) && ((targets_[6]->cross.info.rows_ver_y + targets_[6]->gate.info.height/2) < 1080) && (XJ3UARTDevice::instance()->channel == 0);
                                        else
                                            visible = ((targets_[6]->cross.info.cols_hor_x - targets_[6]->gate.info.width/2) > 320) && ((targets_[6]->cross.info.cols_hor_x + targets_[6]->gate.info.width/2) < 1600) && ((targets_[6]->cross.info.rows_ver_y - targets_[6]->gate.info.height/2) > 28) && ((targets_[6]->cross.info.rows_ver_y + targets_[6]->gate.info.height/2) < 1054) && (XJ3UARTDevice::instance()->channel == 1);                                        
                                        targets_[6]->gate.info.Enable = visible ? wave_gate_visible : false;
                                        // targets_[6]->gate.info.IdEn   = visible ? wave_gate_visible : false; 
                                        targets_[6]->gate.info.IdEn   = false;
                                        targets_[6]->cross.info.Enable = false;
                                        TarInfo_9.gate.info.Enable  = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false;                              
                                        {
                                        	if(HostUARTDevice::instance()->channel == (XJ3UARTDevice::instance()->channel+1)) //is_tv == (XJ3UARTDevice::instance()->channel==0)
                                        	{
                                                // x_coeff = std::abs(targets[0].first  - report_x) / 10.0  + 0.15;
                                                // y_coeff = std::abs(targets[0].second - report_y) / 10.0  + 0.15;
                                                x_coeff = std::abs(temp_x - report_x) / 10.0  + 0.15;
                                                y_coeff = std::abs(temp_y - report_y) / 10.0  + 0.15;
                                                x_coeff = (x_coeff <= 1.0) ? x_coeff : 1.0;
                                                y_coeff = (y_coeff <= 1.0) ? y_coeff : 1.0;
                                                // float temp_x = x_coeff * targets[0].first  + (1 - x_coeff) * report_x;
                                                // float temp_y = y_coeff * targets[0].second + (1 - y_coeff) * report_y;
                                                temp_x = std::round(x_coeff * temp_x + (1 - x_coeff) * report_x);
                                                temp_y = std::round(y_coeff * temp_y + (1 - y_coeff) * report_y);
                                                offset_x = temp_x - cross_x_main*10;//9600;
                                                offset_y = temp_y - cross_y_main*10;//5400;

                                                if(servo_start)
                                                {
                                                    if(offset_x > 50 || offset_x < -50)
                                                        reached_x_servo = 0;
                                                    if(offset_y > 50 || offset_y < -50)
                                                        reached_y_servo = 0; 
                                                    if((reached_x_servo == 0) || (reached_y_servo == 0))
                                                        servo_start = false;
                                                }
                                                else
                                                {
                                                    if(((offset_x <= 100) && (offset_x >= -100)) || (reached_x_servo == 2))
                                                        if(reached_x_servo) reached_x_servo = 2;
                                                    else if(reached_x_servo == 0)
                                                        reached_x_servo = 1;
                                                    if(((offset_y <= 100) && (offset_y >= -100)) || (reached_y_servo == 2))
                                                        if(reached_y_servo) reached_y_servo = 2;                         
                                                    else if(reached_y_servo == 0)
                                                        reached_y_servo = 1;  
                                                }   
                                                if(((reached_x_servo == 2) && (reached_y_servo == 2)) || (count >= (count_track_start + 90)))
                                                { 
                                                    track_command = 2; 
                                                    XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0000, 0x0000, 0x00); 
                                                    xj3_send_command = true;
                                                }

                                                // report_x = std::round(temp_x);
                                                // report_y = std::round(temp_y);
                                                report_x = temp_x;
                                                report_y = temp_y;
                                                uint8_t params[4] = { uint8_t (report_x), uint8_t (report_x >> 8), uint8_t (report_y), uint8_t (report_y >> 8) };
                                                HostUARTDevice::instance()->write_command(0x09, SIZEOF(params), params);
                                                // printf("aux 1 report x:%d and y:%d. \n", report_x, report_y);
                                                // HostUARTDevice::instance()->write_command_new(0x09, SIZEOF(params), params, HostUARTDevice::instance()->write_data, &HostUARTDevice::instance()->write_data_length);
                                            }
                                        }  
                                    }
                                        break;
                                    case 3: // keep slide
                                    {   
                                        if(normal_track_start == 0)
                                            normal_track_start = count;
                                        if(count == (normal_track_start + 5))  //in aux_track mode, if continuous lost of target is 5, report track_status slide
                                        {
                                            uint8_t params[1] = {0x02};
                                            HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                            slide_status = 2;
                                            // printf("report slide when aux_track.\n");
                                        }  
                                        if(count < (count_track_start + 45))
                                        {
                                            // targets_[6]->gate.info.Enable = wave_gate_visible ? 1 : 0;
                                            // targets_[6]->cross.info.cols_hor_x = targets_[6]->cross.info.cols_hor_x - delta_x;
                                            // targets_[6]->cross.info.rows_ver_y = targets_[6]->cross.info.rows_ver_y - delta_y;
                                            targets_[6]->gate.info.Enable = false; 
                                            TarInfo_9.gate.info.Enable = false;
                                        }
                                        else if(count >= (count_track_start + 120))
                                        {
                                            track_command = 2;
                                        }
                                        else
                                        {   
                                            targets_[6]->gate.info.Enable = false;        
                                            TarInfo_9.gate.info.Enable = wave_gate_visible ? (count&0x08 ? 1 : 0) : 0;
                                        }
                                        // targets_[6]->gate.info.IdEn   = wave_gate_visible ? (count&0x08 ? 1 : 0) : 0;
                                        targets_[6]->gate.info.IdEn    = false; 
                                        targets_[6]->cross.info.Enable = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false;
                                        #ifdef TRACKER_TEST_MODE
                                            TarInfo_9.cross.info.cols_hor_x = std::round(targets[0].first*0.1);  //for test//960; //for test
                                            TarInfo_9.cross.info.rows_ver_y = std::round(targets[0].second*0.1);//540;  
                                        #else                                         
                                            TarInfo_9.cross.info.cols_hor_x = cross_x_main;//960;
                                            TarInfo_9.cross.info.rows_ver_y = cross_y_main;//540;
                                        #endif
                                        {
                                        	if(HostUARTDevice::instance()->channel == (XJ3UARTDevice::instance()->channel+1)) //is_tv == (XJ3UARTDevice::instance()->channel==0)
                                        	{
                                                if(report_x < (cross_x_main * 10))
                                                {
                                                    if(delta_x * 10 <= 0)
                                                        report_x = report_x - delta_x * 10;
                                                    report_x = (report_x > (cross_x_main * 10)) ? (cross_x_main * 10) : report_x;
                                                }
                                                else
                                                {
                                                    if(delta_x * 10 >= 0)
                                                        report_x = report_x - delta_x * 10;
                                                    report_x = (report_x < (cross_x_main * 10)) ? (cross_x_main * 10) : report_x;
                                                }
                                                if(report_y < (cross_y_main * 10))
                                                {
                                                    if(delta_y * 10 <= 0)
                                                        report_y = report_y - delta_y * 10;
                                                    report_y = (report_y > (cross_y_main * 10)) ? (cross_y_main * 10) : report_y;
                                                }
                                                else
                                                {
                                                    if(delta_y * 10 >= 0)
                                                        report_y = report_y - delta_y * 10;
                                                    report_y = (report_y < (cross_y_main * 10)) ? (cross_y_main * 10) : report_y;
                                                }

                                                uint8_t params[4] = { uint8_t (report_x), uint8_t (report_x >> 8), uint8_t (report_y), uint8_t (report_y >> 8) };
                                                HostUARTDevice::instance()->write_command(0x09, SIZEOF(params), params);
                                                // printf("aux 2 report x:%d and y:%d. \n", report_x, report_y);
                                            }
                                        } 
                                    }
                                        break;
                                    case 4: //lost
                                    {    
                                        track_command = 1;
                                        track_status  = 1;
                                        // XJ3UARTDevice::instance()->track_status = 1;
                                        targets_[6]->gate.info.Enable  = false;
                                        targets_[6]->gate.info.IdEn    = false;
                                        targets_[6]->cross.info.Enable = false;
                                        TarInfo_9.gate.info.Enable  = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false;                                                            
                                    }
                                        break;
                                    default:
                                    {
                                        targets_[6]->gate.info.Enable  = false;
                                        targets_[6]->gate.info.IdEn    = false;
                                        targets_[6]->cross.info.Enable = false;
                                        TarInfo_9.gate.info.Enable  = false;
                                        TarInfo_9.gate.info.IdEn    = false;
                                        TarInfo_9.cross.info.Enable = false; 

                                        if(count == (count_track_start + resend_delay))
                                        {
                                            printf("resend track command%d.\n", track_command);
                                            XJ3UARTDevice::instance()->write_command_control(0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                                            xj3_send_command = true;
                                        }
                                        else if(count == (count_track_start + resend_delay*2))
                                        {
                                            uint8_t params[1] = {0x03};
                                            HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                        }
                                    }
                                }
                                #ifdef TRACKER_TEST_MODE
                                    targets_[6]->gate.info.width  = targets[1].first;  //for test
                                    targets_[6]->gate.info.height = targets[1].second;
                                    TarInfo_9.gate.info.width  = targets_[6]->gate.info.width;
                                    TarInfo_9.gate.info.height = targets_[6]->gate.info.height;
                                #else 
                                    targets_[6]->gate.info.width  = std::round(64 * HostUARTDevice::instance()->wave_gate_width_scale);
                                    targets_[6]->gate.info.height = std::round(48 * HostUARTDevice::instance()->wave_gate_width_scale);
                                    TarInfo_9.gate.info.width  = targets_[6]->gate.info.width * 5;
                                    TarInfo_9.gate.info.height = targets_[6]->gate.info.height * 5;
                                #endif
                            }
                            //track_command = HostUARTDevice::instance()->track_command;
                        }  
                            break;
                        case 4: //second track
                        {
                            if(targets.empty())
                            {
                                for (int i = 0; i < 8; ++i)
                                {
                                    targets_[i]->gate.info.Enable  = false;
                                    targets_[i]->gate.info.IdEn    = false;
                                    targets_[i]->cross.info.Enable = false;
                                }  
                            }
                            else
                            {                                
                                // if( HostUARTDevice::instance()->second_capture_changed && (second_capture ==1) && ( std::abs(targets[0].first/10 - HostUARTDevice::instance()->cross_symbol_x) < 5 || std::abs(targets[0].second/10 - HostUARTDevice::instance()->cross_symbol_y) < 5 ) )
                                // {
                                //     uint8_t params[1] = {second_capture};
                                //     //printf("%d,%d,%d,%d\n", targets[0].first/10, targets[0].second/10, HostUARTDevice::instance()->cross_symbol_x, HostUARTDevice::instance()->cross_symbol_y);
                                //     HostUARTDevice::instance()->write_command(0x14, SIZEOF(params), params);
                                //     HostUARTDevice::instance()->second_capture_changed = false;
                                // }  
                                if(XJ3UARTDevice::instance()->channel) //!it_tv
                                {
                                    // targets[0].first  = 2*targets[0].first + 3200;// + gate_offset_x;
                                    // targets[0].second = 2*targets[0].second + 280;// + gate_offset_y;
                                    if(cross_x_main%2)
                                        targets[0].first = 2*(targets[0].first+5) + 3200;// + gate_offset_x;
                                    else
                                        targets[0].first = 2*targets[0].first + 3200;// + gate_offset_x;
                                    if(cross_y_main%2)
                                        targets[0].second = 2*(targets[0].second+5) + 280; // + gate_offset_y; 
                                    else
                                        targets[0].second = 2*targets[0].second + 280; // + gate_offset_y; 
                                }                          
                                for (int i = 0; i < 8; ++i)
                                {
                                    targets_[i]->gate.info.Enable  = false;
                                    targets_[i]->gate.info.IdEn    = false;
                                    targets_[i]->cross.info.Enable = false;
                                }  
                                {                                           
                                    //bool visible = ((targets_[6]->cross.info.cols_hor_x > 64) && (targets_[6]->cross.info.cols_hor_x < 1856) && (targets_[6]->cross.info.rows_ver_y > 32) && (targets_[6]->cross.info.rows_ver_y < 1048));
                                    if(is_tv) //!XJ3UARTDevice::instance()->channel
                                        visible = ((targets_[6]->cross.info.cols_hor_x - targets_[6]->gate.info.width/2) > 0) && ((targets_[6]->cross.info.cols_hor_x + targets_[6]->gate.info.width/2) < 1920) && ((targets_[6]->cross.info.rows_ver_y - targets_[6]->gate.info.height/2) > 0) && ((targets_[6]->cross.info.rows_ver_y + targets_[6]->gate.info.height/2) < 1080) && (XJ3UARTDevice::instance()->channel == 0);
                                    else
                                        visible = ((targets_[6]->cross.info.cols_hor_x - targets_[6]->gate.info.width/2) > 320) && ((targets_[6]->cross.info.cols_hor_x + targets_[6]->gate.info.width/2) < 1600) && ((targets_[6]->cross.info.rows_ver_y - targets_[6]->gate.info.height/2) > 28) && ((targets_[6]->cross.info.rows_ver_y + targets_[6]->gate.info.height/2) < 1054) && (XJ3UARTDevice::instance()->channel == 1);                                    

                                    targets_[6]->gate.info.Enable  = visible ? wave_gate_visible : false;
                                    targets_[6]->gate.info.IdEn    = false;  
                                    targets_[6]->cross.info.Enable = false;
                                    TarInfo_9.gate.info.Enable  = false;
                                    TarInfo_9.gate.info.IdEn    = false;
                                    TarInfo_9.cross.info.Enable = false;
                                    second_target->gate.info.Enable = false;
                                    second_target->gate.info.IdEn   = false;
                                    second_target->cross.info.Enable = false;                                        
                                    switch(track_status)
                                    {
                                        // case 0:
                                        //     break;
                                        // case 1:
                                        // {
                                        //     targets_[6]->gate.info.Enable = false;
                                        //     targets_[6]->gate.info.IdEn   = false;
                                        //     targets_[6]->cross.info.Enable = false;
                                        //     if(count == (count_track_start + resend_delay))
                                        //     {
                                        //         printf("resend track command = %d.\n", track_command);
                                        //         if((HostUARTDevice::instance()->cross_symbol_x > 0) && (HostUARTDevice::instance()->cross_symbol_y > 0))
                                        //         {
                                        //             XJ3UARTDevice::instance()->write_command_control(0xF4, 0xFF, 0xFF, 0xFF, 0xFF, HostUARTDevice::instance()->cross_symbol_x, HostUARTDevice::instance()->cross_symbol_y, 0x01);
                                        //             xj3_send_command = true;
                                        //         }
                                        //     }
                                        //     else if(count == (count_track_start + resend_delay*2))
                                        //     {
                                        //         uint8_t params[1] = {0x03};
                                        //         HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                        //     }
                                        // }    
                                        //     break;
                                        case 2: //track
                                        {
                                            normal_track_start = 0;
                                            int16_t temp_x, temp_y;
                                            if(!XJ3UARTDevice::instance()->channel) //is_tv
                                            {
                                                switch(tv_scale)
                                                {
                                                    case 1:
                                                    {
                                                        if((targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10) <= 0) 
                                                        {
                                                            temp_x = 0;
                                                        }
                                                        else if((targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10) >= 19200)
                                                        {
                                                            temp_x = 19200;
                                                        }
                                                        else
                                                        {
                                                            temp_x = targets[0].first * 2 - cross_x_main_tv_z * 20 + cross_x_main * 10;
                                                        }

                                                        if((targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10) <= 0) 
                                                        {
                                                            temp_y = 0;
                                                        }
                                                        else if((targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10) >= 10800)
                                                        {
                                                            temp_y = 10800;
                                                        }
                                                        else
                                                        {
                                                            temp_y = targets[0].second * 2 - cross_y_main_tv_z * 20 + cross_y_main * 10;
                                                        }
                                                        display_x = std::round(temp_x * 0.1); 
                                                        display_y = std::round(temp_y * 0.1);  
                                                        // display_x = std::round(targets[0].first * 0.2 - cross_x_main); 
                                                        // display_y = std::round(targets[0].second * 0.2 - cross_y_main);                                                    
                                                    }
                                                        break;
                                                    case 2:
                                                    {
                                                        temp_x = targets[0].first;
                                                        temp_y = targets[0].second;
                                                        display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                                        display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;                               
                                                    } 
                                                        break;
                                                    // case 3:
                                                    // {
                                                    //     display_x = std::round(targets[0].first * 0.4  - 3*cross_x_main);
                                                    //     display_y = std::round(targets[0].second * 0.4 - 3*cross_y_main);
                                                    //     // display_x = std::round((targets[0].first  - 2400)  * 0.4);//  + gate_offset_x;
                                                    //     // display_y = std::round((targets[0].second - 1350) * 0.4);// + gate_offset_y;
                                                    // }   
                                                    //     break;
                                                    default:
                                                    {
                                                        temp_x = targets[0].first;
                                                        temp_y = targets[0].second;
                                                        display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                                        display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;                               
                                                    } 
                                                        break;
                                                }
                                            }
                                            else
                                            {
                                                temp_x = targets[0].first;
                                                temp_y = targets[0].second;
                                                display_x = std::round(targets[0].first * 0.1);//  + gate_offset_x;
                                                display_y = std::round(targets[0].second * 0.1);// + gate_offset_y;  
                                            }          
                                            float x_coeff, y_coeff;
                                            x_coeff = (float)std::abs(display_x - targets_[6]->cross.info.cols_hor_x) / 5.0;
                                            y_coeff = (float)std::abs(display_y - targets_[6]->cross.info.rows_ver_y) / 5.0;
                                            x_coeff = (x_coeff <= 1.0) ? x_coeff : 1.0;
                                            y_coeff = (y_coeff <= 1.0) ? y_coeff : 1.0;
                                            targets_[6]->cross.info.cols_hor_x = std::round(x_coeff * display_x + (1 - x_coeff) * targets_[6]->cross.info.cols_hor_x);
                                            targets_[6]->cross.info.rows_ver_y = std::round(y_coeff * display_y + (1 - y_coeff) * targets_[6]->cross.info.rows_ver_y);                          
                                            {
                                                if(HostUARTDevice::instance()->channel == (XJ3UARTDevice::instance()->channel+1)) //is_tv == (XJ3UARTDevice::instance()->channel==0)
                                                {
                                                    // x_coeff = std::abs(targets[0].first  - report_x) / 10.0 + 0.15;
                                                    // y_coeff = std::abs(targets[0].second - report_y) / 10.0 + 0.15;
                                                    x_coeff = std::abs(temp_x - report_x) / 10.0 + 0.15;
                                                    y_coeff = std::abs(temp_y - report_y) / 10.0 + 0.15;
                                                    x_coeff = (x_coeff <= 1.0) ? x_coeff : 1.0;
                                                    y_coeff = (y_coeff <= 1.0) ? y_coeff : 1.0;
                                                    // float temp_x = x_coeff * targets[0].first  + (1 - x_coeff) * report_x;
                                                    // float temp_y = y_coeff * targets[0].second + (1 - y_coeff) * report_y;
                                                    temp_x = std::round(x_coeff * temp_x + (1 - x_coeff) * report_x);
                                                    temp_y = std::round(y_coeff * temp_y + (1 - y_coeff) * report_y);

                                                    offset_x = temp_x - cross_x_main*10;//9600;
                                                    offset_y = temp_y - cross_y_main*10;//5400;

                                                    if(servo_start)
                                                    {
                                                        if(offset_x > 50 || offset_x < -50)
                                                            reached_x_servo = 0;
                                                        if(offset_y > 50 || offset_y < -50)
                                                            reached_y_servo = 0; 
                                                        if((reached_x_servo == 0) || (reached_y_servo == 0))
                                                            servo_start = false;
                                                    }
                                                    else
                                                    {
                                                        if(((offset_x <= 100) && (offset_x >= -100)) || (reached_x_servo == 2))
                                                            if(reached_x_servo) reached_x_servo = 2;
                                                        else if(reached_x_servo == 0)
                                                            reached_x_servo = 1;
                                                        if(((offset_y <= 100) && (offset_y >= -100)) || (reached_y_servo == 2))
                                                            if(reached_y_servo) reached_y_servo = 2;                         
                                                        else if(reached_y_servo == 0)
                                                            reached_y_servo = 1;  
                                                    }   
                                                    if(((reached_x_servo == 2) && (reached_y_servo == 2)) || (count >= (count_track_start + 90)))
                                                    { 
                                                        track_command = 2; 
                                                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0000, 0x0000, 0x00); 
                                                        xj3_send_command = true;
                                                    }

                                                    // report_x = std::round(temp_x);
                                                    // report_y = std::round(temp_y);
                                                    report_x = temp_x;
                                                    report_y = temp_y;
                                                    uint8_t params[4] = { uint8_t (report_x), uint8_t (report_x >> 8), uint8_t (report_y), uint8_t (report_y >> 8) };
                                                    HostUARTDevice::instance()->write_command(0x09, SIZEOF(params), params);
                                                    // printf("second 1 report x:%d and y:%d. \n", report_x, report_y);
                                                    // HostUARTDevice::instance()->write_command_new(0x09, SIZEOF(params), params, HostUARTDevice::instance()->write_data, &HostUARTDevice::instance()->write_data_length);
                                                }
                                            }  
                                        }
                                            break;
                                        case 3:// keep slide
                                        {    
                                            if(normal_track_start == 0)
                                                normal_track_start = count;
                                            if(count == (normal_track_start + 5))  //in second_track mode, if continuous lost of target is 5, report track_status slide
                                            {
                                                uint8_t params[1] = {0x02};
                                                HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                                slide_status = 2;
                                                // printf("report slide when second_track.\n");
                                            }  
                                            if(count < (count_track_start + 45))
                                            {
                                                // targets_[6]->gate.info.Enable = wave_gate_visible ? 1 : 0;
                                                // targets_[6]->cross.info.cols_hor_x = targets_[6]->cross.info.cols_hor_x - delta_x;
                                                // targets_[6]->cross.info.rows_ver_y = targets_[6]->cross.info.rows_ver_y - delta_y;
                                                targets_[6]->gate.info.Enable = false;
                                                TarInfo_9.gate.info.Enable = false;
                                            }
                                            else if(count >= (count_track_start + 120))
                                            {
                                                track_command = 2;
                                            }
                                            else
                                            {   
                                                targets_[6]->gate.info.Enable = false;        
                                                TarInfo_9.gate.info.Enable = wave_gate_visible ? (count&0x08 ? 1 : 0) : 0;
                                            }              
                                            targets_[6]->gate.info.IdEn    = false;
                                            targets_[6]->cross.info.Enable = false;
                                            TarInfo_9.gate.info.IdEn    = false;
                                            TarInfo_9.cross.info.Enable = false; 
                                            #ifdef TRACKER_TEST_MODE
                                                TarInfo_9.cross.info.cols_hor_x = std::round(targets[0].first*0.1); //960;
                                                TarInfo_9.cross.info.rows_ver_y = std::round(targets[0].second*0.1); //540;
                                            #else
                                                TarInfo_9.cross.info.cols_hor_x = cross_x_main;// targets_[6]->cross.info.cols_hor_x; //cross_x_main;
                                                TarInfo_9.cross.info.rows_ver_y = cross_y_main;// targets_[6]->cross.info.rows_ver_y; //cross_y_main; 
                                            #endif      
                                            {                                        
                                                if(HostUARTDevice::instance()->channel == (XJ3UARTDevice::instance()->channel+1)) //is_tv == (XJ3UARTDevice::instance()->channel==0)
                                                {
                                                    if(report_x < (cross_x_main * 10))
                                                    {
                                                        if(delta_x * 10 <= 0)
                                                            report_x = report_x - delta_x * 10;
                                                        report_x = (report_x > (cross_x_main * 10)) ? (cross_x_main * 10) : report_x;
                                                    }
                                                    else
                                                    {
                                                        if(delta_x * 10 >= 0)
                                                            report_x = report_x - delta_x * 10;
                                                        report_x = (report_x < (cross_x_main * 10)) ? (cross_x_main * 10) : report_x;
                                                    }
                                                    if(report_y < (cross_y_main * 10))
                                                    {
                                                        if(delta_y * 10 <= 0)
                                                            report_y = report_y - delta_y * 10;
                                                        report_y = (report_y > (cross_y_main * 10)) ? (cross_y_main * 10) : report_y;
                                                    }
                                                    else
                                                    {
                                                        if(delta_y * 10 >= 0)
                                                            report_y = report_y - delta_y * 10;
                                                        report_y = (report_y < (cross_y_main * 10)) ? (cross_y_main * 10) : report_y;
                                                    }

                                                    uint8_t params[4] = { uint8_t (report_x), uint8_t (report_x >> 8), uint8_t (report_y), uint8_t (report_y >> 8) };
                                                    HostUARTDevice::instance()->write_command(0x09, SIZEOF(params), params);
                                                    // printf("second 2 report x:%d and y:%d. \n", report_x, report_y);
                                                    // HostUARTDevice::instance()->write_command_new(0x09, SIZEOF(params), params, HostUARTDevice::instance()->write_data, &HostUARTDevice::instance()->write_data_length);
                                                }
                                            } 
                                        }
                                            break;
                                        case 4: //lost
                                        {     
                                            track_command = 1;
                                            track_status  = 1;
                                            // XJ3UARTDevice::instance()->track_status = 1;
                                            targets_[6]->gate.info.Enable = false;
                                            targets_[6]->gate.info.IdEn   = false;
                                            targets_[6]->cross.info.Enable = false;
                                            TarInfo_9.gate.info.Enable    = false;
                                            TarInfo_9.gate.info.IdEn      = false;
                                            TarInfo_9.cross.info.Enable   = false;
                                            second_target->gate.info.Enable = false;
                                            second_target->gate.info.IdEn   = false; 
                                            second_target->cross.info.Enable = false;               
                                        }
                                            break;
                                        default:
                                        {
                                            targets_[6]->gate.info.Enable = false;
                                            targets_[6]->gate.info.IdEn   = false;
                                            targets_[6]->cross.info.Enable = false;
                                            TarInfo_9.gate.info.Enable    = false;
                                            TarInfo_9.gate.info.IdEn      = false;
                                            TarInfo_9.cross.info.Enable   = false;
                                            second_target->gate.info.Enable = false;
                                            second_target->gate.info.IdEn   = false; 
                                            second_target->cross.info.Enable = false; 

                                            if(count == (count_track_start + resend_delay))
                                            {
                                                printf("resend track command = %d.\n", track_command);
                                                if((HostUARTDevice::instance()->cross_symbol_x > 0) && (HostUARTDevice::instance()->cross_symbol_y > 0))
                                                {
                                                    XJ3UARTDevice::instance()->write_command_control(0xF4, 0xFF, 0xFF, 0xFF, 0xFF, HostUARTDevice::instance()->cross_symbol_x, HostUARTDevice::instance()->cross_symbol_y, 0x01);
                                                    xj3_send_command = true;
                                                }
                                            }
                                            else if(count == (count_track_start + resend_delay*2))
                                            {
                                                uint8_t params[1] = {0x03};
                                                HostUARTDevice::instance()->write_command(0x08, SIZEOF(params), params);
                                            }  
                                        }
                                    }
                                    #ifdef TRACKER_TEST_MODE
                                        targets_[6]->gate.info.width  = targets[1].first;  //for test
                                        targets_[6]->gate.info.height = targets[1].second;
                                        TarInfo_9.gate.info.width  = targets_[6]->gate.info.width;
                                        TarInfo_9.gate.info.height = targets_[6]->gate.info.height;   
                                    #else
                                        targets_[6]->gate.info.width  = std::round(64 * HostUARTDevice::instance()->wave_gate_width_scale);
                                        targets_[6]->gate.info.height = std::round(48 * HostUARTDevice::instance()->wave_gate_width_scale);
                                        TarInfo_9.gate.info.width  = targets_[6]->gate.info.width * 5;
                                        TarInfo_9.gate.info.height = targets_[6]->gate.info.height * 5;                                        
                                    #endif
                              
                                }
                            }                           
                        }
                            break;
                        default:
                        {
                            for (int i = 0; i < 8; ++i)
                            {
                                targets_[i]->gate.info.Enable = false;
                                targets_[i]->gate.info.IdEn = false;
                                targets_[i]->cross.info.Enable = false;
                            } 
                            // track_command = HostUARTDevice::instance()->track_command;                             
                        }
                    }   
                    
                    GPIO_TAR1_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_1.cross.Send;
                    GPIO_TAR1_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_1.gate.Send;
                    GPIO_TAR2_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_2.cross.Send;
                    GPIO_TAR2_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_2.gate.Send;
                    GPIO_TAR3_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_3.cross.Send;
                    GPIO_TAR3_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_3.gate.Send;
                    GPIO_TAR4_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_4.cross.Send;
                    GPIO_TAR4_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_4.gate.Send;
                    GPIO_TAR5_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_5.cross.Send;
                    GPIO_TAR5_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_5.gate.Send;
                    GPIO_TAR6_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_6.cross.Send;
                    GPIO_TAR6_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_6.gate.Send;
                    GPIO_TAR7_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_7.cross.Send;
                    GPIO_TAR7_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_7.gate.Send;
                    GPIO_TAR8_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_8.cross.Send;
                    GPIO_TAR8_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_8.gate.Send;
                    GPIO_TAR9_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_9.cross.Send;
                    GPIO_TAR9_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_9.gate.Send;
                    // GPIO_TAR10_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_10.cross.Send;
                    // GPIO_TAR10_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_10.gate.Send;
                }                
            }

            // mipi node self_check status
            if(XJ3UARTDevice::instance()->mipi_reset)
            {
                printf("XJ3 mipi node reset!\n");
                watchdog_mipi_count = 0;
                XJ3UARTDevice::instance()->mipi_reset = false;
            }
            if(XJ3UARTDevice::instance()->self_check_ok)
            {
                printf("XJ3 mipi node is OK!\n");
                XJ3UARTDevice::instance()->self_check_ok = false;
                watchdog_mipi_count = 0;
                ++count_mipi_ok;
            }
            if(XJ3UARTDevice::instance()->start_ok)
            {
                if(count_mipi_ok==0)
                {
                    rst_lt9211 = true;  //step-1: prepare to reset LT9211;
                    pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst 				= 0;	//step-2: setp LT9211 rst sig
                    GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");//step-2: write LT9211 rst sig;
                    XJ3UARTDevice::instance()->start_ok = false;
                    printf("reset XJ3 mipi node when start.\n");
                }
                else
                {
                    XJ3UARTDevice::instance()->start_ok = false;
                    printf("XJ3 mipi node start ok.\n");
                }    
            }
// ********************end: targets tracking and detection **********************************//

// ********************start: FPGA Uart report to Host **************************************//
            if(HostUARTDevice::instance()->data_changed)
            {
                int len = 0;
                // printf("write_data_length:%d\n", HostUARTDevice::instance()->write_data_length);
                if( (Uart_HOST_TX_HANDLE[0x08 >> 2] & 0x04) && (HostUARTDevice::instance()->write_data_length > 0) )
                {
                    len = HOST_uart_write_callback(HostUARTDevice::instance()->write_data, HostUARTDevice::instance()->write_data_length);
                    // printf("Send_cmd:%d, length:%d.\n", HostUARTDevice::instance()->write_data[3], len);
                }
                // else
                // {
                //     if(HostUARTDevice::instance()->write_data_length > 0)
                // 			printf("UartHOST_TX_IS_NOT_EMPTY!\n");
                //		else
                //     		printf("write_data_length 0!\n");
                // }
                if(len == HostUARTDevice::instance()->write_data_length)
                {
                    // printf("Clear send_buf:%d.\n", HostUARTDevice::instance()->write_data_length);
                    HostUARTDevice::instance()->data_changed = false;
                    HostUARTDevice::instance()->write_data.clear();
                    HostUARTDevice::instance()->write_data_length = 0;
                }
            }
// ********************end: FPGA Uart report to Host **************************************//

// ********************start: display control using device->HostUARTDevice::instance()  **************************************//
            // video display interrupt

            if (video_info_DISP_OUT.ready)
            {        
                ++count; 
                // watchdog_mipi, if not receive xj3 mipi_node ok for 45s, reset lt9211 and mipi_node
                ++watchdog_mipi_count;

                // fov changed message for XJ3 reset tracker size
                if(HostUARTDevice::instance()->yaw_view_angle_changed && !xj3_send_command && !channel_changed)
                {
                    HostUARTDevice::instance()->yaw_view_angle_changed = false;
                    if(is_tv)
                    {
                        if(HostUARTDevice::instance()->tv_yaw_view_angle > 180)
                        {
                            XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->tv_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->tv_yaw_view_angle & 0xFF), 0xFFFF, 0xFFFF, 0xFF);
                        }
                    }
                    else
                    {   
                        if(HostUARTDevice::instance()->ir_yaw_view_angle > 60)
                        {
                            XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->ir_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->ir_yaw_view_angle & 0xFF), 0xFFFF, 0xFFFF, 0xFF);  
                        }     
                    }
                    // HostUARTDevice::instance()->yaw_view_angle_changed = false;
                    xj3_send_command = true;
                }

                if(watchdog_mipi_count == 1200)
                {
                    rst_lt9211 = true;  //step-1: prepare to reset LT9211;
                    pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst 				= 0;	//step-2: setp LT9211 rst sig
                    GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");//step-2: write LT9211 rst sig;
                    printf("manually reset mipi VTC.\n");            
                }

                // ir black_white status change
                // if ( HostUARTDevice::instance()->ir_power )
                // {
                //     if ( ir_black_white != HostUARTDevice::instance()->ir_mode )
                //     {
                //         ir_black_white_switch_happened = true;
                //         ir_black_white = HostUARTDevice::instance()->ir_mode;
                //         uint8_t params[1];
                //         if(ir_black_white)
                //             params[0] = {0x06};
                //         else
                //             params[0] = {0x07};
                //         HostUARTDevice::instance()->write_command(0x18, SIZEOF(params), params);
                //         // printf("ir_black_white: %d  HostUARTDevice::instance()->ir_mode: %d\n",ir_black_white,HostUARTDevice::instance()->ir_mode);
                //     }
                // }
                // tv color_mono status change
                {
                    if( tv_mode != HostUARTDevice::instance()->tv_mode)
                    {
                        count_tv_mode = count;  // start_time of tv_mode_change
                        tv_mode = HostUARTDevice::instance()->tv_mode;
                        tv_mode_change_start = true;
                    }
                    if(count == (count_tv_mode + 20)) // end_time of tv_mode_change
                    {
                        tv_mode_change_end = true;
                        count_tv_mode = -100;
                    }
                }

                if (show_color != HostUARTDevice::instance()->show_color)
                {
                    if(HostUARTDevice::instance()->show_color == 7)
                    {
#ifdef ENABLE_XML_FILE_SAVED_FUNC
                        fs.open("/root/color.xml", cv::FileStorage::WRITE);
                        fs << "color" << show_color;
                        fs.release();
                       // system("sync");//2023.02.03 added by fyc---sync
						HostUARTDevice::instance()->show_color = show_color;
#ifdef DEBUG_PRINTF
                        printf("color cfg saved..\n");
#endif
#endif
                    }
                    else
                        show_color = HostUARTDevice::instance()->show_color;
                    osd.switch_osd_color(show_color);
                    targraphColor.ColorConfig.color.reserved = 0;
                    targraphColor.ColorConfig.color.Color_R_Y = osd.color_cur_Y;
                    targraphColor.ColorConfig.color.Color_G_U = osd.color_cur_U;
                    targraphColor.ColorConfig.color.Color_B_V = osd.color_cur_V;

                    try
                    {
                        north_angle = std::stof(north_angle_str);
                    }
                    catch (std::invalid_argument& e)
                    {
                        printf("north_angle_str stof invalid_argument.\n");
                    }
                    if(show_level==1 || show_level==2 || show_level==5)
                        update_north_arrow(north_angle, targraphColor.ColorConfig.color.Color_R_Y, targraphColor.ColorConfig.color.Color_G_U, targraphColor.ColorConfig.color.Color_B_V, true);
                    else
                        update_north_arrow(north_angle, targraphColor.ColorConfig.color.Color_R_Y, targraphColor.ColorConfig.color.Color_G_U, targraphColor.ColorConfig.color.Color_B_V, false);
                    GPIO_TAR_Graph_Color_HANDLE[Tar_Graph_Color_Addr_offset >> 2] = targraphColor.ColorConfig.send;
                    GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_MAIN>>2] = cross_Main.cross_info.send;
                }

                if(target_type != HostUARTDevice::instance()->target_type)
                {
                    if(track_command == 1) //捕获状态，直接切换跟踪类型
                    {
                        target_type = HostUARTDevice::instance()->target_type;
                        if(target_type == 1)
                        {
                            printf("Normal track.\n");
                            resend_delay = 5;
                            XJ3UARTDevice::instance()->write_command_control(0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                        }
                        else
                        {
                            printf("Motion track.\n");
                            resend_delay = 8;
                            XJ3UARTDevice::instance()->write_command_control(0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                        }
                        xj3_send_command = true;
                    }
                    else //跟踪状态，需要等初始的多目标提示转跟踪后，再切换跟踪类型
                    {
                        if(count >= (count_track_start + resend_delay))
                        {                    
                            target_type = HostUARTDevice::instance()->target_type;
                            if(target_type == 1)
                            {
                                resend_delay = 5;
                                printf("Normal track.\n");
                                XJ3UARTDevice::instance()->write_command_control(0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                            else
                            {
                                printf("Motion track.\n");
                                resend_delay = 8;
                                XJ3UARTDevice::instance()->write_command_control(0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }  
                            xj3_send_command = true;                  
                        }
                    }
                }

                if(multi_type != HostUARTDevice::instance()->multi_type)
                {
                    if(track_command == 1) //捕获状态
                    {
                        multi_type = HostUARTDevice::instance()->multi_type;
                        if(multi_target_prompt == 1) //多目标提示打开时，关闭多目标提示，切换多目标提示类型，再打开多目标提示
                        {
                            count_multi_type = count;
                            XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF2, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                        }
                        else
                        {
                            if(multi_type == 8)
                            {
                                printf("Inteli detect!\n");
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                            else
                            {
                                printf("Motion detect!\n");
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);  
                            }                       
                        }
                        xj3_send_command = true;
                    }
                    else //跟踪状态，需要等初始的多目标提示转跟踪后，再发多目标提示类型切换
                    {
                        if(count >= (count_track_start + resend_delay))
                        {
                            multi_type = HostUARTDevice::instance()->multi_type;
                            if(multi_type == 8)
                            {
                                printf("Inteli detect!\n");
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                            else
                            {
                                printf("Motion detect!\n");
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF); 
                            }  
                            xj3_send_command = true;                      
                        }
                    }
                }
                if(count == (count_multi_type+2))
                {
                    if(multi_type == 8)
                    {
                        printf("Inteli detect!\n");
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                    }
                    else
                    {
                        printf("Motion detect!\n");
                        XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                    }
                    xj3_send_command = true;
                }
                else if(count == ((count_multi_type+4)))
                {
                    XJ3UARTDevice::instance()->write_command_control(0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                    xj3_send_command = true;
                    count_multi_type = -10;
                }


                if(track_command == 1)
                {
                    if(count >= (count_track_start + resend_delay) && HostUARTDevice::instance()->capture_send)
                    {
                        XJ3UARTDevice::instance()->write_command_control(0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                        xj3_send_command = true;
                        HostUARTDevice::instance()->capture_send = false;
                    }
                    if(work_mode == 0x32)
                    {
                        yaw_v_stable   = 0.95 * yaw_v_stable + 0.05 * HostUARTDevice::instance()->yaw_v;
                        pitch_v_stable = 0.95 * pitch_v_stable + 0.05 * HostUARTDevice::instance()->pitch_v;
                    }
                    // if(HostUARTDevice::instance()->wave_gate_size_changed)
                    // {   
                    //     timer2 = 30;
                    //     HostUARTDevice::instance()->wave_gate_size_changed = false;
                    // }
                    if(timer2 > 0)
                    {
                        TarInfo_8.gate.info.Enable  = true;
                        TarInfo_8.gate.info.IdEn    = false;
                        TarInfo_8.cross.info.Enable = false;
                        TarInfo_8.cross.info.cols_hor_x = cross_x_main; //960;
                        TarInfo_8.cross.info.rows_ver_y = cross_y_main; //540;
                        TarInfo_8.gate.info.width  = std::round(64 * HostUARTDevice::instance()->wave_gate_width_scale);
                        TarInfo_8.gate.info.height = std::round(48 * HostUARTDevice::instance()->wave_gate_width_scale);
                        if(timer2 == 30) 
                        {
                            GPIO_TAR8_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_8.cross.Send;
                            GPIO_TAR8_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_8.gate.Send; 
                        } 
                        timer2--;
                    } 
                    else if(timer2 == 0)
                    {
                        TarInfo_8.gate.info.Enable  = false;
                        TarInfo_8.gate.info.IdEn    = false;
                        TarInfo_8.cross.info.Enable = false;
                        GPIO_TAR8_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_8.cross.Send;
                        GPIO_TAR8_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_8.gate.Send;
                        timer2--;                        
                    }         
                    else
                        timer2 = timer2;
                }
                else if(track_command == 2)
                {
                    if(track_status == 2)
                    {
                        yaw_v_stable   = 0.95 * yaw_v_stable + 0.05 * HostUARTDevice::instance()->yaw_v;
                        pitch_v_stable = 0.95 * pitch_v_stable + 0.05 * HostUARTDevice::instance()->pitch_v;                       
                    }
                } 
                else if(track_command == 3 || track_command == 4)
                {
                    if( count > (count_track_start + 1) )
                    {
                        if(is_tv)
                        {
                            delta_x = std::round( 64.0 * (HostUARTDevice::instance()->yaw_v - yaw_v_stable) / HostUARTDevice::instance()->tv_yaw_view_angle);
                            delta_y = std::round(-64.0 * (HostUARTDevice::instance()->pitch_v - pitch_v_stable) / HostUARTDevice::instance()->tv_yaw_view_angle);
                            if(!xj3_send_command)
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, delta_x, delta_y, 0x00);
                        }
                        else
                        {
                            delta_x = std::round( 10.7 * (HostUARTDevice::instance()->yaw_v - yaw_v_stable) / HostUARTDevice::instance()->ir_yaw_view_angle);
                            delta_y = std::round(-10.7 * (HostUARTDevice::instance()->pitch_v - pitch_v_stable) / HostUARTDevice::instance()->ir_yaw_view_angle);
                            if(!xj3_send_command)
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, delta_x, delta_y, 0x00);
                        }
                    }
                }

                //  static std::string test_info = "";
                //  if(track_command != 1)
                //  {
                //      // test_info = pad_str(format("delta_x:%d, delta_y:%d.\n", delta_x, delta_y), 32, -1);
                //      test_info = pad_str(format("p:%3d,a:%3d,p:%3d,a:%3d,n:%2d,s:%1d\n", XJ3UARTDevice::instance()->prev_peak, XJ3UARTDevice::instance()->prev_apce,
                //      XJ3UARTDevice::instance()->cur_peak, XJ3UARTDevice::instance()->cur_apce, XJ3UARTDevice::instance()->nano_score, XJ3UARTDevice::instance()->strategy_ID), 32, -1);
                //      for(int i=0; i < test_info.size(); i++)
                //      {
                //          osd_pos_39_info[i] = test_info[i];
                //      }
                //      osd_pos_test.str_arr = osd_pos_39_info;
                //      update_OSD_chinese(osd_pos_test, OSD_BRAM_HANDLE);
                //  }
                //  else
                //  {
                //      osd_pos_test.str_arr = osd_blank;
                //      update_OSD_chinese(osd_pos_test, OSD_BRAM_HANDLE);
                //  }

                if(second_capture == 1)
                {
                    if(!HostUARTDevice::instance()->cross_symbol_send)
                    {
                        second_target->gate.info.Enable = false;
                        second_target->gate.info.IdEn   = false;
                        second_target->cross.info.Enable = false;
                        second_target->cross.info.cols_hor_x = 960;
                        second_target->cross.info.rows_ver_y = 540;
                    }
                    else
                    {
                        if(is_tv)
                        {
                            // visible = (HostUARTDevice::instance()->cross_symbol_x > 30) && (HostUARTDevice::instance()->cross_symbol_x < 1890) && (HostUARTDevice::instance()->cross_symbol_y > 30) && (HostUARTDevice::instance()->cross_symbol_y < 1050);
                            if(tv_scale == 1)
                            {
                                cross_symbol_x = HostUARTDevice::instance()->cross_symbol_x * 2 - 960;
                                cross_symbol_y = HostUARTDevice::instance()->cross_symbol_y * 2 - 540;
                            }		
                            else
                            {
                                cross_symbol_x = HostUARTDevice::instance()->cross_symbol_x;
                                cross_symbol_y = HostUARTDevice::instance()->cross_symbol_y;
                            }
                            cross_symbol_x = (cross_symbol_x < 40) ? 40 : ((cross_symbol_x > 1880) ?  1879 : cross_symbol_x);	
                            cross_symbol_y = (cross_symbol_y < 40) ? 40 : ((cross_symbol_y > 1040) ?  1039 : cross_symbol_y);
                        }
                        else
                        {
                            // visible = (HostUARTDevice::instance()->cross_symbol_x > 15) && (HostUARTDevice::instance()->cross_symbol_x < 625) && (HostUARTDevice::instance()->cross_symbol_y > 15) && (HostUARTDevice::instance()->cross_symbol_y < 497);                                                        
                            cross_symbol_x = HostUARTDevice::instance()->cross_symbol_x * 2 + 320;
                            cross_symbol_x = (cross_symbol_x < 360) ? 360 : ((cross_symbol_x > 1560) ?  1560 : cross_symbol_x);
                            cross_symbol_y = HostUARTDevice::instance()->cross_symbol_y * 2 + 28;
                            cross_symbol_y = (cross_symbol_y < 68) ? 68 : ((cross_symbol_y > 1012) ?  1012 : cross_symbol_y);
                        }
                        second_target->tar_idx = 10;
                        second_target->gate.info.Enable  = wave_gate_visible; //visible; //? wave_gate_visible : false;
                        second_target->gate.info.IdEn    = wave_gate_visible; //visible; //? wave_gate_visible : false;
                        second_target->cross.info.Enable = wave_gate_visible; //visible; //? wave_gate_visible : false;
                        
                        second_target->cross.info.cols_hor_x = cross_symbol_x;
                        second_target->cross.info.rows_ver_y = cross_symbol_y;

                        draw_dot_line(cross_y_main, cross_x_main, cross_symbol_y, cross_symbol_x, targraphColor.ColorConfig.color.Color_R_Y, targraphColor.ColorConfig.color.Color_G_U, targraphColor.ColorConfig.color.Color_B_V);                            
                    }
                }
                else
                { 
                    if(HostUARTDevice::instance()->cross_symbol_send)
                        clear_dot_line();
                    second_target->gate.info.Enable = false;
                    second_target->gate.info.IdEn   = false;
                    second_target->cross.info.Enable = false;
                    second_target->cross.info.cols_hor_x = 960;
                    second_target->cross.info.rows_ver_y = 540;
                    HostUARTDevice::instance()->cross_symbol_send = false;
                }
                GPIO_TAR10_XYWH_HANDLE[Tar_Cross_Addr_offset >> 2] = TarInfo_10.cross.Send;
                GPIO_TAR10_XYWH_HANDLE[Tar_Gate_Addr_offset >> 2] = TarInfo_10.gate.Send;
                
                if(tv_enhance_on != HostUARTDevice::instance()->image_enhance_tv)
                {
                    tv_enhance_on = HostUARTDevice::instance()->image_enhance_tv;
                    if(tv_enhance_on == 1)
                        pl_imgproc_ctrl.CTRL_CMD.ImgEnhanceLevel = ENHANCE_LEVEL_50per;
                    else 
                        pl_imgproc_ctrl.CTRL_CMD.ImgEnhanceLevel = ENHANCE_LEVEL_0per;
                    GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2] = pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;
                    XJ3UARTDevice::instance()->write_command_control(0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                }
                if( ir_enhance_on != (HostUARTDevice::instance()->image_enhance_ir == 3))
                {
                    ir_enhance_on = (HostUARTDevice::instance()->image_enhance_ir == 3);
                    ir_enhance_switch_happened = true;  
                    XJ3UARTDevice::instance()->write_command_control(0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);                     
                }
                
                if(sync_422_speed != HostUARTDevice::instance()->compress_ratio)
                {
                    sync_422_speed = HostUARTDevice::instance()->compress_ratio;
                    if(sync_422_speed == 3)
                    {
                        pl_imgproc_ctrl.CTRL_CMD.sync_422_speed = 3;
                        GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2] = pl_imgproc_ctrl.CTRL_CMD.ctrl_bit; asm("nop");
                        printf("sync_422 bandwidth: 7.68MHz.\n"); 
                    }
                    else
                    {
                        pl_imgproc_ctrl.CTRL_CMD.sync_422_speed = 0;
                        GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2] = pl_imgproc_ctrl.CTRL_CMD.ctrl_bit; asm("nop");
                        printf("sync_422 bandwidth: 0.96MHz.\n"); 
                    }
                }

                // sys_init_info display
                if (work_mode != HostUARTDevice::instance()->work_mode)
                {
                    if(work_mode == 0x31 && HostUARTDevice::instance()->work_mode != 0x31 )
                    {   
// #ifdef update_sys_init_debug
                    	init_count = 0;
						for(int i = 0; i < 6; i++)
						{
							osd_pos_init_sys[i].config.para.Enable = 0;
							osd_pos_init_sys[i].str_arr = osd_blank;
							update_OSD_chinese(osd_pos_init_sys[i], OSD_BRAM_HANDLE);
						}
						for(int i = 0; i<6; i++)
						{
							for(int j = 0;j<32;j++)
							{
								sys_init_str_cache[i][j] = 0;
							}
						}
						printf("finished init whole sys\n");
// #endif
                    }
                }
               
                if (work_mode == 0x31)
                {            
                   // while(init_count < HostUARTDevice::instance()->init_status_count)//
                    if(!HostUARTDevice::instance()->init_status_cache.empty())
                    {
                        if(init_count < 6)
                        {
                            osd_pos_init_sys[init_count].config.para.Enable = 1;
                            for(int j = 0; j < 32; j++)
                            {
                                if(j < HostUARTDevice::instance()->init_status_cache.front().size())
                                {
                                    osd_system_init_info[j] = (HostUARTDevice::instance()->init_status_cache.front())[j];
                                	sys_init_str_cache[init_count][j] = osd_system_init_info[j];
                                }
                                else
                                {
                                    osd_system_init_info[j] = 0;
                                	 sys_init_str_cache[init_count][j] = osd_system_init_info[j];
                                }
                            }
                            osd_pos_init_sys[init_count].str_arr = osd_system_init_info;
                            update_OSD_chinese(osd_pos_init_sys[init_count], OSD_BRAM_HANDLE);
                            HostUARTDevice::instance()->init_status_cache.pop();
                        }
                        else
                        {
                            for(int i = 0; i <5 ; i++)
                            {
                            	for(int j = 0; j<32; j++)
                            	{
                                	osd_pos_init_sys[i].str_arr[j]	= sys_init_str_cache[i+1][j];
                                	sys_init_str_cache[i][j] 		= sys_init_str_cache[i+1][j];
                            	}
                            	update_OSD_chinese(osd_pos_init_sys[i], OSD_BRAM_HANDLE);
                            }
                            for(int j = 0; j < 32; j++)
                            {
                                if(j < HostUARTDevice::instance()->init_status_cache.front().size())
                                {
                                    osd_system_init_info[j] = (HostUARTDevice::instance()->init_status_cache).front()[j];
                                	sys_init_str_cache[5][j] = osd_system_init_info[j];
                                }
                                else
                                {
                                    osd_system_init_info[j] = 0;
                                	sys_init_str_cache[5][j] = osd_system_init_info[j];
                                }
                            }
                            osd_pos_init_sys[5].str_arr = osd_system_init_info;
                            update_OSD_chinese(osd_pos_init_sys[5], OSD_BRAM_HANDLE);                           
                            HostUARTDevice::instance()->init_status_cache.pop();
                        }
                        init_count++;
                    }
                	// init_count = HostUARTDevice::instance()->init_status_count;
                	// while(!((HostUARTDevice::instance()->init_status_cache).empty()))
                	// {
                	// 	osd_pos_init_sys[bit_info_cnt].config.para.Enable = 1;
                	// 	for(int i=0; i<31; i++)
                	// 		osd_system_init_info[i] = 0;
                	// 	for(int j=0; j<(HostUARTDevice::instance()->init_status_cache.front()).size(); j++)
					// 	{
					// 		osd_system_init_info[j] = (HostUARTDevice::instance()->init_status_cache.front())[j];
					// 	}
					// 	osd_pos_init_sys[bit_info_cnt].str_arr = osd_system_init_info;
					// 	update_OSD_chinese(osd_pos_init_sys[bit_info_cnt], OSD_BRAM_HANDLE);
					// 	bit_info_cnt = (bit_info_cnt+1)%6;
					// 	(HostUARTDevice::instance()->init_status_cache).pop();
                	// }
                	//printf("init_status_count: %d\n",init_count);
                }

                if(HostUARTDevice::instance()->tv_align)
                {
                    if(tv_align != HostUARTDevice::instance()->tv_align)
                    {
                        count_tv_align = count; 
                        tv_align = HostUARTDevice::instance()->tv_align;
                    }

                    if( count < count_tv_align+180)
                    {
                        int align_x = 960;
                        int align_y = 540;
                        bool res_valid = false;
                        // -----------calculate the image centroid of the input image------------
                        #ifdef TV_ALIGN_ON_FMQL
                            if(video_info_TV_IN.ready_TV_ALIGN == true)
                            {
                                float fly_axis_crc_hor = 960;
                                float fly_axis_crc_ver = 540;
                                // float time_start = omp_get_wtime();

                                fly_axis_crc(res_valid,video_info_TV_IN.img_buf + 2334240,fly_axis_crc_hor,fly_axis_crc_ver);
                                // float time_end = omp_get_wtime();
                                if(res_valid)
                                {
                                    // printf("hor_x: %f ver_y: %f time_cost: %f \n", fly_axis_crc_hor, fly_axis_crc_ver, 100.0*(time_end-time_start));
                                    printf("hor_x: %f ver_y: %f\n", fly_axis_crc_hor, fly_axis_crc_ver);
                                }
                                else
                                {
                                	printf("report default value:%f  %f\n", 960.0, 540.0);
                                }
                                align_x = (int) fly_axis_crc_hor;
                                align_y = (int) fly_axis_crc_ver;
                            }                           
                        #endif
                        // -------------------end of calculation----------------- 
                            if(res_valid==true)
                            {
                                uint8_t params[4] = { uint8_t (align_x), uint8_t (align_x >> 8), uint8_t (align_y), uint8_t (align_y >> 8) };
                                HostUARTDevice::instance()->write_command(0x19, SIZEOF(params), params);
                            }
                    }
                    else	//auto quit //else if(count == (count_tv_align + 180)) //after 6s automaticlly exit tv_align mode.
                    {
                        tv_align = false;
                        HostUARTDevice::instance()->tv_align = false;
                        count_tv_align = -10;
                        uint8_t params[1] = {0x02};
                        HostUARTDevice::instance()->write_command(0x12, SIZEOF(params), params);
                    }                 
                }
                else
                {
                	if(tv_align != HostUARTDevice::instance()->tv_align )
                	{
                		printf("quit tv_align..%d \n",count-count_tv_align);
                        tv_align = false;
                    	count_tv_align = -10;
                	}
                }

                disp_mode = (pic_in_pic == 0x02) ? (is_tv ? TV_MAIN_MODE : IR_MAIN_MODE) : (is_tv ? TV_PIP_MODE : IR_PIP_MODE);
                disp_mode = (HostUARTDevice::instance()->fusion == 0x01) ? FUSION_MODE : disp_mode;
                disp_mode = (HostUARTDevice::instance()->ir_align) ? IR_ALIGN_MODE : disp_mode;
                if(disp_mode == IR_ALIGN_MODE)
                {
                    //if open ir_align.20230906 fyc
                    if(ir_align != HostUARTDevice::instance()->ir_align)
                    {
                        ir_align = HostUARTDevice::instance()->ir_align;   //IR_ALIGN_MODE static_image_pip    
                        disp_ctrl.IR_ALIGN_KEEP = true;               
                        disp_ctrl.PIP_START_X = 1920-disp_ctrl.PIP_WIDTH;
                        disp_ctrl.PIP_START_Y = 0; 
                        offset_x_pip = HostUARTDevice::instance()->cross_x_pip - 960;
                        offset_y_pip = HostUARTDevice::instance()->cross_y_pip - 540; 
                        cross_PIP.cross_info.info.crossEn = 1;  
                        cross_PIP.cross_info.info.cross_hor = 1600 + offset_x_pip;
                        cross_PIP.cross_info.info.cross_ver = 180 + offset_y_pip;
                        GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send;  asm("nop"); 
                        count_ir_align = count;
                        HostUARTDevice::instance()->new_comm_recv = false;
                       // printf("enter IR_align mode\n");
                    }   
                    if(count == (count_ir_align+33))  
                    {
                        if(!HostUARTDevice::instance()->new_comm_recv)  //no command received from host for 1 sec, exit
                        {
                            count_ir_align = -10;
                            ir_align = false;
                            HostUARTDevice::instance()->ir_align = false;    
                            disp_ctrl.IR_ALIGN_KEEP = false; 
                            uint8_t params[1] = {0x02};
                            HostUARTDevice::instance()->write_command(0x11, SIZEOF(params), params);                            
                            printf("quit IR_align mode auto.\n");
                        }
                        else
                        {
                            count_ir_align = count;
                            HostUARTDevice::instance()->new_comm_recv = false;
                        }
                    }                          
                }
                else
                {
                    switch(pic_in_pic)
                    {
                        case 1:
                        {
                            offset_x_pip = HostUARTDevice::instance()->cross_x_pip - 960;
                            offset_y_pip = HostUARTDevice::instance()->cross_y_pip - 540;
                            if(cross_PIP.cross_info.info.crossEn != HostUARTDevice::instance()->cross_pip_en)
                            {
                                cross_PIP.cross_info.info.crossEn = HostUARTDevice::instance()->cross_pip_en;
                                GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send;
                                asm("nop");
                            }

                            switch(HostUARTDevice::instance()->picture_in_picture)
                            {                            
                                case 2:
                                {
                                    pic_in_pic = 2;//HostUARTDevice::instance()->picture_in_picture;
                                }
                                    break;
                                case 1:
                                case 3:
                                {
                                    pic_in_pic = 1;
                                    if(picture_in_picture_position != HostUARTDevice::instance()->picture_in_picture_position)
                                        disp_ctrl.SWITCH_HAPPENED = true;
                                    picture_in_picture_position = HostUARTDevice::instance()->picture_in_picture_position;
                                }
                                    break;
                                case 4:
                                {
#ifdef ENABLE_XML_FILE_SAVED_FUNC
                                    fs.open("/root/pip.xml", cv::FileStorage::WRITE);
                                    fs << "pic_in_pic" << picture_in_picture_position;
                                    fs.release(); 
    #ifdef TEST_SYNC
                                    sync();
    #endif
    #ifndef TEST_SYNC
                                   // system("sync");//2023.02.03 added by fyc---sync
    #endif
                                    pic_in_pic = 1;
    #ifdef DEBUG_PRINTF
                                    printf("pip loc cfg saved..\n");
    #endif
#endif
                                    HostUARTDevice::instance()->picture_in_picture = 1;
                                }
                                    break;
                                default:
                                    pic_in_pic = 1;
                                    break;
                            }

                            switch(picture_in_picture_position)
                            {
                                case 0:
                                {
                                    cross_PIP.cross_info.info.cross_hor = 320 + offset_x_pip;
                                    cross_PIP.cross_info.info.cross_ver = 180 + offset_y_pip;
                                    GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send; 
                                    asm("nop");
                                    disp_ctrl.PIP_START_X = 0;
                                    disp_ctrl.PIP_START_Y = 0;
                                }
                                    break;
                                case 1:
                                {
                                    cross_PIP.cross_info.info.cross_hor = 1600 + offset_x_pip;
                                    cross_PIP.cross_info.info.cross_ver = 180 + offset_y_pip;
                                    GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send; 
                                    asm("nop");
                                    disp_ctrl.PIP_START_X = 1920-disp_ctrl.PIP_WIDTH;
                                    disp_ctrl.PIP_START_Y = 0;
                                }
                                    break;
                                case 2:
                                {
                                    cross_PIP.cross_info.info.cross_hor = 1600 + offset_x_pip;
                                    cross_PIP.cross_info.info.cross_ver = 900 + offset_y_pip;
                                    GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send; 
                                    asm("nop");
                                    disp_ctrl.PIP_START_X = 1920-disp_ctrl.PIP_WIDTH;
                                    disp_ctrl.PIP_START_Y = 720;
                                }
                                    break;
                                case 3:
                                {
                                    cross_PIP.cross_info.info.cross_hor = 320 + offset_x_pip;
                                    cross_PIP.cross_info.info.cross_ver = 900 + offset_y_pip;
                                    GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send; 
                                    asm("nop");                        
                                    disp_ctrl.PIP_START_X = 0;
                                    disp_ctrl.PIP_START_Y = 720;
                                }
                                    break;
                                default:
                                {
                                    cross_PIP.cross_info.info.cross_hor = 320 + offset_x_pip;
                                    cross_PIP.cross_info.info.cross_ver = 180 + offset_y_pip;
                                    GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send; 
                                    asm("nop");
                                    disp_ctrl.PIP_START_X = 0;
                                    disp_ctrl.PIP_START_Y = 0;                            
                                } 
                            }
                        }
                            break;
                        case 2:
                        {
                            if(cross_PIP.cross_info.info.crossEn != 0)
                            {
                                cross_PIP.cross_info.info.crossEn = 0;
                                GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send;
                                asm("nop"); 
                            }
                            if(HostUARTDevice::instance()->picture_in_picture == 1)
                            {
                                pic_in_pic = 1;
                                try
                                {
                                    fs.open("/root/pip.xml", cv::FileStorage::READ);
                                }
                                catch(cv::Exception)
                                {
                                    printf("read pip xml faild..\n");
                                }

                                if(!fs.isOpened())
                                    HostUARTDevice::instance()->picture_in_picture_position = 0;
                                else
                                    fs["pic_in_pic"] >> HostUARTDevice::instance()->picture_in_picture_position;
                            }
                            else 
                                pic_in_pic = 2;
                        }
                            break;
                        default:
                            break;   
                    }
                }

                if ((disp_ctrl.DISP_MODE != disp_mode))
                {
                    //if close ir_algn.20230906 fyc 
                    if((disp_mode != IR_ALIGN_MODE) && (disp_ctrl.DISP_MODE == IR_ALIGN_MODE))
                    {
                        ir_align = false;
                	    disp_ctrl.IR_ALIGN_KEEP = false; 
                    	printf("quit IR_align mode mannual.\n");
                    }                      
                    disp_ctrl.DISP_MODE = disp_mode;
                    disp_ctrl.SWITCH_HAPPENED = true;
                }

                {   
                    if(HostUARTDevice::instance()->work_mode == 0x3d)
                    {
                        if(is_tv)
                        {
                            switch(HostUARTDevice::instance()->view_size)
                            {
                                case 1:
                                {
                                    cross_x_main_tv_l = cross_x_main;
                                    cross_y_main_tv_l = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_tv_l = offset_x_pip;
                                        offset_y_pip_tv_l = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_tv_l.xml", cv::FileStorage::WRITE); 
                                    break;
                                case 2:
                                {
                                    cross_x_main_tv_m = cross_x_main;
                                    cross_y_main_tv_m = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_tv_m = offset_x_pip;
                                        offset_y_pip_tv_m = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_tv_m.xml", cv::FileStorage::WRITE);
                                    break;
                                case 3:
                                {
                                    cross_x_main_tv_s = cross_x_main;
                                    cross_y_main_tv_s = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_tv_s = offset_x_pip;
                                        offset_y_pip_tv_s = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_tv_s.xml", cv::FileStorage::WRITE);
                                    break;
                                case 5:
                                {
                                    cross_x_main_tv_z = cross_x_main;
                                    cross_y_main_tv_z = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_tv_z = offset_x_pip;
                                        offset_y_pip_tv_z = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_tv_z.xml", cv::FileStorage::WRITE);
                                    break;
                                default:
                                    // fs.open("/root/cross_tv_l.xml", cv::FileStorage::WRITE);
                                    break;
                            }
                        }
                        else
                        {
                            switch(HostUARTDevice::instance()->view_size)
                            {
                                case 0x01:
                                {
                                    cross_x_main_ir_l = cross_x_main;
                                    cross_y_main_ir_l = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_ir_l = offset_x_pip;
                                        offset_y_pip_ir_l = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_ir_l.xml", cv::FileStorage::WRITE); 
                                    break;
                                case 0x02:
                                {
                                    cross_x_main_ir_m = cross_x_main;
                                    cross_y_main_ir_m = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_ir_m = offset_x_pip;
                                        offset_y_pip_ir_m = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_ir_m.xml", cv::FileStorage::WRITE);
                                    break;
                                case 0x03:
                                {
                                    cross_x_main_ir_s = cross_x_main;
                                    cross_y_main_ir_s = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_ir_s = offset_x_pip;
                                        offset_y_pip_ir_s = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_ir_s.xml", cv::FileStorage::WRITE);
                                    break;
                                case 0x05:
                                {
                                    cross_x_main_ir_z = cross_x_main;
                                    cross_y_main_ir_z = cross_y_main;
                                    if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                                    {
                                        offset_x_pip_ir_z = offset_x_pip;
                                        offset_y_pip_ir_z = offset_y_pip;
                                    }
                                }
                                    // fs.open("/root/cross_ir_z.xml", cv::FileStorage::WRITE);
                                    break;
                                default:
                                    // fs.open("/root/cross_ir_l.xml", cv::FileStorage::WRITE);
                                    break;
                            }
                        }
                        if(HostUARTDevice::instance()->save_split_position)
                        {
#ifdef ENABLE_XML_FILE_SAVED_FUNC
                            fs.open("/root/cross.xml", cv::FileStorage::WRITE); 
                            fs << "cross_x_main_tv_l" << cross_x_main_tv_l;
                            fs << "cross_y_main_tv_l" << cross_y_main_tv_l;
                            fs << "cross_x_main_tv_m" << cross_x_main_tv_m;
                            fs << "cross_y_main_tv_m" << cross_y_main_tv_m;
                            fs << "cross_x_main_tv_s" << cross_x_main_tv_s;
                            fs << "cross_y_main_tv_s" << cross_y_main_tv_s;
                            fs << "cross_x_main_tv_z" << cross_x_main_tv_z;
                            fs << "cross_y_main_tv_z" << cross_y_main_tv_z;
                            fs << "cross_x_main_ir_l" << cross_x_main_ir_l;
                            fs << "cross_y_main_ir_l" << cross_y_main_ir_l;
                            fs << "cross_x_main_ir_m" << cross_x_main_ir_m;
                            fs << "cross_y_main_ir_m" << cross_y_main_ir_m; 
                            fs << "cross_x_main_ir_s" << cross_x_main_ir_s;
                            fs << "cross_y_main_ir_s" << cross_y_main_ir_s; 
                            fs << "cross_x_main_ir_z" << cross_x_main_ir_z;
                            fs << "cross_y_main_ir_z" << cross_y_main_ir_z;                                                                  
                            // if(disp_mode == TV_PIP_MODE || disp_mode == IR_PIP_MODE)
                            // {
                            fs << "offset_x_pip_tv_l" << offset_x_pip_tv_l;
                            fs << "offset_y_pip_tv_l" << offset_y_pip_tv_l;
                            fs << "offset_x_pip_tv_m" << offset_x_pip_tv_m;
                            fs << "offset_y_pip_tv_m" << offset_y_pip_tv_m;
                            fs << "offset_x_pip_tv_s" << offset_x_pip_tv_s;
                            fs << "offset_y_pip_tv_s" << offset_y_pip_tv_s;
                            fs << "offset_x_pip_tv_z" << offset_x_pip_tv_z;
                            fs << "offset_y_pip_tv_z" << offset_y_pip_tv_z;
                            fs << "offset_x_pip_ir_l" << offset_x_pip_ir_l;
                            fs << "offset_y_pip_ir_l" << offset_y_pip_ir_l;
                            fs << "offset_x_pip_ir_m" << offset_x_pip_ir_m;
                            fs << "offset_y_pip_ir_m" << offset_y_pip_ir_m;
                            fs << "offset_x_pip_ir_s" << offset_x_pip_ir_s;
                            fs << "offset_y_pip_ir_s" << offset_y_pip_ir_s;
                            fs << "offset_x_pip_ir_z" << offset_x_pip_ir_z;
                            fs << "offset_y_pip_ir_z" << offset_y_pip_ir_z;
                            // }
                            fs.release();
                           // system("sync");//2023.02.03 added by fyc---sync
#ifdef DEBUG_PRINTF
                            printf("cross loc cfg saved..\n");
#endif
#endif
                            HostUARTDevice::instance()->save_split_position = false;                        
                        }
                    }
                }

                // show_level
                // show_level=1: enable to show all osd;
                // show_level=2: menu and system_info disable;
                // show level=3: cross,wave_gate
                // show_level=4: all disable
                // show_level=5: menu disbale
                // show_level=6: 
				level_changed = false;
                {
                    if(show_level != HostUARTDevice::instance()->show_level)
                    {
                        level_changed = true;
                        show_level = HostUARTDevice::instance()->show_level;
                    }
                    switch(show_level) 
                    {
                        case 1:
                            cross_Main.cross_info.info.crossEn = 1;
                            break;
                        case 2:
                            cross_Main.cross_info.info.crossEn = 1;
                            break;
                        case 3:
                            cross_Main.cross_info.info.crossEn = 1;
                            break;
                        case 4:
                        {
                            cross_PIP.cross_info.info.crossEn = 0;
                            cross_Main.cross_info.info.crossEn = 0;
                        }
                            break;
                        case 5:
                            cross_Main.cross_info.info.crossEn = 1;
                            break;
                        case 6:
                            cross_Main.cross_info.info.crossEn = 1;
                            break;
                        default:
							break;
                    }
                    if(level_changed || count == 1 )
                    {					                       
                        GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_MAIN>>2] = cross_Main.cross_info.send;
                        GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] = cross_PIP.cross_info.send;                       
                    }
                }				

			if(level_changed || channel_disp != HostUARTDevice::instance()->channel_disp || enhance_level != HostUARTDevice::instance()->enhance_level || sensor_view_size != HostUARTDevice::instance()->sensor_view_size || ir_power != HostUARTDevice::instance()->ir_power
			|| laser_power != HostUARTDevice::instance()->laser_power || brigheness_contrast_modify != HostUARTDevice::instance()->brigheness_contrast_modify || ir_pola != HostUARTDevice::instance()->ir_pola || yaw_view_angle_str.compare(HostUARTDevice::instance()->yaw_view_angle_str) != 0)
			{
				// command ID 0x02 HostUARTDevice::instance()->channel device.cpp case 0x02   1
				channel_disp = HostUARTDevice::instance()->channel_disp;
				if (channel_disp == 1)
				{
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[0] = CH_idx_dian_312_1;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[1] = CH_idx_dian_313_2;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[2] = CH_idx_shi_840_1;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[3] = CH_idx_shi_841_2;
				}
				else if (channel_disp == 2)
				{
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[0] = CH_idx_hong_468_1;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[1] = CH_idx_hong_469_2;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[2] = CH_idx_wai_926_1;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[3] = CH_idx_wai_927_2;
				}
                else if (channel_disp == 3)
                {
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[0] = CH_idx_guang_432_1;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[1] = CH_idx_guang_433_2;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[2] = CH_idx_ban_1256_1;
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[3] = CH_idx_ban_1257_2; 
                    // osd_sensor_viewangle_enhance_viewstate_bricont_ir[4] = CH_idx_guan_426_1;
					// osd_sensor_viewangle_enhance_viewstate_bricont_ir[5] = CH_idx_guan_427_2;  
                    // osd_sensor_viewangle_enhance_viewstate_bricont_ir[6] = CH_idx_ce_210_1;
					// osd_sensor_viewangle_enhance_viewstate_bricont_ir[7] = CH_idx_ce_211_2;                   
                }
				// command ID 0x26   HostUARTDevice::instance()->yaw_view_angle_str    2
				yaw_view_angle_str = HostUARTDevice::instance()->yaw_view_angle_str;
				for(int i = 0; i<6; i++)
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[i+5] = (HostUARTDevice::instance()->yaw_view_angle_str)[i];
				//command ID 0x21 location ID 0x03  HostUARTDevice::instance()->enhance_level   3
                if(enhance_level != HostUARTDevice::instance()->enhance_level)
				{
                    enhance_level = HostUARTDevice::instance()->enhance_level;
                    enhance_level_changed = true;
                }
				switch (enhance_level)
				{
					case '0':
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[14] = ' ';
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[15] = ' ';
					}
					break;
					case '1':
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[14] = CH_idx_qing_1250_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[15] = CH_idx_qing_1251_2;
					}
					break;
					case '2':
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[14] = CH_idx_zhong_1192_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[15] = CH_idx_zhong_1193_2;
					}
					break;
					case '3':
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[14] = CH_idx_zhong_1198_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[15] = CH_idx_zhong_1199_2;
					}
					break;
					default:
						break;
				}
				//ID 0x17   HostUARTDevice::instance()->sensor_view_size     4
                sensor_view_size = HostUARTDevice::instance()->sensor_view_size;
				switch (sensor_view_size)
				{
					case 1:
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[18] = CH_idx_da_276_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[19] = CH_idx_da_277_2;
					}
					break;
					case 2:
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[18] = CH_idx_zhong_1192_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[19] = CH_idx_zhong_1193_2;
					}
					break;
					case 3:
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[18] = CH_idx_xiao_1002_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[19] = CH_idx_xiao_1003_2;
					}
					break;
					case 4:
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[18] = CH_idx_chao_228_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[19] = CH_idx_chao_229_2;
					}
                    break;
					case 5:
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[18] = 'X';
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[19] = '2';
					}
					break;
					default:
						break;
				}
				//command ID 0x21  location ID 0x05   HostUARTDevice::instance()->brigheness_contrast_modify    5
				brigheness_contrast_modify = HostUARTDevice::instance()->brigheness_contrast_modify;
				switch (brigheness_contrast_modify)
				{
					case '0':
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[22] = CH_idx_shou_848_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[23] = CH_idx_shou_849_2;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[24] = CH_idx_dong_326_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[25] = CH_idx_dong_327_2;
					}
					break;
					case '1':
					{
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[22] = CH_idx_zi_1220_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[23] = CH_idx_zi_1221_2;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[24] = CH_idx_dong_326_1;
						osd_sensor_viewangle_enhance_viewstate_bricont_ir[25] = CH_idx_dong_327_2;
					}
					break;
					default:
						break;
				}
				// ID 0x19   HostUARTDevice::instance()->ir_pola   6
				if(ir_pola != HostUARTDevice::instance()->ir_pola)
                {
                    ir_pola = HostUARTDevice::instance()->ir_pola;
                    XJ3UARTDevice::instance()->write_command_control(0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF); 
                }
				if(HostUARTDevice::instance()->ir_power && HostUARTDevice::instance()->channel == 2)
				{
					switch (ir_pola)
					{
						case 0x05:
						{
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[28] = CH_idx_hei_464_1;
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[29] = CH_idx_hei_465_2;
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[30] = CH_idx_re_778_1;
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[31] = CH_idx_re_779_2;
						}
						break;
						case 0x06:
						{
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[28] = CH_idx_bai_132_1;
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[29] = CH_idx_bai_133_2;
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[30] = CH_idx_re_778_1;
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[31] = CH_idx_re_779_2;
						}
						break;
						default:
						{
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[28] = ' ';
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[29] = ' ';
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[30] = ' ';
							osd_sensor_viewangle_enhance_viewstate_bricont_ir[31] = ' ';
						}
						break;
					}
				}
				else
				{
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[28] = ' ';
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[29] = ' ';
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[30] = ' ';
					osd_sensor_viewangle_enhance_viewstate_bricont_ir[31] = ' ';
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.config.para.Enable = 1;
				}
				else
				{
					osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.config.para.Enable = 0;
				}
				osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.str_arr = osd_sensor_viewangle_enhance_viewstate_bricont_ir;
				update_OSD_chinese(osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir, OSD_BRAM_HANDLE);
            }

			/* inertia state, velocity compensation, work mode */
			static uint8_t inertial_state = '3';
			static uint8_t vel_comp = 3;
			// static uint8_t work_mode = 0xff;
			if(level_changed || inertial_state != HostUARTDevice::instance()->inertial_state || vel_comp != HostUARTDevice::instance()->vel_comp || work_mode != HostUARTDevice::instance()->work_mode)
			{
				// command ID 0x21  location ID 0x06  inertia state  HostUARTDevice::instance()->inertial_state
				inertial_state = HostUARTDevice::instance()->inertial_state;
				if (inertial_state == '0' || show_level == 6)
				{
					osd_inertia_velocitycompensation_workmode[0] = ' ';
					osd_inertia_velocitycompensation_workmode[1] = ' ';
					osd_inertia_velocitycompensation_workmode[2] = ' ';
					osd_inertia_velocitycompensation_workmode[3] = ' ';
				}
				else if (inertial_state == '1')
				{
					osd_inertia_velocitycompensation_workmode[0] = CH_idx_you_1100_1;
					osd_inertia_velocitycompensation_workmode[1] = CH_idx_you_1101_2;
					osd_inertia_velocitycompensation_workmode[2] = CH_idx_xiao_1006_1;
					osd_inertia_velocitycompensation_workmode[3] = CH_idx_xiao_1007_2;
				}
				// command ID 0x1b  velocity compensation  HostUARTDevice::instance()->vel_comp
				vel_comp = HostUARTDevice::instance()->vel_comp;
				if (vel_comp == 1)
				{
					osd_inertia_velocitycompensation_workmode[8] = 'L';
					osd_inertia_velocitycompensation_workmode[9] = 'M';
					osd_inertia_velocitycompensation_workmode[10] = 'C';
				}
				else if (vel_comp == 2)
				{
					osd_inertia_velocitycompensation_workmode[8] = ' ';
					osd_inertia_velocitycompensation_workmode[9] = ' ';
					osd_inertia_velocitycompensation_workmode[10] = ' ';
				}
				// command ID 0x16  work mode  HostUARTDevice::instance()->work_mode
				work_mode = HostUARTDevice::instance()->work_mode;
				switch (work_mode)
				{
					case 0x31:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_chu_244_1;//CH_idx_qi_742_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_chu_245_2;//CH_idx_qi_743_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_shi_832_1;//CH_idx_dong_326_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_shi_833_2;//CH_idx_dong_327_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
					}
					break;
					case 0x32:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_shou_848_1;//CH_idx_guan_430_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_shou_849_2;//CH_idx_guan_431_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_dong_326_1;//CH_idx_xing_1026_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_dong_327_2;//CH_idx_xing_1027_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
					}
					break;
					case 0x33:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_hang_450_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_hang_451_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_ce_210_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_ce_211_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
					}
					break;
					case 0x34:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_hui_490_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_hui_491_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_shou_846_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_shou_847_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
					}
					break;
					case 0x35:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_zi_1220_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_zi_1221_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_dong_326_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_dong_327_2;
						osd_inertia_velocitycompensation_workmode[18] = CH_idx_sao_802_1;
						osd_inertia_velocitycompensation_workmode[19] = CH_idx_sao_803_2;
						osd_inertia_velocitycompensation_workmode[20] = CH_idx_miao_686_1;
						osd_inertia_velocitycompensation_workmode[21] = CH_idx_miao_687_2;
					}
					break;
					case 0x03:
					case 0x36:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_suo_886_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_suo_887_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_ding_320_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_ding_321_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
					}
					break;
					case 0x37:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_tu_918_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_tu_919_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_xiang_998_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_xiang_999_2;
						osd_inertia_velocitycompensation_workmode[18] = CH_idx_gen_406_1;
						osd_inertia_velocitycompensation_workmode[19] = CH_idx_gen_407_2;
						osd_inertia_velocitycompensation_workmode[20] = CH_idx_zong_1224_1;
						osd_inertia_velocitycompensation_workmode[21] = CH_idx_zong_1225_2;
					}
					break;
					case 0x38:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_gen_406_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_gen_407_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_sou_1252_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_sou_1253_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
					}
					break;
					case 0x39:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_sou_1252_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_sou_1253_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_suo_1254_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_suo_1255_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
					}
					break;
					case 0x3a:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_suo_886_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_suo_887_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_ding_320_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_ding_321_2;
						osd_inertia_velocitycompensation_workmode[18] = CH_idx_dang_286_1;
						osd_inertia_velocitycompensation_workmode[19] = CH_idx_dang_287_2;
						osd_inertia_velocitycompensation_workmode[20] = CH_idx_qian_750_1;
						osd_inertia_velocitycompensation_workmode[21] = CH_idx_qian_751_2;
					}
					break;
					case 0x3b:
					{
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_di_298_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_di_299_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_li_632_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_li_633_2;
						osd_inertia_velocitycompensation_workmode[18] = CH_idx_gen_406_1;
						osd_inertia_velocitycompensation_workmode[19] = CH_idx_gen_407_2;
						osd_inertia_velocitycompensation_workmode[20] = CH_idx_zong_1224_1;
						osd_inertia_velocitycompensation_workmode[21] = CH_idx_zong_1225_2;
					}
					break;
                    case 0x3c:
                    {
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_sui_882_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_sui_883_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_dong_326_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_dong_327_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
                    }
                        break;
                    case 0x0b:
                    {
						osd_inertia_velocitycompensation_workmode[14] = CH_idx_qian_750_1;
						osd_inertia_velocitycompensation_workmode[15] = CH_idx_qian_751_2;
						osd_inertia_velocitycompensation_workmode[16] = CH_idx_shi_840_1;
						osd_inertia_velocitycompensation_workmode[17] = CH_idx_shi_841_2;
						osd_inertia_velocitycompensation_workmode[18] = ' ';
						osd_inertia_velocitycompensation_workmode[19] = ' ';
						osd_inertia_velocitycompensation_workmode[20] = ' ';
						osd_inertia_velocitycompensation_workmode[21] = ' ';
                    }
                        break;
					default:
						break;
				}
				if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
				{
					osd_pos_inertia_velocitycompensation_workmode.config.para.Enable = 1;
				}
				else
				{
					osd_pos_inertia_velocitycompensation_workmode.config.para.Enable = 0;
				}
				osd_pos_inertia_velocitycompensation_workmode.str_arr = osd_inertia_velocitycompensation_workmode;
				update_OSD_chinese(osd_pos_inertia_velocitycompensation_workmode, OSD_BRAM_HANDLE);
            }

			// command ID 0x23
			static std::string yaw_str = "  ";
			static std::string yaw_v_str = "  ";
			if(level_changed || yaw_str.compare(HostUARTDevice::instance()->yaw_str) != 0 || yaw_v_str.compare(HostUARTDevice::instance()->yaw_v_str) != 0)
			{
				yaw_str = HostUARTDevice::instance()->yaw_str;
				yaw_v_str = HostUARTDevice::instance()->yaw_v_str;
				for(int i = 0; i<(HostUARTDevice::instance()->yaw_str).size(); i++)
				{
					osd_yaw[i+5] = (HostUARTDevice::instance()->yaw_str)[i];
				}

                for(int i = 0; i<10; i++)
                {
                    if(show_level == 6)
                        osd_yaw[i+14] = osd_blank[i];   
                    else
                    {
                        if(i<(HostUARTDevice::instance()->yaw_v_str).size())
                            osd_yaw[i+14] = (HostUARTDevice::instance()->yaw_v_str)[i];
                        else if(i == 7)
                            osd_yaw[i+14] = DEGREE_SYMBOL_ASCII;
                        else if(i == 8)
                            osd_yaw[i+14] = '/';
                        else if(i == 9)
                            osd_yaw[i+14] = 's';
                    }
                }               

				if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
				{
					osd_pos_yaw.config.para.Enable = 1;
				}
				else
				{
					osd_pos_yaw.config.para.Enable = 0;
				}
				osd_pos_yaw.str_arr = osd_yaw;
				update_OSD_chinese(osd_pos_yaw, OSD_BRAM_HANDLE);
            }

			// command ID 0x25
			static std::string pitch_str = "  ";
			static std::string pitch_v_str = "  ";
			if(level_changed || pitch_str.compare(HostUARTDevice::instance()->pitch_str) != 0 || pitch_v_str.compare(HostUARTDevice::instance()->pitch_v_str) != 0)
			{
				pitch_str = HostUARTDevice::instance()->pitch_str;
				pitch_v_str = HostUARTDevice::instance()->pitch_v_str;
				for(int i = 0; i<(HostUARTDevice::instance()->pitch_str).size(); i++)
				{
					osd_pitch[i+5] = (HostUARTDevice::instance()->pitch_str)[i];
				}

                for(int i = 0; i<10; i++)
                {
                    if(show_level == 6)
                        osd_pitch[i+14] = osd_blank[i];
                    else
                    {
                        if(i<(HostUARTDevice::instance()->pitch_v_str).size())
                            osd_pitch[i+14] = (HostUARTDevice::instance()->pitch_v_str)[i];
                        else if(i == 7)
                            osd_pitch[i+14] = DEGREE_SYMBOL_ASCII;
                        else if(i == 8)
                            osd_pitch[i+14] = '/';
                        else if(i == 9)
                            osd_pitch[i+14] = 's';
                    }
                }

				if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
				{
					osd_pos_pitch.config.para.Enable = 1;
				}
				else
				{
					osd_pos_pitch.config.para.Enable = 0;
				}
				osd_pos_pitch.str_arr = osd_pitch;
				update_OSD_chinese(osd_pos_pitch, OSD_BRAM_HANDLE);
			}

			// command ID 0x45  focus value  HostUARTDevice::instance()->focus_str
			// 借用槽位7（惯性/工作模式）的[23]~[27]格，[0]~[21]由工作模式逻辑使用，[22]留空隔开
			static std::string focus_str = "  ";
			if(focus_str.compare(HostUARTDevice::instance()->focus_str) != 0)
			{
				focus_str = HostUARTDevice::instance()->focus_str;
				for(int i = 0; i < 5; i++)
				{
					osd_inertia_velocitycompensation_workmode[23 + i] = (i < (int)focus_str.size()) ? focus_str[i] : 0;
				}
				osd_pos_inertia_velocitycompensation_workmode.str_arr = osd_inertia_velocitycompensation_workmode;
				update_OSD_chinese(osd_pos_inertia_velocitycompensation_workmode, OSD_BRAM_HANDLE);
			}

			// command ID 0x21 location ID 0x01 date  HostUARTDevice::instance()->date_str
			static std::string data_str = "  ";
			if(level_changed || data_str.compare(HostUARTDevice::instance()->date_str) != 0)
			{
				data_str = HostUARTDevice::instance()->date_str;
				for(int i = 0; i<(HostUARTDevice::instance()->date_str).size(); i++)
				{
					osd_date[i] = (HostUARTDevice::instance()->date_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
				{
					osd_pos_date.config.para.Enable = 1;
				}
				else
				{
					osd_pos_date.config.para.Enable = 0;
				}
				osd_pos_date.str_arr = osd_date;
				update_OSD_chinese(osd_pos_date, OSD_BRAM_HANDLE);
			}

			// command ID 0x21  location ID 0x02 HostUARTDevice::instance()->time_str
			static std::string time_str = "  ";
			if(level_changed || time_str.compare(HostUARTDevice::instance()->time_str) != 0)
			{
				time_str = HostUARTDevice::instance()->time_str;
				for(int i = 0; i<(HostUARTDevice::instance()->time_str).size(); i++)
				{
					osd_time[i] = (HostUARTDevice::instance()->time_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
				{
					osd_pos_time.config.para.Enable = 1;
				}
				else
				{
					osd_pos_time.config.para.Enable = 0;
				}
				osd_pos_time.str_arr = osd_time;
				update_OSD_chinese(osd_pos_time, OSD_BRAM_HANDLE);
			}


			//ID 0x19 ir power  HostUARTDevice::instance()->ir_power
			if(level_changed || ir_power != HostUARTDevice::instance()->ir_power)
			{
                if(ir_power && !HostUARTDevice::instance()->ir_power)
                {
                	count_clear_ir_buffer = -9;
#ifdef	IR_CLOSE_VDMA_S2MM_20240125
					VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
					VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
#endif
                	printf("start clear ir buf & close S2MM\n");

                }


				ir_power = HostUARTDevice::instance()->ir_power;
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_ir_power.config.para.Enable = 1;
				}
				else
				{
					osd_pos_ir_power.config.para.Enable = 0;
				}
				if (ir_power)
				{
					osd_pos_ir_power.str_arr = osd_iropen;
					update_OSD_chinese(osd_pos_ir_power, OSD_BRAM_HANDLE);
					printf("avt recv ir power on\n");
				}
				else
				{
					osd_pos_ir_power.str_arr = osd_blank;
					update_OSD_chinese(osd_pos_ir_power, OSD_BRAM_HANDLE);
#ifdef	IR_CLOSE_VDMA_S2MM_20240125
//	    			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
//	    			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000002;					asm("nop");
//	    			ir_power_on_stat = false;
//	    			cur_ir_s2mm_stat = false;
//	    			ir_power_on_cnt = 0;
//	    			printf("avt recv ir power off, close ir s2MM\n");
	    			printf("avt recv ir power off\n");
#else
					printf("avt recv ir power off\n");
#endif
				}
			}
            if(!ir_power)
            {
            	if(count_clear_ir_buffer>=-9 && count_clear_ir_buffer<=0)
            	{
            		count_clear_ir_buffer = count_clear_ir_buffer+1;
            		//printf("wait count: %d\n",count_clear_ir_buffer);
            	}

                switch(count_clear_ir_buffer)
                {
                case 1:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_1[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_1[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_1[r*5760 + c*3+2] = 16;
                            }
                        }
    //                    printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer++;
                    }
                        break;
                case 2:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_2[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_2[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_2[r*5760 + c*3+2] = 16;
                            }
                        }
    //                    printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer++;                   
                    }
                        break;
                case 3:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_3[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_3[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_3[r*5760 + c*3+2] = 16;
                            }
                        }
    //                    printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer++;                   
                    }
                        break;
                case 4:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_4[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_4[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_4[r*5760 + c*3+2] = 16;
                            }
                        }
    //                    printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer++;                   
                    }
                        break;
                case 5:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_5[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_5[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_5[r*5760 + c*3+2] = 16;
                            }
                        }
    //                    printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer++;                   
                    }
                        break;
                case 6:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_6[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_6[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_6[r*5760 + c*3+2] = 16;
                            }
                        }
    //                    printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer++;                   
                    }
                        break;
                case 7:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_7[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_7[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_7[r*5760 + c*3+2] = 16;
                            }
                        }
    //                    printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer++;                   
                    }
                        break;
                case 8:
                    {
                        for(int r = 0; r<512; r++)
                        {
                            for(int c = 0; c<640; c++)
                            {
                                pChar_VDMA_IMG_BUF_IR_8[r*5760 + c*3+0] = 128;
                                pChar_VDMA_IMG_BUF_IR_8[r*5760 + c*3+1] = 128;
                                pChar_VDMA_IMG_BUF_IR_8[r*5760 + c*3+2] = 16;
                            }
                        }
                        printf("clear ir buf %d\n",count_clear_ir_buffer);
                        count_clear_ir_buffer = -30;
                    }
                        break;
                default:
                    break;
                }
            }
            static std::string pos_22_info = "";
			if(level_changed || pos_22_info.compare(HostUARTDevice::instance()->position_22_str) != 0)  
            {
                pos_22_info = HostUARTDevice::instance()->position_22_str;
				for (int i = 0; i < (HostUARTDevice::instance()->position_22_str).size(); i++)
				{
					osd_pos_22_info[i] = (HostUARTDevice::instance()->position_22_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_22.config.para.Enable = 1;
				}
				else
				{
					osd_pos_22.config.para.Enable = 0;
				}
				osd_pos_22.str_arr = osd_pos_22_info;
				update_OSD_chinese(osd_pos_22, OSD_BRAM_HANDLE);                
            }  
            // command ID:0x21 location ID:0x04   image enhancement HostUARTDevice::instance()->image_enhancement
			static uint8_t image_enhancement = '2';
			if(level_changed || image_enhancement != HostUARTDevice::instance()->image_enhancement)
			{
				image_enhancement = HostUARTDevice::instance()->image_enhancement;
				switch (image_enhancement)
				{
					case '0':
					{
						osd_pos_image_enhancement.config.para.Enable = 0;
						osd_pos_image_enhancement.str_arr = osd_blank;
						update_OSD_chinese(osd_pos_image_enhancement, OSD_BRAM_HANDLE);
					}
					break;
					case '1':
					{
						if(show_level==1 || show_level==2 || show_level==5)
						{
							osd_pos_image_enhancement.config.para.Enable = 1;
						}
						else
						{
							osd_pos_image_enhancement.config.para.Enable = 0;
						}
						osd_pos_image_enhancement.str_arr = osd_imageenhance;
						update_OSD_chinese(osd_pos_image_enhancement, OSD_BRAM_HANDLE);
					}
					break;
				}
			}

			// command ID 0x21  location ID 0x10 object style  HostUARTDevice::instance()->target_style
			static uint8_t target_style = '2';
			if(level_changed || target_style != HostUARTDevice::instance()->target_style)
			{
				target_style = HostUARTDevice::instance()->target_style;
				switch (target_style)
				{
					case '0':
					{
						osd_pos_moving_target.config.para.Enable = 0;
						osd_pos_moving_target.str_arr = osd_blank;
						update_OSD_chinese(osd_pos_moving_target, OSD_BRAM_HANDLE);
					}
					break;
					case '1':
					{
						if(show_level==1 || show_level==2 || show_level==5)
						{
							osd_pos_moving_target.config.para.Enable = 1;
						}
						else
						{
							osd_pos_moving_target.config.para.Enable = 0;
						}
						osd_pos_moving_target.str_arr = osd_movingtarget;
						update_OSD_chinese(osd_pos_moving_target, OSD_BRAM_HANDLE);
					}
					break;
				}
			}

			// command ID 0x21  location ID 0x11 parameter collect HostUARTDevice::instance()->para_collect
			static uint8_t para_collect = '2';
			if(level_changed || para_collect != HostUARTDevice::instance()->para_collect)
			{
				para_collect = HostUARTDevice::instance()->para_collect;
				switch (para_collect)
				{
					case '0':
					{
						osd_pos_data_collect.config.para.Enable = 0;
						osd_pos_data_collect.str_arr = osd_blank;
						update_OSD_chinese(osd_pos_data_collect, OSD_BRAM_HANDLE);
					}
					break;
					case '1':
					{
						if(show_level==1 || show_level==2 || show_level==5)
						{
							osd_pos_data_collect.config.para.Enable = 1;
						}
						else
						{
							osd_pos_data_collect.config.para.Enable = 0;
						}
						osd_pos_data_collect.str_arr = osd_datacollect;
						update_OSD_chinese(osd_pos_data_collect, OSD_BRAM_HANDLE);
					}
					break;
				}
			}

			// command ID 0x21    location ID 0x07 fault code  HostUARTDevice::instance()->fault_code_str
			static std::string fault_code_str = "---";
			if(level_changed || fault_code_str.compare(HostUARTDevice::instance()->fault_code_str) != 0)
			{
				fault_code_str = HostUARTDevice::instance()->fault_code_str;
				for(int i = 0; i<(HostUARTDevice::instance()->fault_code_str).size(); i++)
				{
					osd_faultcode_HEX[i] = (HostUARTDevice::instance()->fault_code_str)[i];
				}
				if (osd_faultcode_HEX[0] == '0' && osd_faultcode_HEX[1] == '0')
				{
					osd_pos_fault_code_1.config.para.Enable = 0;
					update_OSD_chinese(osd_pos_fault_code_1, OSD_BRAM_HANDLE);
					osd_pos_fault_code_2.config.para.Enable = 0;
					update_OSD_chinese(osd_pos_fault_code_2, OSD_BRAM_HANDLE);
				}
				else
				{
					if(show_level==1 || show_level==2 || show_level==5)
					{
						osd_pos_fault_code_1.config.para.Enable = 1;
						osd_pos_fault_code_2.config.para.Enable = 1;
					}
					else
					{
						osd_pos_fault_code_1.config.para.Enable = 0;
						osd_pos_fault_code_2.config.para.Enable = 0;
					}
					osd_pos_fault_code_1.str_arr = osd_faultcode;
					update_OSD_chinese(osd_pos_fault_code_1, OSD_BRAM_HANDLE);
					osd_pos_fault_code_2.str_arr = osd_faultcode_HEX;
					update_OSD_chinese(osd_pos_fault_code_2, OSD_BRAM_HANDLE);
				}
			}

			// command ID 0x1a 0x24  laser work mode   HostUARTDevice::instance()->laser_ranging_distance_str
			uint8_t laser_mode = HostUARTDevice::instance()->laser_mode;
			if(laser_power != HostUARTDevice::instance()->laser_power)
				laser_power = HostUARTDevice::instance()->laser_power;
				for(int i = 0; i<(HostUARTDevice::instance()->laser_ranging_distance_str).size(); i++)
				{
					osd_laser_work_mode[i+12] = (HostUARTDevice::instance()->laser_ranging_distance_str)[i];
					osd_laser_work_five_seconds_stop[i+12] = (HostUARTDevice::instance()->laser_ranging_distance_str)[i];  // 对齐下标11
				}
			if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
			{
				osd_pos_laser_work_mode.config.para.Enable = 1;
			}
			else
			{
				osd_pos_laser_work_mode.config.para.Enable = 0;
			}
            if(laser_power)
            {
                switch (laser_mode)
                {
                    case 0x07:
                        // 收到新测距，清零延迟逻辑
                        laser_stop_time = -1;
                        laser_five_sec_showing = false;
                        laser_had_worked = true;  // 标记曾经工作过
                        if (get_wall_time() - HostUARTDevice::instance()->laser_work_start > HostUARTDevice::instance()->laser_work_time + 1)
                        {
                            HostUARTDevice::instance()->laser_work_time += 1;
                            HostUARTDevice::instance()->laser_work_time_str = format("%ds", (int)(HostUARTDevice::instance()->laser_work_time));
                            HostUARTDevice::instance()->laser_work_time_str = pad_str(HostUARTDevice::instance()->laser_work_time_str, 4, -1);
                            osd_laser_work_mode[6] = (HostUARTDevice::instance()->laser_work_time_str)[0];
                            osd_laser_work_mode[7] = (HostUARTDevice::instance()->laser_work_time_str)[1];
                            osd_laser_work_mode[8] = (HostUARTDevice::instance()->laser_work_time_str)[2];
                            osd_laser_work_mode[9] = (HostUARTDevice::instance()->laser_work_time_str)[3];
                        }
                        osd_laser_work_mode[0] = CH_idx_ce_210_1;
                        osd_laser_work_mode[1] = CH_idx_ce_211_2;
                        osd_laser_work_mode[2] = CH_idx_ju_584_1;
                        osd_laser_work_mode[3] = CH_idx_ju_585_2;
                        // 闪烁：以0.5秒为周期，显示'﹡'或空格
                        osd_laser_work_mode[19] = (fmod(get_wall_time(), 1.0) < 0.5) ? '*' : ' ';
                        osd_pos_laser_work_mode.str_arr = osd_laser_work_mode;
                        update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);
                        break;

                    case 0x08:
                        // 收到新照射，清零延迟逻辑
                        laser_stop_time = -1;
                        laser_five_sec_showing = false;
                        laser_had_worked = true;  // 标记曾经工作过
                        if (get_wall_time() - HostUARTDevice::instance()->laser_work_start > HostUARTDevice::instance()->laser_work_time + 1)
                        {
                            HostUARTDevice::instance()->laser_work_time += 1;
                            HostUARTDevice::instance()->laser_work_time_str = format("%ds", (int)(HostUARTDevice::instance()->laser_work_time));
                            HostUARTDevice::instance()->laser_work_time_str = pad_str(HostUARTDevice::instance()->laser_work_time_str, 4, -1);
                            osd_laser_work_mode[6] = (HostUARTDevice::instance()->laser_work_time_str)[0];
                            osd_laser_work_mode[7] = (HostUARTDevice::instance()->laser_work_time_str)[1];
                            osd_laser_work_mode[8] = (HostUARTDevice::instance()->laser_work_time_str)[2];
                            osd_laser_work_mode[9] = (HostUARTDevice::instance()->laser_work_time_str)[3];
                        }
                        osd_laser_work_mode[0] = CH_idx_zhao_1154_1;
                        osd_laser_work_mode[1] = CH_idx_zhao_1155_2;
                        osd_laser_work_mode[2] = CH_idx_she_814_1;
                        osd_laser_work_mode[3] = CH_idx_she_815_2;
                        // 闪烁：以0.5秒为周期，显示'★'或空格
                        osd_laser_work_mode[19] = (fmod(get_wall_time(), 1.0) < 0.5) ? ASCII_28_PENTA_STAR : ' ';
                        osd_pos_laser_work_mode.str_arr = osd_laser_work_mode;
                        update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);
                        break;

                    case 0x09:
                    default:
                        if(!laser_had_worked)
                        {
                            // 从未工作过，直接显示激光准备，不计时
                            osd_pos_laser_work_mode.str_arr = osd_laser_work_mode_stop;
                            update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);
                            break;
                        }
                        if(laser_stop_time < 0)
                        {
                            // 刚切到停止，记录时间，开始5秒倒计时
                            laser_stop_time = get_wall_time();
                            laser_five_sec_showing = true;
//                            printf("[OSD] case 0x09 开始计时: laser_had_worked=%d laser_stop_time=%.2f laser_five_sec_showing=%d\n", (int)laser_had_worked, laser_stop_time, (int)laser_five_sec_showing);

                        }

                        if(laser_five_sec_showing)
                        {
                            if(get_wall_time() - laser_stop_time < 5.0)
                            {
                                // 5秒内：显示激光准备+距离
                                osd_pos_laser_work_mode.str_arr = osd_laser_work_five_seconds_stop;
                                update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);
                            }
                            else
                            {
                                // 5秒到：消隐，停止更新
                                laser_five_sec_showing = false;
                                laser_had_worked = false;  // 5秒结束，重置，回到初始状态
//                                printf("[OSD] case 0x09 初始状态(未曾工作) → osd_laser_work_mode_stop\n");
                                osd_pos_laser_work_mode.str_arr = osd_laser_work_mode_stop;
                                update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);
                            }
                        }
                        break;
                // case 0x20:
                //     osd_pos_laser_work_mode.str_arr = osd_blank;
                //     update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);
                //     break;
                }
            }
            else
            {
                // 激光关闭，同时清零延迟状态
                laser_stop_time = -1;
                laser_five_sec_showing = false;
                laser_had_worked = false;  // 激光关闭，完全重置
                osd_pos_laser_work_mode.str_arr = osd_laserclose;
                update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);
            }

			// command ID 0x05   deorbitting time  HostUARTDevice::instance()->weapon_work_time
			if (HostUARTDevice::instance()->weapon_activate)
			{
				if (get_wall_time() - HostUARTDevice::instance()->weapon_work_start > HostUARTDevice::instance()->weapon_work_time + 1)
				{
					HostUARTDevice::instance()->weapon_work_time += 1;
					HostUARTDevice::instance()->weapon_work_time_str = format("%ds", (int)(HostUARTDevice::instance()->weapon_work_time));
					HostUARTDevice::instance()->weapon_work_time_str = pad_str(HostUARTDevice::instance()->weapon_work_time_str, 3, -1);
					for(int i = 0; i<(HostUARTDevice::instance()->weapon_work_time_str).size(); i++)
					{
						osd_deorbitingtime[i+10] = (HostUARTDevice::instance()->weapon_work_time_str)[i];
					}
				}
				if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
				{
					osd_pos_deorbit_time.config.para.Enable = 1;
				}
				else
				{
					osd_pos_deorbit_time.config.para.Enable = 0;
				}
				osd_pos_deorbit_time.str_arr = osd_deorbitingtime;
				update_OSD_chinese(osd_pos_deorbit_time, OSD_BRAM_HANDLE);
				if (HostUARTDevice::instance()->weapon_work_time >= 60) // ori:45 20231011 change to 60s according to agreement
				{
					HostUARTDevice::instance()->weapon_activate = false;
					osd_pos_deorbit_time.config.para.Enable = 0;
					osd_pos_deorbit_time.str_arr = osd_blank;
					update_OSD_chinese(osd_pos_deorbit_time, OSD_BRAM_HANDLE);
				}
			}
			else
			{
				osd_pos_deorbit_time.config.para.Enable = 0;
				update_OSD_chinese(osd_pos_deorbit_time, OSD_BRAM_HANDLE);
			}

            // command ID 0x21 location ID 0x12  shine code HostUARTDevice::instance()->shine_code_str
            // static bool weapon_activate_for_shinecode = false;
            static std::string shine_code_str = "-";
            // if(level_changed || shine_code_str.compare(HostUARTDevice::instance()->shine_code_str) != 0 || weapon_activate_for_shinecode != HostUARTDevice::instance()->weapon_activate)
            if(level_changed || shine_code_str.compare(HostUARTDevice::instance()->shine_code_str) != 0)
            {
                shine_code_str = HostUARTDevice::instance()->shine_code_str;
                // weapon_activate_for_shinecode = HostUARTDevice::instance()->weapon_activate;
                for(int i = 0; i<(HostUARTDevice::instance()->shine_code_str).size(); i++)
                {
                    osd_shinecode[i+9] = (HostUARTDevice::instance()->shine_code_str)[i];
                }
                // if((show_level==1 || show_level==2 || show_level==5 || show_level==6) && weapon_activate_for_shinecode == false)
                if((show_level==1 || show_level==2 || show_level==5 || show_level==6) && shine_code_str.compare(" ") != 0)
                {
                    osd_pos_shine_code.config.para.Enable = 1;
                }
                else
                {
                    osd_pos_shine_code.config.para.Enable = 0;
                }
                osd_pos_shine_code.str_arr = osd_shinecode;
                update_OSD_chinese(osd_pos_shine_code, OSD_BRAM_HANDLE);
            }

            // command ID 0x21  location ID 0x13 shine time HostUARTDevice::instance()->shine_time_str
            // static bool weapon_activate_for_shinetime = false;
            static std::string shine_time_str = "-";
            // if(level_changed || shine_time_str.compare(HostUARTDevice::instance()->shine_time_str) != 0 || weapon_activate_for_shinetime != HostUARTDevice::instance()->weapon_activate)
            if(level_changed || shine_time_str.compare(HostUARTDevice::instance()->shine_time_str) != 0)
            {
                shine_time_str = HostUARTDevice::instance()->shine_time_str;
                // weapon_activate_for_shinetime = HostUARTDevice::instance()->weapon_activate;
                for(int i = 0; i<(HostUARTDevice::instance()->shine_time_str).size(); i++)
                {
                    osd_shinetime[i+9] = (HostUARTDevice::instance()->shine_time_str)[i];
                }
                // if((show_level==1 || show_level==2 || show_level==5 || show_level==6) && weapon_activate_for_shinetime == false)
                if((show_level==1 || show_level==2 || show_level==5 || show_level==6) && shine_time_str.compare(" ") != 0)
                {
                    osd_pos_shine_time.config.para.Enable = 1;
                }
                else
                {
                    osd_pos_shine_time.config.para.Enable = 0;
                }
                osd_pos_shine_time.str_arr = osd_shinetime;
                update_OSD_chinese(osd_pos_shine_time, OSD_BRAM_HANDLE);
            }

			// command ID 0x1a  laser forbid HostUARTDevice::instance()->laser_forbid
			static bool laser_forbid = true;
			if(level_changed || laser_forbid != HostUARTDevice::instance()->laser_forbid)
			{
				laser_forbid = HostUARTDevice::instance()->laser_forbid;
				if (!laser_forbid)
				{
					osd_pos_laser_prohibit.config.para.Enable = 0;
					osd_pos_laser_prohibit.str_arr = osd_blank;
					update_OSD_chinese(osd_pos_laser_prohibit, OSD_BRAM_HANDLE);
				}
				else
				{
					if(show_level==1 || show_level==2 || show_level==5 || show_level==6)
					{
						osd_pos_laser_prohibit.config.para.Enable = 1;
					}
					else
					{
						osd_pos_laser_prohibit.config.para.Enable = 0;
					}
					osd_pos_laser_prohibit.str_arr = osd_laserprohibit;
					update_OSD_chinese(osd_pos_laser_prohibit, OSD_BRAM_HANDLE);
				}
			}

			// command ID 0x1a temperature alarm HostUARTDevice::instance()->laser_alarm
			static bool laser_alarm = false;
			if(level_changed || laser_alarm != HostUARTDevice::instance()->laser_alarm)
			{
				laser_alarm = HostUARTDevice::instance()->laser_alarm;
				if (laser_alarm)
				{
					if(show_level==1 || show_level==2 || show_level==5)
					{
						osd_pos_temperature_alarm.config.para.Enable = 1;
					}
					else
					{
						osd_pos_temperature_alarm.config.para.Enable = 0;
					}
					osd_pos_temperature_alarm.str_arr = osd_temperaturealarm;
					update_OSD_chinese(osd_pos_temperature_alarm, OSD_BRAM_HANDLE);
				}
				else
				{
					osd_pos_temperature_alarm.config.para.Enable = 0;
					osd_pos_temperature_alarm.str_arr = osd_blank;
					update_OSD_chinese(osd_pos_temperature_alarm, OSD_BRAM_HANDLE);
				}
			}

			// object location
			if(level_changed)
			{
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_object_position.config.para.Enable = 1;
				}
				else
				{
					osd_pos_object_position.config.para.Enable = 0;
				}
				osd_pos_object_position.str_arr = osd_object_location;
				update_OSD_chinese(osd_pos_object_position, OSD_BRAM_HANDLE);
            }

			// command ID 0x0d  object location    longitude   HostUARTDevice::instance()->object_longitude_str
			static std::string object_longitude_str = "---";
			if(level_changed || object_longitude_str.compare(HostUARTDevice::instance()->object_longitude_str) != 0)
			{
				object_longitude_str = HostUARTDevice::instance()->object_longitude_str;
				for (int i = 0; i < (HostUARTDevice::instance()->object_longitude_str).size(); i++)
				{
					osd_object_longitude[i + 5] = (HostUARTDevice::instance()->object_longitude_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_object_position_longitude.config.para.Enable = 1;
				}
				else
				{
					osd_pos_object_position_longitude.config.para.Enable = 0;
				}
				osd_pos_object_position_longitude.str_arr = osd_object_longitude;
				update_OSD_chinese(osd_pos_object_position_longitude, OSD_BRAM_HANDLE);
			}

			// command ID 0x0e  object location  latitute   HostUARTDevice::instance()->object_latitude_str
			static std::string object_latitude_str = "---";
			if(level_changed || object_latitude_str.compare(HostUARTDevice::instance()->object_latitude_str) != 0)
			{
				object_latitude_str = HostUARTDevice::instance()->object_latitude_str;
				for (int i = 0; i < (HostUARTDevice::instance()->object_latitude_str).size(); i++)
				{
					osd_object_latitude[i+5] = (HostUARTDevice::instance()->object_latitude_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_object_position_latitude.config.para.Enable = 1;
				}
				else
				{
					osd_pos_object_position_latitude.config.para.Enable = 0;
				}
				osd_pos_object_position_latitude.str_arr = osd_object_latitude;
				update_OSD_chinese(osd_pos_object_position_latitude, OSD_BRAM_HANDLE);
			}

			// command ID 0x0f object location   altitude   HostUARTDevice::instance()->object_altitude_str
			static std::string object_altitude_str = "---";
			if(level_changed || object_altitude_str.compare(HostUARTDevice::instance()->object_altitude_str) != 0)
			{
				object_altitude_str = HostUARTDevice::instance()->object_altitude_str;
				for (int i = 0; i < (HostUARTDevice::instance()->object_altitude_str).size(); i++)
				{
					osd_object_altitude[i + 5] = (HostUARTDevice::instance()->object_altitude_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_object_position_altitude.config.para.Enable = 1;
				}
				else
				{
					osd_pos_object_position_altitude.config.para.Enable = 0;
				}
				osd_pos_object_position_altitude.str_arr = osd_object_altitude;
				update_OSD_chinese(osd_pos_object_position_altitude, OSD_BRAM_HANDLE);
            }

			// aircraft location
			if(level_changed)
			{
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_aircraft_position.config.para.Enable = 1;
				}
				else
				{
					osd_pos_aircraft_position.config.para.Enable = 0;
				}
				osd_pos_aircraft_position.str_arr = osd_aircraft_location;
				update_OSD_chinese(osd_pos_aircraft_position, OSD_BRAM_HANDLE);
			}

            // command ID 0x08 pos_30_info HostUARTDevice::instance()->position_30_str multi_target_status
            static uint8_t detect_status = '0';
			if(level_changed || (detect_status != HostUARTDevice::instance()->detect_status))  
            {
                detect_status = HostUARTDevice::instance()->detect_status;
				// for (int i = 0; i < (HostUARTDevice::instance()->position_30_str).size(); i++)
				// {
				// 	osd_pos_30_info[i] = (HostUARTDevice::instance()->position_30_str)[i];
				// }
                switch(detect_status)
                {
                    case '0':
                        osd_pos_30.str_arr = osd_blank;
                        break;
                    case '1':
                        osd_pos_30.str_arr = osd_ai_detect;
                        break;
                    case '2':
                        osd_pos_30.str_arr = osd_moving_detect;
                        break;
                }
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_30.config.para.Enable = 1;
				}
				else
				{
					osd_pos_30.config.para.Enable = 0;
				}
				update_OSD_chinese(osd_pos_30, OSD_BRAM_HANDLE);                
            }    

			// command ID 0x09 aircraft location  longitude   HostUARTDevice::instance()->aircraft_longitude_str
			static std::string aircraft_longitude_str = "---";
			if(level_changed || aircraft_longitude_str.compare(HostUARTDevice::instance()->aircraft_longitude_str) != 0)
			{
				aircraft_longitude_str = HostUARTDevice::instance()->aircraft_longitude_str;
				for (int i = 0; i < (HostUARTDevice::instance()->aircraft_longitude_str).size(); i++)
				{
					osd_aircraft_longitude[i + 5] = (HostUARTDevice::instance()->aircraft_longitude_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_aircraft_position_longitude.config.para.Enable = 1;
				}
				else
				{
					osd_pos_aircraft_position_longitude.config.para.Enable = 0;
				}
				osd_pos_aircraft_position_longitude.str_arr = osd_aircraft_longitude;
				update_OSD_chinese(osd_pos_aircraft_position_longitude, OSD_BRAM_HANDLE);
            }

			// command ID 0x0a  aircraft location  latitude   HostUARTDevice::instance()->aircraft_latitude_str
			static std::string aircraft_latitude_str = "---";
			if(level_changed || aircraft_latitude_str.compare(HostUARTDevice::instance()->aircraft_latitude_str) != 0)
			{
				aircraft_latitude_str = HostUARTDevice::instance()->aircraft_latitude_str;
				for (int i = 0; i < (HostUARTDevice::instance()->aircraft_latitude_str).size(); i++)
				{
					osd_aircraft_latitude[i + 5] = (HostUARTDevice::instance()->aircraft_latitude_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_aircraft_position_latitude.config.para.Enable = 1;
				}
				else
				{
					osd_pos_aircraft_position_latitude.config.para.Enable = 0;
				}
				osd_pos_aircraft_position_latitude.str_arr = osd_aircraft_latitude;
				update_OSD_chinese(osd_pos_aircraft_position_latitude, OSD_BRAM_HANDLE);
            }

			// command ID 0x0b  aircraft location  altitude   HostUARTDevice::instance()->aircraft_altitude_str
			static std::string aircraft_altitude_str = "---";
			if(level_changed || aircraft_altitude_str.compare(HostUARTDevice::instance()->aircraft_altitude_str) != 0)
			{
				aircraft_altitude_str = HostUARTDevice::instance()->aircraft_altitude_str;
				for (int i = 0; i < (HostUARTDevice::instance()->aircraft_altitude_str).size(); i++)
				{
					osd_aircraft_altitude[i + 5] = (HostUARTDevice::instance()->aircraft_altitude_str)[i];
				}
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_aircraft_position_altitude.config.para.Enable = 1;
				}
				else
				{
					osd_pos_aircraft_position_altitude.config.para.Enable = 0;
				}
				osd_pos_aircraft_position_altitude.str_arr = osd_aircraft_altitude;
				update_OSD_chinese(osd_pos_aircraft_position_altitude, OSD_BRAM_HANDLE);
                // aircraft_altitude = (uint16_t)(HostUARTDevice::instance()->aircraft_altitude / 50);
			}

            // command ID 0x0c pos_31_info HostUARTDevice::instance()->position_31_str  take_photo_status
            static uint8_t photo_status = 'f';
			if(level_changed || (photo_status != HostUARTDevice::instance()->photo_status))  
            {
                photo_status = HostUARTDevice::instance()->photo_status;
				// for (int i = 0; i < (HostUARTDevice::instance()->position_31_str).size(); i++)
				// {
				// 	osd_pos_31_info[i] = (HostUARTDevice::instance()->position_31_str)[i];
				// }
                switch(photo_status)
                {
                    case '0':
                        osd_pos_31.str_arr = osd_blank;
                        break;
                    case '1':
                        osd_pos_31.str_arr = osd_single_photo;
                        break;
                    case '2':
                        osd_pos_31.str_arr = osd_multi_photo;
                        break;
                }
				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_31.config.para.Enable = 1;
				}
				else
				{
					osd_pos_31.config.para.Enable = 0;
				}
				update_OSD_chinese(osd_pos_31, OSD_BRAM_HANDLE);                
            } 

			// command ID 0x27 north angle
			if(level_changed || north_angle_str.compare(HostUARTDevice::instance()->north_angle_str) != 0)
			{
				north_angle_str = HostUARTDevice::instance()->north_angle_str;
				for(int i = 0; i<7; i++)
				{
					osd_north_angle[i+2] = (HostUARTDevice::instance()->north_angle_str)[i];
				}

                try
                {
                    north_angle = std::stof(north_angle_str);
                }
                catch (std::invalid_argument& e)
                {
                    printf("north_angle_str stof invalid_argument.\n");
                }

				if(show_level==1 || show_level==2 || show_level==5)
				{
					osd_pos_north_angle.config.para.Enable = 1;
                    update_north_arrow(north_angle, targraphColor.ColorConfig.color.Color_R_Y, targraphColor.ColorConfig.color.Color_G_U, targraphColor.ColorConfig.color.Color_B_V, true);
				}
				else
				{
					osd_pos_north_angle.config.para.Enable = 0;
                    update_north_arrow(north_angle, targraphColor.ColorConfig.color.Color_R_Y, targraphColor.ColorConfig.color.Color_G_U, targraphColor.ColorConfig.color.Color_B_V, false);
				}
				osd_pos_north_angle.str_arr = osd_north_angle;
				update_OSD_chinese(osd_pos_north_angle, OSD_BRAM_HANDLE);
			}

			// command ID 0x22   system information  HostUARTDevice::instance()->system_info_str
            static uint8_t start_pos = 0;
			if (HostUARTDevice::instance()->system_info_enable)
			{
				if (get_wall_time() - HostUARTDevice::instance()->system_info_start < 3)
				{
                    for(int i = 0; i < 32; i++)
                        osd_system_info[i] = osd_blank[i];
                    switch(HostUARTDevice::instance()->show_type)
                    {
                        case 11:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_xiao_1004_1; osd_system_info[start_pos+1] = CH_idx_xiao_1005_2; 
                            osd_system_info[start_pos+2] = CH_idx_zhou_1202_1; osd_system_info[start_pos+3] = CH_idx_zhou_1203_2;
                            osd_system_info[start_pos+4] = CH_idx_kai_588_1; osd_system_info[start_pos+5] = CH_idx_kai_589_2;
                            osd_system_info[start_pos+6] = CH_idx_shi_832_1; osd_system_info[start_pos+7] = CH_idx_shi_833_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;
                        case 12:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_xiao_1004_1; osd_system_info[start_pos+1] = CH_idx_xiao_1005_2; 
                            osd_system_info[start_pos+2] = CH_idx_zhou_1202_1; osd_system_info[start_pos+3] = CH_idx_zhou_1203_2;
                            osd_system_info[start_pos+4] = CH_idx_jie_558_1; osd_system_info[start_pos+5] = CH_idx_jie_559_2;
                            osd_system_info[start_pos+6] = CH_idx_shu_1290_1; osd_system_info[start_pos+7] = CH_idx_shu_1291_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                        
                        case 13:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_xiao_1004_1; osd_system_info[start_pos+1] = CH_idx_xiao_1005_2; 
                            osd_system_info[start_pos+2] = CH_idx_zhou_1202_1; osd_system_info[start_pos+3] = CH_idx_zhou_1203_2;
                            osd_system_info[start_pos+4] = CH_idx_cheng_232_1; osd_system_info[start_pos+5] = CH_idx_cheng_233_2;
                            osd_system_info[start_pos+6] = CH_idx_gong_414_1; osd_system_info[start_pos+7] = CH_idx_gong_415_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 14:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_xiao_1004_1; osd_system_info[start_pos+1] = CH_idx_xiao_1005_2; 
                            osd_system_info[start_pos+2] = CH_idx_zhou_1202_1; osd_system_info[start_pos+3] = CH_idx_zhou_1203_2;
                            osd_system_info[start_pos+4] = CH_idx_shi_820_1; osd_system_info[start_pos+5] = CH_idx_shi_821_2;
                            osd_system_info[start_pos+6] = CH_idx_bai_134_1; osd_system_info[start_pos+7] = CH_idx_bai_135_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 15:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_xiao_1004_1; osd_system_info[start_pos+1] = CH_idx_xiao_1005_2; 
                            osd_system_info[start_pos+2] = CH_idx_zhou_1202_1; osd_system_info[start_pos+3] = CH_idx_zhou_1203_2;
                            osd_system_info[start_pos+4] = CH_idx_wu_962_1; osd_system_info[start_pos+5] = CH_idx_wu_963_2;
                            osd_system_info[start_pos+6] = CH_idx_cha_214_1; osd_system_info[start_pos+7] = CH_idx_cha_215_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 21:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((10 - (HostUARTDevice::instance()->system_info_str).size()/2) + (10 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 0;
                            osd_system_info[start_pos] = CH_idx_ji_506_1; osd_system_info[start_pos+1] = CH_idx_ji_507_2; 
                            osd_system_info[start_pos+2] = CH_idx_guang_432_1; osd_system_info[start_pos+3] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+4] = CH_idx_mai_1274_1; osd_system_info[start_pos+5] = CH_idx_mai_1275_2;
                            osd_system_info[start_pos+6] = CH_idx_chong_1258_1; osd_system_info[start_pos+7] = CH_idx_chong_1259_2;
                            osd_system_info[start_pos+8] = CH_idx_ji_518_1; osd_system_info[start_pos+9] = CH_idx_ji_519_2;
                            osd_system_info[start_pos+10] = CH_idx_shu_860_1; osd_system_info[start_pos+11] = CH_idx_shu_861_2;                            
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+12+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 22:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((10 - (HostUARTDevice::instance()->system_info_str).size()/2) + (10 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 0;
                            osd_system_info[start_pos] = CH_idx_neng_712_1; osd_system_info[start_pos+1] = CH_idx_neng_713_2; 
                            osd_system_info[start_pos+2] = CH_idx_liang_644_1; osd_system_info[start_pos+3] = CH_idx_liang_645_2;
                            osd_system_info[start_pos+4] = CH_idx_jian_532_1; osd_system_info[start_pos+5] = CH_idx_jian_533_2;
                            osd_system_info[start_pos+6] = CH_idx_ce_210_1; osd_system_info[start_pos+7] = CH_idx_ce_211_2;
                            osd_system_info[start_pos+8] = CH_idx_kai_588_1; osd_system_info[start_pos+9] = CH_idx_kai_589_2;
                            osd_system_info[start_pos+10] = CH_idx_shi_832_1; osd_system_info[start_pos+11] = CH_idx_shi_833_2;                            
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+12+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 23:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((10 - (HostUARTDevice::instance()->system_info_str).size()/2) + (10 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 0;
                            osd_system_info[start_pos] = CH_idx_neng_712_1; osd_system_info[start_pos+1] = CH_idx_neng_713_2; 
                            osd_system_info[start_pos+2] = CH_idx_liang_644_1; osd_system_info[start_pos+3] = CH_idx_liang_645_2;
                            osd_system_info[start_pos+4] = CH_idx_jian_532_1; osd_system_info[start_pos+5] = CH_idx_jian_533_2;
                            osd_system_info[start_pos+6] = CH_idx_ce_210_1; osd_system_info[start_pos+7] = CH_idx_ce_211_2;
                            osd_system_info[start_pos+8] = CH_idx_jie_558_1; osd_system_info[start_pos+9] = CH_idx_jie_559_2;
                            osd_system_info[start_pos+10] = CH_idx_shu_1290_1; osd_system_info[start_pos+11] = CH_idx_shu_1291_2;                            
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+12+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 24:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((10 - (HostUARTDevice::instance()->system_info_str).size()/2) + (10 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 0;
                            osd_system_info[start_pos] = CH_idx_ji_506_1; osd_system_info[start_pos+1] = CH_idx_ji_507_2; 
                            osd_system_info[start_pos+2] = CH_idx_guang_432_1; osd_system_info[start_pos+3] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+4] = CH_idx_mo_696_1; osd_system_info[start_pos+5] = CH_idx_mo_697_2;
                            osd_system_info[start_pos+6] = CH_idx_ni_1282_1; osd_system_info[start_pos+7] = CH_idx_ni_1283_2;
                            osd_system_info[start_pos+8] = CH_idx_jin_568_1; osd_system_info[start_pos+9] = CH_idx_jin_569_2;
                            osd_system_info[start_pos+10] = CH_idx_ru_792_1; osd_system_info[start_pos+11] = CH_idx_ru_793_2;                            
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+12+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 25:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((10 - (HostUARTDevice::instance()->system_info_str).size()/2) + (10 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 0;
                            osd_system_info[start_pos] = CH_idx_ji_506_1; osd_system_info[start_pos+1] = CH_idx_ji_507_2; 
                            osd_system_info[start_pos+2] = CH_idx_guang_432_1; osd_system_info[start_pos+3] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+4] = CH_idx_mo_696_1; osd_system_info[start_pos+5] = CH_idx_mo_697_2;
                            osd_system_info[start_pos+6] = CH_idx_ni_1282_1; osd_system_info[start_pos+7] = CH_idx_ni_1283_2;
                            osd_system_info[start_pos+8] = CH_idx_tui_922_1; osd_system_info[start_pos+9] = CH_idx_tui_923_2;
                            osd_system_info[start_pos+10] = CH_idx_chu_242_1; osd_system_info[start_pos+11] = CH_idx_chu_243_2;                            
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+12+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 26:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_ji_506_1; osd_system_info[start_pos+1] = CH_idx_ji_507_2; 
                            osd_system_info[start_pos+2] = CH_idx_guang_432_1; osd_system_info[start_pos+3] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+4] = CH_idx_neng_712_1; osd_system_info[start_pos+5] = CH_idx_neng_713_2; 
                            osd_system_info[start_pos+6] = CH_idx_liang_644_1; osd_system_info[start_pos+7] = CH_idx_liang_645_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 27:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((11 - (HostUARTDevice::instance()->system_info_str).size()/2) + (11 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_ji_506_1; osd_system_info[start_pos+1] = CH_idx_ji_507_2; 
                            osd_system_info[start_pos+2] = CH_idx_guang_432_1; osd_system_info[start_pos+3] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+4] = CH_idx_dang_288_1; osd_system_info[start_pos+5] = CH_idx_dang_289_2; 
                            osd_system_info[start_pos+6] = CH_idx_pian_726_1; osd_system_info[start_pos+7] = CH_idx_pian_727_2;
                            osd_system_info[start_pos+8] = CH_idx_kai_588_1; osd_system_info[start_pos+9] = CH_idx_kai_589_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+10+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                        
                        case 28:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((11 - (HostUARTDevice::instance()->system_info_str).size()/2) + (11 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_ji_506_1; osd_system_info[start_pos+1] = CH_idx_ji_507_2; 
                            osd_system_info[start_pos+2] = CH_idx_guang_432_1; osd_system_info[start_pos+3] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+4] = CH_idx_dang_288_1; osd_system_info[start_pos+5] = CH_idx_dang_289_2; 
                            osd_system_info[start_pos+6] = CH_idx_pian_726_1; osd_system_info[start_pos+7] = CH_idx_pian_727_2;
                            osd_system_info[start_pos+8] = CH_idx_guan_426_1; osd_system_info[start_pos+9] = CH_idx_guan_427_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+10+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 31:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((10 - (HostUARTDevice::instance()->system_info_str).size()/2) + (10 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 0;
                            osd_system_info[start_pos] = CH_idx_re_778_1; osd_system_info[start_pos+1] = CH_idx_re_779_2; 
                            osd_system_info[start_pos+2] = CH_idx_xiang_998_1; osd_system_info[start_pos+3] = CH_idx_xiang_999_2;
                            osd_system_info[start_pos+4] = CH_idx_gong_410_1; osd_system_info[start_pos+5] = CH_idx_gong_411_2;
                            osd_system_info[start_pos+6] = CH_idx_zuo_1236_1; osd_system_info[start_pos+7] = CH_idx_zuo_1237_2;
                            osd_system_info[start_pos+8] = CH_idx_shi_824_1; osd_system_info[start_pos+9] = CH_idx_shi_825_2;
                            osd_system_info[start_pos+10] = CH_idx_jian_530_1; osd_system_info[start_pos+11] = CH_idx_jian_531_2;                            
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+12+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 36:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((14 - (HostUARTDevice::instance()->system_info_str).size()/2) + (14 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 4;
                            osd_system_info[start_pos] = CH_idx_chang_224_1; osd_system_info[start_pos+1] = CH_idx_chang_225_2; 
                            osd_system_info[start_pos+2] = CH_idx_gao_390_1; osd_system_info[start_pos+3] = CH_idx_gao_391_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+4+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 41:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((11 - (HostUARTDevice::instance()->system_info_str).size()/2) + (11 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_yao_1316_1; osd_system_info[start_pos+1] = CH_idx_yao_1317_2; 
                            osd_system_info[start_pos+2] = CH_idx_gan_1260_1; osd_system_info[start_pos+3] = CH_idx_gan_1261_2;
                            osd_system_info[start_pos+4] = CH_idx_xiang_992_1; osd_system_info[start_pos+5] = CH_idx_xiang_993_2; 
                            osd_system_info[start_pos+6] = CH_idx_ying_1090_1; osd_system_info[start_pos+7] = CH_idx_ying_1091_2;
                            osd_system_info[start_pos+8] = CH_idx_kuai_608_1; osd_system_info[start_pos+9] = CH_idx_kuai_609_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+10+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 42:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((11 - (HostUARTDevice::instance()->system_info_str).size()/2) + (11 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_yao_1316_1; osd_system_info[start_pos+1] = CH_idx_yao_1317_2; 
                            osd_system_info[start_pos+2] = CH_idx_gan_1260_1; osd_system_info[start_pos+3] = CH_idx_gan_1261_2;
                            osd_system_info[start_pos+4] = CH_idx_xiang_992_1; osd_system_info[start_pos+5] = CH_idx_xiang_993_2; 
                            osd_system_info[start_pos+6] = CH_idx_ying_1090_1; osd_system_info[start_pos+7] = CH_idx_ying_1091_2;
                            osd_system_info[start_pos+8] = CH_idx_man_1314_1; osd_system_info[start_pos+9] = CH_idx_man_1315_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+10+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 51:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((11 - (HostUARTDevice::instance()->system_info_str).size()/2) + (11 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) :2;
                            osd_system_info[start_pos] = CH_idx_bu_188_1; osd_system_info[start_pos+1] = CH_idx_bu_189_2; 
                            osd_system_info[start_pos+2] = CH_idx_yun_1128_1; osd_system_info[start_pos+3] = CH_idx_yun_1129_2;
                            osd_system_info[start_pos+4] = CH_idx_xu_1032_1; osd_system_info[start_pos+5] = CH_idx_xu_1033_2;
                            osd_system_info[start_pos+6] = CH_idx_ding_320_1; osd_system_info[start_pos+7] = CH_idx_ding_321_2;
                            osd_system_info[start_pos+8] = CH_idx_wei_946_1; osd_system_info[start_pos+9] = CH_idx_wei_947_2;                          
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+10+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 52:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_ding_320_1; osd_system_info[start_pos+1] = CH_idx_ding_321_2;
                            osd_system_info[start_pos+2] = CH_idx_wei_946_1; osd_system_info[start_pos+3] = CH_idx_wei_947_2; 
                            osd_system_info[start_pos+4] = CH_idx_cheng_232_1; osd_system_info[start_pos+5] = CH_idx_cheng_233_2;
                            osd_system_info[start_pos+6] = CH_idx_gong_414_1; osd_system_info[start_pos+7] = CH_idx_gong_415_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 53:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_ding_320_1; osd_system_info[start_pos+1] = CH_idx_ding_321_2;
                            osd_system_info[start_pos+2] = CH_idx_wei_946_1; osd_system_info[start_pos+3] = CH_idx_wei_947_2; 
                            osd_system_info[start_pos+4] = CH_idx_shi_820_1; osd_system_info[start_pos+5] = CH_idx_shi_821_2;
                            osd_system_info[start_pos+6] = CH_idx_bai_134_1; osd_system_info[start_pos+7] = CH_idx_bai_135_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 61:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guan_430_1; osd_system_info[start_pos+1] = CH_idx_guan_431_2;
                            osd_system_info[start_pos+2] = CH_idx_dao_292_1; osd_system_info[start_pos+3] = CH_idx_dao_293_2; 
                            osd_system_info[start_pos+4] = CH_idx_fang_352_1; osd_system_info[start_pos+5] = CH_idx_fang_353_2;
                            osd_system_info[start_pos+6] = CH_idx_wei_946_1; osd_system_info[start_pos+7] = CH_idx_wei_947_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 62:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guan_430_1; osd_system_info[start_pos+1] = CH_idx_guan_431_2;
                            osd_system_info[start_pos+2] = CH_idx_dao_292_1; osd_system_info[start_pos+3] = CH_idx_dao_293_2; 
                            osd_system_info[start_pos+4] = CH_idx_fu_370_1; osd_system_info[start_pos+5] = CH_idx_fu_371_2;
                            osd_system_info[start_pos+6] = CH_idx_yang_1054_1; osd_system_info[start_pos+7] = CH_idx_yang_1055_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 63:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guan_430_1; osd_system_info[start_pos+1] = CH_idx_guan_431_2;
                            osd_system_info[start_pos+2] = CH_idx_dao_292_1; osd_system_info[start_pos+3] = CH_idx_dao_293_2; 
                            osd_system_info[start_pos+4] = CH_idx_heng_466_1; osd_system_info[start_pos+5] = CH_idx_heng_467_2;
                            osd_system_info[start_pos+6] = CH_idx_gun_436_1; osd_system_info[start_pos+7] = CH_idx_gun_437_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 64:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guan_430_1; osd_system_info[start_pos+1] = CH_idx_guan_431_2;
                            osd_system_info[start_pos+2] = CH_idx_dao_292_1; osd_system_info[start_pos+3] = CH_idx_dao_293_2; 
                            osd_system_info[start_pos+4] = CH_idx_jing_574_1; osd_system_info[start_pos+5] = CH_idx_jing_575_2;
                            osd_system_info[start_pos+6] = CH_idx_du_332_1; osd_system_info[start_pos+7] = CH_idx_du_333_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 65:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guan_430_1; osd_system_info[start_pos+1] = CH_idx_guan_431_2;
                            osd_system_info[start_pos+2] = CH_idx_dao_292_1; osd_system_info[start_pos+3] = CH_idx_dao_293_2; 
                            osd_system_info[start_pos+4] = CH_idx_wei_938_1; osd_system_info[start_pos+5] = CH_idx_wei_939_2;
                            osd_system_info[start_pos+6] = CH_idx_du_332_1; osd_system_info[start_pos+7] = CH_idx_du_333_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 66:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guan_430_1; osd_system_info[start_pos+1] = CH_idx_guan_431_2;
                            osd_system_info[start_pos+2] = CH_idx_dao_292_1; osd_system_info[start_pos+3] = CH_idx_dao_293_2; 
                            osd_system_info[start_pos+4] = CH_idx_gao_390_1; osd_system_info[start_pos+5] = CH_idx_gao_391_2;
                            osd_system_info[start_pos+6] = CH_idx_du_332_1; osd_system_info[start_pos+7] = CH_idx_du_333_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 67:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guang_432_1; osd_system_info[start_pos+1] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+2] = CH_idx_dian_312_1; osd_system_info[start_pos+3] = CH_idx_dian_313_2; 
                            osd_system_info[start_pos+4] = CH_idx_fang_352_1; osd_system_info[start_pos+5] = CH_idx_fang_353_2;
                            osd_system_info[start_pos+6] = CH_idx_wei_946_1; osd_system_info[start_pos+7] = CH_idx_wei_947_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 68:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_guang_432_1; osd_system_info[start_pos+1] = CH_idx_guang_433_2;
                            osd_system_info[start_pos+2] = CH_idx_dian_312_1; osd_system_info[start_pos+3] = CH_idx_dian_313_2; 
                            osd_system_info[start_pos+4] = CH_idx_fu_370_1; osd_system_info[start_pos+5] = CH_idx_fu_371_2;
                            osd_system_info[start_pos+6] = CH_idx_yang_1054_1; osd_system_info[start_pos+7] = CH_idx_yang_1055_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 71:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_jia_526_1; osd_system_info[start_pos+1] = CH_idx_jia_527_2;
                            osd_system_info[start_pos+2] = CH_idx_re_778_1; osd_system_info[start_pos+3] = CH_idx_re_779_2; 
                            osd_system_info[start_pos+4] = CH_idx_kai_588_1; osd_system_info[start_pos+5] = CH_idx_kai_589_2;
                            osd_system_info[start_pos+6] = CH_idx_shi_832_1; osd_system_info[start_pos+7] = CH_idx_shi_833_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;                         
                        case 72:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_jia_526_1; osd_system_info[start_pos+1] = CH_idx_jia_527_2;
                            osd_system_info[start_pos+2] = CH_idx_re_778_1; osd_system_info[start_pos+3] = CH_idx_re_779_2; 
                            osd_system_info[start_pos+4] = CH_idx_ting_908_1; osd_system_info[start_pos+5] = CH_idx_ting_909_2;
                            osd_system_info[start_pos+6] = CH_idx_zhi_1178_1; osd_system_info[start_pos+7] = CH_idx_zhi_1179_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;  
                        case 81:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((13 - (HostUARTDevice::instance()->system_info_str).size()/2) + (13 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 4;
                            osd_system_info[start_pos] = CH_idx_wu_954_1; osd_system_info[start_pos+1] = CH_idx_wu_955_2;
                            osd_system_info[start_pos+2] = CH_idx_gu_424_1; osd_system_info[start_pos+3] = CH_idx_gu_425_2; 
                            osd_system_info[start_pos+4] = CH_idx_zhang_1152_1; osd_system_info[start_pos+5] = CH_idx_zhang_1153_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+6+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }                            
                        }   
                            break;
                        case 82:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((12 - (HostUARTDevice::instance()->system_info_str).size()/2) + (12 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 2;
                            osd_system_info[start_pos] = CH_idx_gu_424_1; osd_system_info[start_pos+1] = CH_idx_gu_425_2;
                            osd_system_info[start_pos+2] = CH_idx_zhang_1152_1; osd_system_info[start_pos+3] = CH_idx_zhang_1153_2;
                            osd_system_info[start_pos+4] = CH_idx_cun_266_1; osd_system_info[start_pos+5] = CH_idx_cun_267_2;
                            osd_system_info[start_pos+6] = CH_idx_zai_1134_1; osd_system_info[start_pos+7] = CH_idx_zai_1135_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+8+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }   
                            break;
                        case 83:
                        {
                            start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((13 - (HostUARTDevice::instance()->system_info_str).size()/2) + (13 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 4;
                            osd_system_info[start_pos] = CH_idx_gu_424_1; osd_system_info[start_pos+1] = CH_idx_gu_425_2;
                            osd_system_info[start_pos+2] = CH_idx_zhang_1152_1; osd_system_info[start_pos+3] = CH_idx_zhang_1153_2;
                            osd_system_info[start_pos+4] = CH_idx_ma_674_1; osd_system_info[start_pos+5] = CH_idx_ma_675_2;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+6+start_pos] = (HostUARTDevice::instance()->system_info_str)[i];
                            } 
                        }   
                            break;                      
                        default:
                        {
                            HostUARTDevice::instance()->system_info_str = pad_str(HostUARTDevice::instance()->system_info_str, 20, 0);
                            // start_pos = ((HostUARTDevice::instance()->system_info_str).size() <= 20) ? ((16 - (HostUARTDevice::instance()->system_info_str).size()/2) + (16 - (HostUARTDevice::instance()->system_info_str).size()/2)%2) : 6;
                            for(int i=0; i<(HostUARTDevice::instance()->system_info_str).size(); i++)
                            {
                                osd_system_info[i+6] = (HostUARTDevice::instance()->system_info_str)[i];
                            }
                        }
                            break;
                    }

					if(show_level==1 || show_level==5)
					{
						osd_pos_system_information.config.para.Enable = 1;
					}
					else
					{
						osd_pos_system_information.config.para.Enable = 0;
					}
					osd_pos_system_information.str_arr = osd_system_info;
					update_OSD_chinese(osd_pos_system_information, OSD_BRAM_HANDLE);
				}
				else
				{                   
					HostUARTDevice::instance()->system_info_enable = false;
					osd_pos_system_information.config.para.Enable = 0;
					osd_pos_system_information.str_arr = osd_blank;
					update_OSD_chinese(osd_pos_system_information, OSD_BRAM_HANDLE);
				}
			}
			else
			{
                if(osd_pos_system_information.config.para.Enable == 1)
                {
                    for(int i = 0; i < 32; i++)
                        osd_system_info[i] = osd_blank[i];
					osd_pos_system_information.config.para.Enable = 0;
					update_OSD_chinese(osd_pos_system_information, OSD_BRAM_HANDLE);
                }
			}
			
			if (HostUARTDevice::instance()->system_version_enable)
			{
				if (get_wall_time() - HostUARTDevice::instance()->system_version_start < 3)
				{
					for(int i=0; i<(HostUARTDevice::instance()->show_str_version).size(); i++)
					{
						osd_version_info[i] = (HostUARTDevice::instance()->show_str_version)[i];
					}
					if(show_level==1 || show_level==2 || show_level==5)
					{
						osd_pos_version.config.para.Enable = 1;
					}
					else
					{
						osd_pos_version.config.para.Enable = 0;
					}
					osd_pos_version.str_arr = osd_version_info;
					update_OSD_chinese(osd_pos_version, OSD_BRAM_HANDLE);
				}
				else
				{
					HostUARTDevice::instance()->system_version_enable = false;
					osd_pos_version.config.para.Enable = 0;
					osd_pos_version.str_arr = osd_blank;
					update_OSD_chinese(osd_pos_version, OSD_BRAM_HANDLE);
				}
			}
			else
			{
                if(osd_pos_version.config.para.Enable == 1)
                {
                    osd_pos_version.config.para.Enable = 0;
                    update_OSD_chinese(osd_pos_version, OSD_BRAM_HANDLE);
                }
			}
                //TV and IR scaling
                {                       
                    if( view_scale != HostUARTDevice::instance()->view_scale ) //|| (ir_scale != HostUARTDevice::instance()->ir_scale)
                    {   
                        view_scale = HostUARTDevice::instance()->view_scale;                    
                        // if(is_tv)
                        // {                            
                        if(HostUARTDevice::instance()->view_scale == 1)
                            disp_ctrl.SCALING_RATE_TV = SCALING_X2;
                        else if(HostUARTDevice::instance()->view_scale == 3)
                            disp_ctrl.SCALING_RATE_TV = SCALING_X4;
                        else
                            disp_ctrl.SCALING_RATE_TV = SCALING_X1;
                        disp_ctrl.SWITCH_HAPPENED = true;
                        tv_scale = view_scale;
                        // }
                        // else
                        // {                       
                        //     if (HostUARTDevice::instance()->view_scale == 1)
                        //         disp_ctrl.SCALING_RATE_TV = SCALING_X2;
                        //     else
                        //         disp_ctrl.SCALING_RATE_TV = SCALING_X1;
                        //     disp_ctrl.SWITCH_HAPPENED = true;
                        // }
                    }
                }
                //  sdi_aux_info
//#define DEBUG_SDI_FIRST_LINE_SIM_DATA
#ifdef DEBUG_SDI_FIRST_LINE_SIM_DATA
                int i;
                // year
                for(i=0; i<4; i++)
                    sdi_first_line[i] = 't';//HostUARTDevice::instance()->date_str[6+i];
                // month
                for(i=0; i<2; i++)
                    sdi_first_line[4+i] = 'e';//HostUARTDevice::instance()->date_str[3+i];
                // day
                for(i=0; i<2; i++)
                    sdi_first_line[6+i] = 's';//HostUARTDevice::instance()->date_str[i];
                // hour
                for(i=0; i<2; i++)
                    sdi_first_line[8+i] = 't';//HostUARTDevice::instance()->time_str[i];
                // minute
                for(i=0; i<2; i++)
                    sdi_first_line[10+i] = 'd';//HostUARTDevice::instance()->time_str[3+i];
                // second
                for(i=0; i<2; i++)
                    sdi_first_line[12+i] = 't';//HostUARTDevice::instance()->time_str[6+i];
                // milisecond
                intToAscii( &sdi_first_line[0], 14, 100, 3);
                // flight type
                intToAscii( &sdi_first_line[0], 17, 100, 3);
                // flight ID
                intToAscii( &sdi_first_line[0], 20, 66, 4);
                // flight pitch
                intToAscii( &sdi_first_line[0], 24, 90000, 6);
                // flight roll
                intToAscii( &sdi_first_line[0], 30, 45000, 6);
                // flight yaw
                intToAscii( &sdi_first_line[0], 36, 135000, 6);
                // flight longtitude
                for(i=0; i<11; i++)
                    sdi_first_line[42+i] = HostUARTDevice::instance()->aircraft_longitude_str[i];
                // flight latitude
                for(i=0; i<11; i++)
                    sdi_first_line[53+i] = HostUARTDevice::instance()->aircraft_latitude_str[i];
                // flight altitude
                for(i=0; i<6; i++)
                    sdi_first_line[64+i] = HostUARTDevice::instance()->aircraft_altitude_str[i];
                // flight eastward_vel
                intToAscii( &sdi_first_line[0], 70, 5000, 6);
                // flight northward_vel
                intToAscii( &sdi_first_line[0], 76, 5000, 6);
                // flight zenith_vel
                intToAscii( &sdi_first_line[0], 82, 100, 6);
                // sensor type
                intToAscii( &sdi_first_line[0], 88, 35, 3);
                // sensor ID
                intToAscii( &sdi_first_line[0], 91, 1, 4);
                // sensor select
                if(is_tv)
                {
                    intToAscii( &sdi_first_line[0], 95, 4, 3);
                    //tv_view_angle
                    intToAscii( &sdi_first_line[0], 98, HostUARTDevice::instance()->tv_yaw_view_angle, 6);
                    intToAscii( &sdi_first_line[0], 104, HostUARTDevice::instance()->tv_yaw_view_angle*0.5625, 6);
                }
                else
                {
                    intToAscii( &sdi_first_line[0], 95, 2, 3);
                    //ir_view_angle
                    intToAscii( &sdi_first_line[0], 98, HostUARTDevice::instance()->ir_yaw_view_angle, 6);
                    intToAscii( &sdi_first_line[0], 104, HostUARTDevice::instance()->ir_yaw_view_angle*0.8, 6);
                }
                // sensor mode
                intToAscii( &sdi_first_line[0], 110, HostUARTDevice::instance()->work_mode, 3);
                //sensor yaw
                intToAscii( &sdi_first_line[0], 113, HostUARTDevice::instance()->yaw, 6);
                //sensor pitch
                intToAscii( &sdi_first_line[0], 119, HostUARTDevice::instance()->pitch, 6);
                //sensor roll
                intToAscii( &sdi_first_line[0], 125, 0, 6);
                // laser meassure distance
                intToAscii( &sdi_first_line[0], 131, HostUARTDevice::instance()->distance, 5);
                // object longtitude
                for(i=0; i<11; i++)
                    sdi_first_line[136+i] = HostUARTDevice::instance()->object_longitude_str[i];
                // object latitude
                for(i=0; i<11; i++)
                    sdi_first_line[147+i] = HostUARTDevice::instance()->object_latitude_str[i];
                // object altitude
                for(i=0; i<6; i++)
                    sdi_first_line[158+i] = HostUARTDevice::instance()->object_altitude_str[i];
#else
                int i;
                //year
                for(i=0; i<4; i++)
                    sdi_first_line[i] = HostUARTDevice::instance()->date_str[6+i];
                // month
                for(i=0; i<2; i++)
                    sdi_first_line[4+i] = HostUARTDevice::instance()->date_str[3+i];
                // day
                for(i=0; i<2; i++)
                    sdi_first_line[6+i] = HostUARTDevice::instance()->date_str[i];
                // hour
                for(i=0; i<2; i++)
                    sdi_first_line[8+i] = HostUARTDevice::instance()->time_str[i];
                // minute
                for(i=0; i<2; i++)
                    sdi_first_line[10+i] = HostUARTDevice::instance()->time_str[3+i];
                // second
                for(i=0; i<2; i++)
                    sdi_first_line[12+i] = HostUARTDevice::instance()->time_str[6+i];
                // milisecond
                intToAscii( &sdi_first_line[0], 14, 0, 3);
                // flight type 
                intToAscii( &sdi_first_line[0], 17, 0, 3);
                // flight ID 
                intToAscii( &sdi_first_line[0], 20, 0, 4);
                // flight pitch 
                intToAscii( &sdi_first_line[0], 24, HostUARTDevice::instance()->flight_pitch, 6);
                // flight roll 
                intToAscii( &sdi_first_line[0], 30, HostUARTDevice::instance()->flight_roll, 6);
                // flight yaw 
                intToAscii( &sdi_first_line[0], 36, HostUARTDevice::instance()->flight_yaw, 6);
                // flight longtitude 
                intToAscii( &sdi_first_line[0], 42, HostUARTDevice::instance()->aircraft_longitude, 11);
                // flight latitude 
                intToAscii( &sdi_first_line[0], 53, HostUARTDevice::instance()->aircraft_latitude, 11);
                // flight altitude 
                intToAscii( &sdi_first_line[0], 64, HostUARTDevice::instance()->aircraft_altitude, 6);
                // flight eastward_vel 
                intToAscii( &sdi_first_line[0], 70, HostUARTDevice::instance()->eastward_vel, 6);
                // flight northward_vel 
                intToAscii( &sdi_first_line[0], 76, HostUARTDevice::instance()->northward_vel, 6);
                // flight zenith_vel
                intToAscii( &sdi_first_line[0], 82, HostUARTDevice::instance()->zenith_vel, 6);
                // sensor type
                intToAscii( &sdi_first_line[0], 88, 0, 3);
                // sensor ID
                intToAscii( &sdi_first_line[0], 91, 0, 4); 
                // sensor select 
                if(is_tv)
                {
                    intToAscii( &sdi_first_line[0], 95, 4, 3);
                    //tv_view_angle
                    intToAscii( &sdi_first_line[0], 98, HostUARTDevice::instance()->tv_yaw_view_angle, 6);
                    intToAscii( &sdi_first_line[0], 104, HostUARTDevice::instance()->tv_yaw_view_angle*0.5625, 6);  
                }
                else
                {
                    intToAscii( &sdi_first_line[0], 95, 2, 3); 
                    //ir_view_angle
                    intToAscii( &sdi_first_line[0], 98, HostUARTDevice::instance()->ir_yaw_view_angle, 6);
                    intToAscii( &sdi_first_line[0], 104, HostUARTDevice::instance()->ir_yaw_view_angle*0.8, 6);   
                }           
                // sensor mode
                intToAscii( &sdi_first_line[0], 110, HostUARTDevice::instance()->work_mode, 3);
                //sensor yaw
                intToAscii( &sdi_first_line[0], 113, HostUARTDevice::instance()->yaw, 6);
                //sensor pitch
                intToAscii( &sdi_first_line[0], 119, HostUARTDevice::instance()->pitch, 6);
                //sensor roll
                intToAscii( &sdi_first_line[0], 125, HostUARTDevice::instance()->flight_roll, 6);
                // laser meassure distance
                intToAscii( &sdi_first_line[0], 131, HostUARTDevice::instance()->distance, 5);
                // object longtitude 
                intToAscii( &sdi_first_line[0], 136, HostUARTDevice::instance()->object_longitude, 11);
                // object latitude 
                intToAscii( &sdi_first_line[0], 147, HostUARTDevice::instance()->object_latitude, 11);
                // object altitude 
                intToAscii( &sdi_first_line[0], 158, HostUARTDevice::instance()->object_altitude, 6);
#endif

                update_SDI_first_line_info(&sdi_first_line[0]);
//#define debug_sdi_info_
#ifdef debug_sdi_info_
                static int sdi_cnt =0;
                sdi_cnt++;

                if(sdi_cnt%15==0)
				{
					printf("info: ");
					printf("time: ");//year
					for(int i = 0;i<4;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf("-");//mon
					for(int i = 4;i<6;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf("-");//day
					for(int i = 6;i<8;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf("-");//h
					for(int i = 8;i<10;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(":");//min
					for(int i = 10;i<12;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(":");//s
					for(int i = 12;i<14;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(":");//millisecond
					for(int i = 14;i<17;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" flight_type:");//
					for(int i = 17;i<20;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" flight_num:");//
					for(int i = 20;i<24;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" flight_pitch:");//
					for(int i = 24;i<30;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" flight_roll:");//
					for(int i = 30;i<36;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" flight_yaw:");//
					for(int i = 36;i<42;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" f_lon:");//
					for(int i = 42;i<53;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" f_lat:");//
					for(int i = 53;i<64;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" f_altitude:");//
					for(int i = 64;i<70;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" f_e_vel:");//
					for(int i = 70;i<76;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" f_n_vel:");//
					for(int i = 76;i<82;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" f_z_vel:");//
					for(int i = 82;i<88;i++)
					{
						printf("%c",sdi_first_line[i]);
					}

					printf(" sensor_type:");//
					for(int i = 88;i<91;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" sensor_ID:");//
					for(int i = 91;i<95;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" sensor_report:");//
					for(int i = 95;i<98;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" fov_h:");//
					for(int i = 98;i<104;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" fov_v:");//
					for(int i = 104;i<110;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" work_mode:");//
					for(int i = 110;i<113;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" sensor_yaw:");//
					for(int i = 113;i<119;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" sensor_pitch:");//
					for(int i = 119;i<125;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" sensor_roll:");//
					for(int i = 125;i<131;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" lazer_dis:");//
					for(int i = 131;i<136;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" obj_lon:");//
					for(int i = 136;i<147;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" obj_lat:");//
					for(int i = 147;i<158;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf(" obj_altitude:");//
					for(int i = 158;i<164;i++)
					{
						printf("%c",sdi_first_line[i]);
					}
					printf("\n");
				}

#endif          // end of sdi_aux_info

                if( (track_command == 1) && (multi_target_prompt != 2) )
                {
                    if(HostUARTDevice::instance()->report_multi_target_info)
                    {
                        uint8_t params[21] = {0};
                        int byte_index = 0;
                        uint8_t target_num = std::min(5, target_count);
                        STRUCT_DATA_LE(params, &target_num);
                        for(int i = 0; i < 5; ++i)
                        {
                            if(targets_[i]->cross.info.cols_hor_x == 0)
                            {
                                uint16_t x = 0;
                                STRUCT_DATA_LE(params, &x);
                                uint16_t y = 0;
                                STRUCT_DATA_LE(params, &y);
                            }
                            else
                            {
                                uint16_t x = targets_[i]->cross.info.cols_hor_x;
                                STRUCT_DATA_LE(params, &x);
                                uint16_t y = targets_[i]->cross.info.rows_ver_y;
                                STRUCT_DATA_LE(params, &y);                                    
                            }
                        }
                        HostUARTDevice::instance()->write_command_new(0x0B, SIZEOF(params), params, HostUARTDevice::instance()->write_data, &HostUARTDevice::instance()->write_data_length);
                    }
                }    
                video_info_DISP_OUT.ready = false;
            }
// ********************end: display control using device->HostUARTDevice::instance()  **************************************//

// *************************  start: other interrupt control  **************************************//
            // static int disp_out_cnt = 0;
            if (video_info_TV_IN.ready)
            {   
                video_info_TV_IN.ready = false;
            }

            if (video_info_IR_IN.ready)
            {
                video_info_IR_IN.ready = false;
            }

            if (video_info_MIPI_OUT.ready)
            {
                xj3_send_command = false;
                if(is_tv != (HostUARTDevice::instance()->channel == 1))
                {
                    is_tv = (HostUARTDevice::instance()->channel == 1);
                    mipi_video_ctrl.switch_channel = true;
                    mipi_video_ctrl.is_TV = !is_tv; 
                    channel_changed = true;  
                    count_ch_switch = mipi_out_cnt;
                    // uint8_t params[1] = {HostUARTDevice::instance()->channel};
                    // HostUARTDevice::instance()->write_command(0x06, SIZEOF(params), params);
                    printf("switch mipi channel!\n");
                }

                if ((cross_x_main != HostUARTDevice::instance()->cross_x_main) || (cross_y_main != HostUARTDevice::instance()->cross_y_main) || HostUARTDevice::instance()->cross_main_send)
                {
                    HostUARTDevice::instance()->cross_main_send = false;
                    if((cross_x_main != HostUARTDevice::instance()->cross_x_main) || (cross_y_main != HostUARTDevice::instance()->cross_y_main))
                    {
                        cross_main_changed = true;
                        // printf("corss_main_x:%d, corss_main_y:%d.\n", HostUARTDevice::instance()->cross_x_main, HostUARTDevice::instance()->cross_y_main);
                    }
                    cross_x_main = HostUARTDevice::instance()->cross_x_main;
                    cross_y_main = HostUARTDevice::instance()->cross_y_main;
                    if(is_tv)
                    {
                        if(HostUARTDevice::instance()->sensor_view_size == 0x04)
                        {
                            cross_x_main_tv_z = cross_x_main;
                            cross_y_main_tv_z = cross_y_main;
                            // printf("tv_sn_x:%d, tv_sn_y:%d.\n",cross_x_main_tv_z,cross_y_main_tv_z);
                        }
                        // else if(HostUARTDevice::instance()->sensor_view_size == 0x05)
                        // {
                        //     cross_x_main_tv_s = cross_x_main;
                        //     cross_y_main_tv_s = cross_y_main;
                        //     printf("tv_z_x:%d, tv_z_y:%d.\n",cross_x_main_tv_s,cross_y_main_tv_s);                            
                        // }
                        // else if(HostUARTDevice::instance()->sensor_view_size == 0x03)
                        // {
                        //     cross_x_main_tv_m = cross_x_main;
                        //     cross_y_main_tv_m = cross_y_main;
                        //     printf("tv_s_x:%d, tv_s_y:%d.\n",cross_x_main_tv_m,cross_y_main_tv_m); 
                        // }
                    }
                    cross_Main.cross_info.info.cross_hor = cross_x_main;
                    cross_Main.cross_info.info.cross_ver = cross_y_main;
                    GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_MAIN>>2] = cross_Main.cross_info.send;
                    cross_main_send = true;
                }

                if(HostUARTDevice::instance()->sensor_view_changed)
                {
                    HostUARTDevice::instance()->sensor_view_changed = false;
                    if(is_tv)
                    {
                        if( (tv_view_size == 0x03) && (HostUARTDevice::instance()->sensor_view_size == 0x04) )
                        {
                            count_electron = mipi_out_cnt;
                            tv_zoom_status = 1;
                        }
                        else if( (tv_view_size == 0x04) && (HostUARTDevice::instance()->sensor_view_size == 0x05) )
                        {
                            count_electron = mipi_out_cnt;
                            tv_zoom_status = 2;
                        }
                        else if( (tv_view_size == 0x04) && (HostUARTDevice::instance()->sensor_view_size == 0x03) )
                        {
                            count_electron = mipi_out_cnt;
                            tv_zoom_status = 3;
                        }
                        else if( (tv_view_size == 0x05) && (HostUARTDevice::instance()->sensor_view_size == 0x04) )
                        {
                            count_electron = mipi_out_cnt;
                            tv_zoom_status = 4;
                        }
                        else 
                        {
                            tv_zoom_status = tv_zoom_status;  
                        }
                        tv_view_size = HostUARTDevice::instance()->sensor_view_size;
                        // printf("tv_view_size:%d, tv_status:%d.\n", tv_view_size, tv_zoom_status);
                    }
                    else
                    {
                        // if( (ir_view_size == 0x03) && (HostUARTDevice::instance()->sensor_view_size == 0x04) )
                        // {
                        //     ir_zoom_status = 1;
                        // }
                        // else 
                        if( (ir_view_size == 0x04) && (HostUARTDevice::instance()->sensor_view_size == 0x05) )
                        {
                            count_electron = mipi_out_cnt;
                            ir_zoom_status = 2;
                        }
                        // else if( (ir_view_size == 0x04) && (HostUARTDevice::instance()->sensor_view_size == 0x03) )
                        // {
                        //     ir_zoom_status = 3;
                        // }
                        else if( (ir_view_size == 0x05) && (HostUARTDevice::instance()->sensor_view_size == 0x04) )
                        {
                            count_electron = mipi_out_cnt;
                            ir_zoom_status = 4;
                        }
                        else 
                        {
                            ir_zoom_status = ir_zoom_status;
                        }
                        ir_view_size = HostUARTDevice::instance()->sensor_view_size;
                        // printf("ir_view_size:%d, ir_status:%d.\n", ir_view_size, ir_zoom_status);
                    }
                }
                // enhance_level_change message for XJ3 reinit tracker 
                if(enhance_level_changed)
                {
                    enhance_level_changed = false;
                    if(is_tv)
                    {
                        if(HostUARTDevice::instance()->sensor_view_size < 4)
                        {
                            if(enhance_level_tv != HostUARTDevice::instance()->enhance_level)
                            {
                                enhance_level_tv = HostUARTDevice::instance()->enhance_level;
                                XJ3UARTDevice::instance()->write_command_control(0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                        }
                        else
                        {
                            if(enhance_level_zoom != HostUARTDevice::instance()->enhance_level)
                            {
                                enhance_level_zoom = HostUARTDevice::instance()->enhance_level;
                                XJ3UARTDevice::instance()->write_command_control(0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                            }
                        }
                        xj3_send_command = true;
                    } 
                    else
                    {
                        if(enhance_level_ir != HostUARTDevice::instance()->enhance_level)
                        {
                            enhance_level_ir = HostUARTDevice::instance()->enhance_level;
                            count_ir_enhance = mipi_out_cnt;
                            // XJ3UARTDevice::instance()->write_command_control(0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                        }                    
                    } 
                    // xj3_send_command = true;
                } 
                if(mipi_out_cnt == (count_ir_enhance + 6))
                {
                    XJ3UARTDevice::instance()->write_command_control(0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                    xj3_send_command = true;
                    count_ir_enhance = -100;
                }                
                // channel changed message for XJ3 reinit tracker 
                if( channel_changed && (mipi_out_cnt == (count_ch_switch+3)))
                {
                    cross_main_send = false;
                    cross_main_changed = false;
                    channel_changed = false;
                    if(is_tv)
                    {
                        if(tv_scale != 1)
                        {
                            XJ3UARTDevice::instance()->write_command_control(0x1F, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->tv_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->tv_yaw_view_angle & 0xFF), cross_x_main, cross_y_main, 0x02);
                        }
                        else
                        {
                            XJ3UARTDevice::instance()->write_command_control(0x1F, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->tv_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->tv_yaw_view_angle & 0xFF), 0xFFFF, 0xFFFF, 0xFF);                          
                        }
                    }
                    else
                    {                  
                        XJ3UARTDevice::instance()->write_command_control(0x2F, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->ir_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->ir_yaw_view_angle & 0xFF), (cross_x_main-320)/2, (cross_y_main-28)/2, 0x03);
                    }
                    xj3_send_command = true;
                    printf("J3 receive channel_changed:%d,cross_x:%d,cross_y:%d \n", HostUARTDevice::instance()->channel, cross_x_main, cross_y_main); 
                    // count_ch_switch = -10;
                } 
                if(mipi_out_cnt == (count_ch_switch+6)) 
                {
                    if(is_tv != (XJ3UARTDevice::instance()->channel == 0))
                    {
                        if(is_tv)
                        {
                            if(tv_scale != 1)
                            {
                                XJ3UARTDevice::instance()->write_command_control(0x1F, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->tv_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->tv_yaw_view_angle & 0xFF), cross_x_main, cross_y_main, 0x02);
                            }
                            else
                            {
                                XJ3UARTDevice::instance()->write_command_control(0x1F, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->tv_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->tv_yaw_view_angle & 0xFF), 0xFFFF, 0xFFFF, 0xFF);                           
                            }
                        }
                        else
                        {                                  
                            XJ3UARTDevice::instance()->write_command_control(0x2F, 0xFF, 0xFF, (uint8_t)((HostUARTDevice::instance()->ir_yaw_view_angle >> 8) & 0xFF), (uint8_t)(HostUARTDevice::instance()->ir_yaw_view_angle & 0xFF), (cross_x_main-320)/2, (cross_y_main-28)/2, 0x03);
                        }
                        xj3_send_command = true; 
                        // printf("J3 receive channel_changed_2:%d \n", HostUARTDevice::instance()->channel); 
                    }                    
                    // count_ch_switch = -10;                  
                }
                // cross main message for XJ3 relocate tracker
                if(cross_main_send && !channel_changed)
                {
                    cross_main_send = false;
                    if(is_tv)
                    {
                        if((tv_zoom_status != 1) && (tv_zoom_status != 3))
                        {
                            if((tv_scale != 1) && cross_main_changed)
                            {
                                // cross_main_changed = false;
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, cross_x_main, cross_y_main, 0x02);
                                xj3_send_command = true;
                            }
                        }     
                    }           
                    else
                    {
                        if((ir_zoom_status != 2) && (ir_zoom_status != 4))
                        {
                            if(cross_main_changed)
                            {
                                // cross_main_changed = false;
                                XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, (cross_x_main-320)/2, (cross_y_main-28)/2, 0x03);
                                xj3_send_command = true;
                            }
                        }
                    }
                    cross_main_changed = false;
                    // xj3_send_command = true;
                }

                // if(count == (count_electron + 4) && (tv_zoom_status != 0))
                // {
                //     tv_zoom_status = 0;
                // }
                if(mipi_out_cnt == (count_electron + 4))
                {
                    switch(tv_zoom_status)
                    {
                        case 1:
                        {
                            printf("tv electron open.\n");
                            XJ3UARTDevice::instance()->write_command_control(0x5F, 0xFF, 0xFF, 0xFF, 0xFF, cross_x_main, cross_y_main, 0x02);
                            tv_zoom_status = 0;
                            xj3_send_command = true;
                        }
                            break;
                        case 2:
                        {
                            tv_zoom_status = 0;
                        }
                            break;
                        case 3:
                        {
                            printf("tv electron close.\n");
                            XJ3UARTDevice::instance()->write_command_control(0x6F, 0xFF, 0xFF, 0xFF, 0xFF, cross_x_main, cross_y_main, 0x02);  
                            tv_zoom_status = 0;
                            xj3_send_command = true;
                        }
                            break;
                        case 4:
                        {
                            tv_zoom_status = 0;
                        }
                            break;
                        default:
                            tv_zoom_status = 0;
                            break;
                    }
                }
                // if(count == (count_electron + 5) && (ir_zoom_status != 0))
                // {
                //     ir_zoom_status = 0;
                // }
                if(mipi_out_cnt == (count_electron + 4))
                {
                    switch(ir_zoom_status)
                    {
                        case 1:
                        {
                            ir_zoom_status = 0;
                        }
                            break;
                        case 2:
                        {
                            printf("ir electron open.\n");
                            XJ3UARTDevice::instance()->write_command_control(0x5F, 0xFF, 0xFF, 0xFF, 0xFF, (cross_x_main-320)/2, (cross_y_main-28)/2, 0x03);
                            ir_zoom_status = 0;
                            xj3_send_command = true;
                        }
                            break;
                        case 3:
                        {
                            ir_zoom_status = 0;
                        }
                            break;
                        case 4:
                        {
                            printf("ir electron close.\n");
                            XJ3UARTDevice::instance()->write_command_control(0x6F, 0xFF, 0xFF, 0xFF, 0xFF, (cross_x_main-320)/2, (cross_y_main-28)/2, 0x03);
                            ir_zoom_status = 0;
                            xj3_send_command = true;
                        }
                            break;
                        default:
                            ir_zoom_status = 0;
                            break;
                    }
                }
                if(track_command != 1)
                {
                    if(tv_mode_change_start)
                    {
                        tv_mode_change_start = false;
                        XJ3UARTDevice::instance()->write_command_control(0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                        xj3_send_command = true;
                    }
                    if(tv_mode_change_end)
                    {
                        tv_mode_change_end = false;
                        XJ3UARTDevice::instance()->write_command_control(0x4F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF); 
                        xj3_send_command = true;
                    }
                }  
                else
                {
                    tv_mode_change_start = false;
                    tv_mode_change_end = false;
                }      
                video_info_MIPI_OUT.ready = false;
            }     
// ************************* end: other interrupt control  **************************************//
        #endif
        static int unnormal_tv_in_time_gap_cnt = 0;
        static int unnormal_ir_in_time_gap_cnt = 0;
        static int unnormal_disp_out_time_gap_cnt = 0;
        static int unnormal_mipi_out_time_gap_cnt = 0;
#ifdef 	TEST_TV_IN_TIME_STABLIZATION			//added by fengyachun 20230210 to check input posedge time gap of TV or IR VS signal
//    	if((enable_print_time_gap_tv_in==true) && ((disp_out_cnt%10)==0))
//    	{
//    		static bool print_time_gap_enable = true;
//    		if(((tv_in_time_gap<33.3) || (tv_in_time_gap>33.4)) && (video_status.config.para.status_TV == 1))
//    		{
//    			unnormal_tv_in_time_gap_cnt++;
//    			print_time_gap_enable = true;
//    		}
//
//
//    		if(((disp_out_time_gap<33.3) || (disp_out_time_gap>33.4)) && (video_status.config.para.status_TV == 1))
//    		{
//    			print_time_gap_enable = true;
//    		}
//    		if(((mipi_out_time_gap<16.6) || (mipi_out_time_gap>16.7)) && (video_status.config.para.status_IR == 1))
//    		{
//    			print_time_gap_enable = true;
//    		}
//
//    		if(print_time_gap_enable == true)
//    		{
//    			printf("t_i_g_t: %f  i_i_g_t: %f  t_cnt: %d  i_cnt: %d\n ",tv_in_time_gap,ir_in_time_gap,tv_in_cnt,ir_in_cnt);
//    			printf("d_o_t_g: %f  m_o_t_g: %f \n ",disp_out_time_gap, mipi_out_time_gap);
//    			print_time_gap_enable = false;
//    		}
//    	}
        if(enable_print_time_gap_tv_in==true)
		{
			if(((tv_in_time_gap<33.3) || (tv_in_time_gap>33.4)) && (video_status.config.para.status_TV == 1))
			{
				unnormal_tv_in_time_gap_cnt++;
			}

			if(((disp_out_time_gap<33.3) || (disp_out_time_gap>33.4)) && (video_status.config.para.status_TV == 1))
			{
				unnormal_disp_out_time_gap_cnt++;
			}
			if(((mipi_out_time_gap<16.6) || (mipi_out_time_gap>16.7)) && (video_status.config.para.status_IR == 1))
			{
				unnormal_mipi_out_time_gap_cnt++;
			}

			enable_print_time_gap_tv_in = false;
		}
    	if(enable_print_time_gap_ir_in==true)
    	{
    		if(((ir_in_time_gap<16.6) || (ir_in_time_gap>16.7)) && (video_status.config.para.status_IR == 1))
			{
    			unnormal_ir_in_time_gap_cnt++;
			}
    		enable_print_time_gap_ir_in = false;
    	}

#endif

#ifdef PHOTO_IN_DVP_OUT
    	if(photo_info_PHOTO_IN.ready==true)
    	{
    		cnt_photo_in++;
    		photo_info_PHOTO_IN.ready = false;
    	}
    	if(photo_info_DVP_OUT.ready==true)
		{
			cnt_dvp_out++;
			photo_info_DVP_OUT.ready = false;

//			int cur_write_mem = FrmWR_ID_PHOTO*3*5120*4096;
//
//			int _idx_1_ = (FrmWR_SUB_ID_PHOTO_DVP_OUT%2) ? (((FrmWR_SUB_ID_PHOTO_DVP_OUT+1)/2) - 1) : (FrmWR_SUB_ID_PHOTO_DVP_OUT/2);
//			int _idx_2_ = (FrmWR_SUB_ID_PHOTO_DVP_OUT%2);
//			int _cur_read_mem = _idx_1_*15728640 + _idx_2_*7680;
//			if(_cur_read_mem-cur_write_mem<3*5120*4096)
//			{
//				printf("pt_id: %d dvp_sub_id: %d\n",FrmWR_ID_PHOTO,FrmWR_SUB_ID_PHOTO_DVP_OUT);
//			}

		}

#endif

#ifdef DEBUG_MIPI_IR_DELAY
        if(print_diff_time==true)
        {
            mipi_ir_cnt++;
            if ((mipi_ir_cnt%900)==0)
            {
    			int temp = XADC_HANDLE[0x200>>2];
    			float temp_lsb= ((((float)(temp)/65536.0f)/0.00198421639f ) - 273.15f);//XADC_obj.data.sub_data.lsb_temp;
#ifdef DEBUG_STD_LENGTH_ERR
            	printf("Sys info: i_h: %d i_j3 %d t_i %d i_i %d d_o %d m_o %d c_t: %f m_C %d t_i_g: %d i_i_g %d d_o_g %d m_o_g %d k_n %d k_eof %d pt_in: %d dvp_o %d pt_sts %d\n ",intr_cnt_uart_host,intr_cnt_uart_xj3,tv_in_cnt,ir_in_cnt, disp_out_cnt, mipi_out_cnt,temp_lsb,mismatched_crc_cnt,unnormal_tv_in_time_gap_cnt,unnormal_ir_in_time_gap_cnt,unnormal_disp_out_time_gap_cnt,unnormal_mipi_out_time_gap_cnt,key_enter_num,key_EOF_num,cnt_photo_in,cnt_dvp_out,video_status.config.para.status_send_photo);
#endif
#ifndef DEBUG_STD_LENGTH_ERR
            	printf("System OK! \n ");
#endif

            }
        	print_diff_time = false;
        }
#endif
#ifdef 	DVP_OUT_STAB_TEST
        static bool dvp_time_gap_print_rst = true;
        static int dvp_out_time_gap_cnt = 0;
        if(((abs(dvp_out_time_gap)>42.0) || (abs(dvp_out_time_gap)<41.0) ) && (dvp_time_gap_print_rst && lock_dvp_start))
        {
        	dvp_out_time_gap_cnt++;
        	printf("dvp_out_time_gap: %f; cnt: %d \n",dvp_out_time_gap,dvp_out_time_gap_cnt);
        	dvp_time_gap_print_rst = false;
        }
        static int dvp_cnt_test = 0;
        if(enable_print_time_gap_dvp_out==true)
	   {
        	dvp_time_gap_print_rst = true;
        	dvp_cnt_test++;
//        	if(dvp_cnt_test%3==0)
//        	{
//            	printf("pt_gap: %f dvp_gap: %f dvp_pt: %f \n",pt_in_time_gap,dvp_out_time_gap,pt_dvp_time_gap);
//        	}
        	if(dvp_cnt_test%1==0)
			{
				//printf("dvp_gap: %f \n",dvp_out_time_gap);
			}
        	enable_print_time_gap_dvp_out = false;
	   }
#endif
#ifdef SUM_DELAY
       	if(video_info_SUM_delay.ready == true)
		{
//       		float start = omp_get_wtime();
       		video_info_SUM_delay.ready = false;
       		memcpy(img_for_delay.data,video_info_SUM_delay.img_buf,160*160);

//		#pragma omp parallel sections
//		{
//			#pragma omp section
//			{
//				memcpy(img_TV_spot_pic.data+0*1920*3*270,video_info_SUM_delay.img_buf+0*1920*3*270,1920*270*3);
//			}
//			#pragma omp section
//			{
//				memcpy(img_TV_spot_pic.data+1*1920*3*270,video_info_SUM_delay.img_buf+1*1920*3*270,1920*270*3);
//			}
//			#pragma omp section
//			{
//				memcpy(img_TV_spot_pic.data+2*1920*3*270,video_info_SUM_delay.img_buf+2*1920*3*270,1920*270*3);
//			}
//			#pragma omp section
//			{
//				memcpy(img_TV_spot_pic.data+3*1920*3*270,video_info_SUM_delay.img_buf+3*1920*3*270,1920*270*3);
//			}
//		}
////       		memcpy(img_TV_spot_pic.data,video_info_SUM_delay.img_buf,1920*1080*3);
//
//       		float end = omp_get_wtime();;
//       		printf("1080P memcpy time: %f ms \n", 1000.0*(end-start));
		}
#endif

#ifdef open_main_loop_time_monitor
        end = std::chrono::system_clock::now();
        double _during = (float)std::chrono::duration_cast<std::chrono::microseconds> (end - start).count();
        printf("Loop Interval: %f ms \n", _during/1000.0);
#endif
    }

    return 0;
}
#ifdef TV_ALIGN_ON_FMQL
// void fly_axis_crc(bool &res, unsigned char *ptrImg_buf, float &center_hor, float &center_ver)
// {
// 	float sum_hor_1 = 0,sum_hor_2 = 0,	sum_hor_3 = 0,	sum_hor_4 = 0;
// 	float sum_ver_1 = 0,sum_ver_2 = 0,	sum_ver_3 = 0,	sum_ver_4 = 0;
// 	float sum_Y_1 = 0,	sum_Y_2 = 0,	sum_Y_3 = 0,	sum_Y_4 = 0;
// 	#pragma omp parallel sections
// 	{
// 		#pragma omp section
// 		{
// 			for(int r = 220; r<380;r++)
// 			{
// 				for(int c = 640; c<1280; c++)
// 				{
// 					if(ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2] > 64)
// 					{
// 						sum_Y_1 = sum_Y_1 + ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_hor_1 = sum_hor_1 + c*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_ver_1 = sum_ver_1 + r*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 					}
// 				}
// 			}
// 		}
// 		#pragma omp section
// 		{
// 			for(int r = 380; r<540;r++)
// 			{
// 				for(int c = 640; c<1280; c++)
// 				{
// 					if(ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2] > 64)
// 					{
// 						sum_Y_2 = sum_Y_2 + ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_hor_2 = sum_hor_2 + c*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_ver_2 = sum_ver_2 + r*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 					}
// 				}
// 			}
// 		}
// 		#pragma omp section
// 		{
// 			for(int r = 540; r<700;r++)
// 			{
// 				for(int c = 640; c<1280; c++)
// 				{
// 					if(ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2] > 64)
// 					{
// 						sum_Y_3 = sum_Y_3 + ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_hor_3 = sum_hor_3 + c*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_ver_3 = sum_ver_3 + r*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 					}
// 				}
// 			}
// 		}
// 		#pragma omp section
// 		{
// 			for(int r = 700; r<860;r++)
// 			{
// 				for(int c = 640; c<1280; c++)
// 				{
// 					if(ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2] > 64)
// 					{
// 						sum_Y_4 = sum_Y_4 + ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_hor_4 = sum_hor_4 + c*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 						sum_ver_4 = sum_ver_4 + r*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 					}
// 				}
// 			}
// 		}
// 	}
// 	float _center_hor = (sum_hor_1+sum_hor_2+sum_hor_3+sum_hor_4)/(sum_Y_1+sum_Y_2+sum_Y_3+sum_Y_4+0.001);
// 	float _center_ver = (sum_ver_1+sum_ver_2+sum_ver_3+sum_ver_4)/(sum_Y_1+sum_Y_2+sum_Y_3+sum_Y_4+0.001);
// 	int center_hor_temp = max(max((int)_center_hor,640),min((int)_center_hor,1280));
// 	int center_ver_temp = max(max((int)_center_ver,220),min((int)_center_ver,860));
// 	sum_Y_4 = 0;
// 	for(int r = center_ver_temp-5; r<center_ver_temp+5;r++)
// 	{
// 		for(int c = center_hor_temp-5; c<center_hor_temp+5; c++)
// 		{
// 			sum_Y_4 = sum_Y_4 + ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 			sum_hor_4 = sum_hor_4 + c*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 			sum_ver_4 = sum_ver_4 + r*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
// 		}
// 	}
// //    printf("src hor, src ver, sum_Y_4: %f %f %f.\n", _center_hor, _center_ver, sum_Y_4);
// 	if(sum_Y_4>100*64)
// 	{
// 		center_hor = _center_hor;
// 		center_ver = _center_ver;
// 		printf("%f\n",sum_Y_4);
// 		res = true;
// 	}
// 	else
// 	{
// 		center_hor = 960;
// 		center_ver = 540;
// 		res = false;
// 	}
// }
void fly_axis_crc(bool &res, unsigned char *ptrImg_buf, float &center_hor, float &center_ver)
{

	float sum_hor = 0;
	float sum_ver = 0;
	float sum_Y = 0;

    for(int r = 380; r<700; r++)
    {
        for(int c = 800; c<1120; c++)
        {
            if(ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2] > 64)
            {
                sum_Y = sum_Y + ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
                sum_hor = sum_hor + c*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
                sum_ver = sum_ver + r*ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
            }
        }
    }

	float _center_hor = sum_hor/(sum_Y + 0.001);
	float _center_ver = sum_ver/(sum_Y + 0.001);
	int center_hor_temp = max(max((int)_center_hor,800),min((int)_center_hor,1120));
	int center_ver_temp = max(max((int)_center_ver,380),min((int)_center_ver,700));
	sum_Y = 0;
	for(int r = center_ver_temp-5; r<center_ver_temp+5;r++)
	{
		for(int c = center_hor_temp-5; c<center_hor_temp+5; c++)
		{
			sum_Y = sum_Y + ptrImg_buf[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 3*c+2];
		}
	}
//    printf("src hor, src ver, sum_Y: %f %f %f.\n", _center_hor, _center_ver, sum_Y);
	if(sum_Y>100*64)
	{
		center_hor = _center_hor;
		center_ver = _center_ver;
		printf("%f\n",sum_Y);
		res = true;
	}
	else
	{
		center_hor = 960;
		center_ver = 540;
		res = false;
	}
//	if(center_hor>1920)
//		center_hor = 960.0;
//	if(center_ver>1080)
//		center_ver = 540.0;
}
#endif
