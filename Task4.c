///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//NAME : MUHAMMAD AKMAL BIN MOHD ADZUDDIN
//ID : 25350
//TASK : TASK 4
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DELAY_TIME 300000
#define osObjectsPublic
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "osObjects.h"                           //     Include necessary libraries to run:
#include "stdio.h"                               //     
#include "stdlib.h"                              //     1. Seven Segment Display
#include "math.h"                                //     2. 3x3 Keypad
#include "string.h"                              //     3. LCD
#include "sys.c"                                 //     4. Variable Resistor
#include "clk.c"                                 //
#include "gpio.c"                                //     and common C functions such as abs(), string...
#include "spi.c"
#include "ADC.c"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Seven_Segment_Display.c"               //     Device libraries: Seven Segment Display, Keypad, LCD and 
#include "Keypad.c"                              //                       Variable Resistor.
#include "lcd.c"
#include "Variable_Resistor.c"                   
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern int Init_Thread(void);                    //     Thread Initialization function call and constants declarations.
 
int digit, value, Keypad_Value, Keypad_Value_Diff, Keypad_Value_Old=2, adc_diff, fill, DispSep1, DispSep2, DispSep3, VRPercent;
float adc_old = 2, VRValue=0;

char VolBar[15] = "Volume: ";
char KeyVal[15] = "Keypad:   ";


// EXTRA FUNCTION INCLUDED IN SEVEN_SEGMENT_DISPLAY.C

void seven_segment_value_input_Keypad(int integer_input)                        //   Input Receive:
{                                                                        //
	int integer_digit, block_state;                                        //   1. Any value to be displayed in integer 
	            	
	integer_digit = integer_input%10/1;
	if (integer_input>=0) block_state=1; else block_state=0;
	seven_segment_digit_input(3,integer_digit,block_state);
}

//TASK 4///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void thread1()                                   //     Thread function declarations.
{
	seven_segment_begin();                         //     Start Seven Segment Display. This function covers segment block
}                                                //     power switching and segments to be display for each digits. This     
void thread2()                                   //     function should be run individually in one thread.
{
	while(1)
	{
		Keypad_Value = Keypad_Return();	             //     Keypad_Return(): 3x3 Keypad value returned in integer data type.
		VRValue = Variable_Resistor_Value();         //     Variable_Resistor_Value(): On-board Variable Resistor (VR1)
	}                                              //                                analog value returned in int32_t data 
}                                                //                                type. (0~255)

void thread3()
{
	while(1)
	{		
		VRPercent = (VRValue/255)*100;
				
		digit = VRPercent%1000/100;
		DispSep3=digit;
		
		digit = VRPercent%100/10;
		DispSep2=digit;
		
		digit = VRPercent%10/1;;
		DispSep1=digit;
		
//	DrvGPIO_ClrBit(E_GPC,4);
//	DrvGPIO_ClrBit(E_GPC,5);
//	DrvGPIO_ClrBit(E_GPC,6);	
		//seven_segment_value_input(VRValue*3.917647059);          //     seven_segment_value_input(x): X is an integer range from (0~9999) which
		seven_segment_value_input_Keypad(Keypad_Value);

		if (VRPercent>=25) PC12=0;        
		if (VRPercent>=50) PC13=0;                   //     PC (12~15) are the RED LEDs located at Port C Pin 12 ~ 15.
		if (VRPercent>=75) PC14=0;
		if (VRPercent>=100) PC15=0;
		if (VRPercent>=33) PA12=0;                   //     PA (12~14) are the RGB LEDs located at Port A Pin 12 ~ 14.
		if (VRPercent>=66) PA13=0;                 
		if (VRPercent>=99) PA14=0;
		
		if (VRPercent>=100) VolBar[8]= DispSep3 + 0x30; else VolBar[8]= ' ';
		if (VRPercent>=10) VolBar[9]= DispSep2 + 0x30; else VolBar[9]= ' ';
		VolBar[10]= DispSep1 + 0x30;
		VolBar[11]= '%';
		KeyVal[10] = Keypad_Value + 0x30;
		
		adc_diff = adc_old - VRValue;
		Keypad_Value_Diff = Keypad_Value_Old - Keypad_Value;
	}
}

void thread4()                                          //     Graphical LCD takes time to render individual pixel. Kindly
{                                                       //     allocate one thread for LCD to display content to prevent
	while(1)                                              //     interupts or delay on other device interfacing.
	{
		RectangleDraw(0,15,127,30, FG_COLOR, BG_COLOR);     //     RectangleDraw/Fill (X1, Y1, X2, Y2, Foreground, Background).
		if (abs(adc_diff)>0||abs(Keypad_Value_Diff)>0)      //     A Graphical LCD function used to display Rectangle Shapes.
		{
			PC12=PC13=PC14=PC15=1;                            //     Reset RED LEDs to OFF state.
			PA12=PA13=PA14=1;                                 //     Reset RGB LEDs to OFF state.
			adc_old = VRValue;
			fill = (adc_old/255)*127;
			RectangleFill(0,16,fill,29,FG_COLOR, BG_COLOR);
			RectangleFill(127,16,fill+1,29,BG_COLOR, FG_COLOR);
			print_Line(2, VolBar);                            //     Print_Line(Line Number, Content). A Graphical LCD function used 
			Keypad_Value_Old=Keypad_Value;                    //     to display TEXT. Line Number must be in integer data type and 
			print_Line(3, KeyVal);                            //     Content must be in string/ char data type.
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
osThreadDef(thread1,osPriorityNormal,1,0);              //     Thread Priority define.
osThreadDef(thread2,osPriorityNormal,1,0);
osThreadDef(thread3,osPriorityNormal,1,0);
osThreadDef(thread4,osPriorityNormal,1,0);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t main (void) 
{
	SYS_UnlockReg();
	CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);           //     Enable clock source -> XTL12M. 12 MHz.
	CLK->CLKSEL0|=CLK_CLKSEL0_HCLK_S_Msk;                 //     Select 12MHz clock as Host Clock.
	SYS_LockReg();
	
  osKernelInitialize ();
	
	InitVRADC();                                          // Initialize Variable Resistor ADC, LCD.
	init_LCD();
	clear_LCD();
	
	Init_Thread();                                        // Initialize thread.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	osThreadCreate(osThread(thread1),NULL);               // Allocating thread locations for specific thread function.
	osThreadCreate(osThread(thread2),NULL);
	osThreadCreate(osThread(thread3),NULL);
	osThreadCreate(osThread(thread4),NULL);
  osKernelStart ();                                     // Start thread.
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
