/***************************************************************************************************
                                   ExploreEmbedded	
 ****************************************************************************************************
 * File:   lcd.c
 * Version: 15.1
 * Author: ExploreEmbedded
 * Website: http://www.exploreembedded.com/wiki
 * Description: File contains the Library routines for Alpha Numeric LCD

The libraries have been tested on ExploreEmbedded development boards. We strongly believe that the 
library works on any of development boards for respective controllers. However, ExploreEmbedded 
disclaims any kind of hardware failure resulting out of usage of libraries, directly or indirectly.
Files may be subject to change without prior notice. The revision history contains the information 
related to updates. 


GNU GENERAL PUBLIC LICENSE: 
    Copyright (C) 2012  ExploreEmbedded

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


Errors and omissions should be reported to codelibraries@exploreembedded.com
 **************************************************************************************************/




/***************************************************************************************************
                             Revision History
 ***************************************************************************************************
15.0: Initial version 
15.1: Updated the LCD_DisplayNumber function to display Bin/Dec/Hex numbers.
      Removed the functions LCD_DisplayHexNumber and LCD_DisplayBinaryNumber.
      Changed the structure prefix from STK to STR.
***************************************************************************************************/


/***************************************************************************************************
                                   2x16 LCD internal structure
****************************************************************************************************
    
            |<--------------------16 chars on Each Line-------------------->|
    	   ____________________________________________________________________	
          |\                                                                  /|
          |	\ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___/ |
          |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | |
    Line1 |  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B | C | D | E | F | |
          |  |___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___| |
          |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | |
    Line2 |  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B | C | D | E | F | |
          |  |___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___| |
    	  | /                                                                \ |
    	  |/__________________________________________________________________\|
    	                | D7 D6 D5 D4 D3 D2 D1 D0 |     EN   RW   RS          
    		            |<------Lcd Data Bus----->|    Lcd control Lines
    		            |	                      |	      
***************************************************************************************************/								   
#include <stdarg.h>
#include "delay.h"
#include "lcd.h"



/***************************************************************************************************
                          Global Variables and Structures
 ***************************************************************************************************/
uint8_t VAR_LcdTrackLineNum_U8;         //Variable to track the line numbers
uint8_t VAR_LcdTrackCursorPos_U8;       //Variable to track the cursor
LcdConfig_st STR_LCDConfig;             //Structure containing the selected LCD Configuration
uint8_t ARR_LcdLineNumAddress_U8[]={0x80,0xc0,0x90,0xd0};
/**************************************************************************************************/




/***************************************************************************************************
                            local function prototypes
 ***************************************************************************************************/
static void lcd_DataWrite( uint8_t dat);
static void lcd_BusyCheck(void);
static void lcd_Reset(void);
static void lcd_SendCmdSignals(void);
static void lcd_SendDataSignals(void);
static void lcd_SendHigherNibble(uint8_t dataByte);
static void lcd_SendLowerNibble(uint8_t dataByte);
/**************************************************************************************************/





/**************************************************************************************************
void LCD_SetUp( pin numbers of lcd)
***************************************************************************************************
 * Function name:  LCD_SetUp()
 * I/P Arguments: gpioPins_et RS: Pin where RS is connected 
                  gpioPins_et RW: Pin where RW is connected (P_NC if not connected) 
                  gpioPins_et EN: Pin where EN is connected
                  
                  gpioPins_et D0: Pin where D0 is connected (P_NC if not connected for 4-bit mode)  
                  gpioPins_et D1: Pin where D1 is connected (P_NC if not connected for 4-bit mode)  
                  gpioPins_et D2: Pin where D2 is connected (P_NC if not connected for 4-bit mode)  
                  gpioPins_et D3: Pin where D3 is connected (P_NC if not connected for 4-bit mode) 
                  gpioPins_et D4: Pin where D4 is connected 
                  gpioPins_et D5: Pin where D5 is connected 
                  gpioPins_et D6: Pin where D6 is connected 
                  gpioPins_et D7: Pin where D7 is connected 
 * Return value	: none

 * description  :This function is used to configure the controller pins for LCD operation.
                 Pass the pin numbers where the RS,RW,EN, D0-D7 are connected as parameters.
				 In case of four bit mode pass P_NC as parameter for D0-D3.
				 If RW is not used then pass P_NC as parameter for for RS.
**************************************************************************************************/
void LCD_SetUp(gpioPins_et RS, 
               gpioPins_et RW, 
               gpioPins_et EN,
               gpioPins_et D0, 
               gpioPins_et D1, 
               gpioPins_et D2, 
               gpioPins_et D3,
               gpioPins_et D4,
               gpioPins_et D5,
               gpioPins_et D6,
               gpioPins_et D7 )
{
  /* Copy the PIN numbers at whihc the LCD is connected */
    STR_LCDConfig.LCD_RS = RS;
    STR_LCDConfig.LCD_RW = RW;
    STR_LCDConfig.LCD_EN = EN;

    STR_LCDConfig.LCD_D0 = D0;
    STR_LCDConfig.LCD_D1 = D1;
    STR_LCDConfig.LCD_D2 = D2;
    STR_LCDConfig.LCD_D3 = D3;
    STR_LCDConfig.LCD_D4 = D4;
    STR_LCDConfig.LCD_D5 = D5;
    STR_LCDConfig.LCD_D6 = D6;
    STR_LCDConfig.LCD_D7 = D7;


    if((D0 == P_NC) || (D1 == P_NC) || (D2 == P_NC) || (D3 == P_NC))
	{
        STR_LCDConfig.var_LcdMode_U8 = 4; // Select 4-bit mode as D0-D3 are not used(P_NC)
	}
    else
	{
        STR_LCDConfig.var_LcdMode_U8 = 8; // 8-bit mode configure D0-D3 as output.	  
		GPIO_PinDirection(D0,OUTPUT);
        GPIO_PinDirection(D1,OUTPUT);
        GPIO_PinDirection(D2,OUTPUT);
        GPIO_PinDirection(D3,OUTPUT);
	}

	/* Configure RS,RW,EN, D4-D7 as Output for both 4/8-bit mode.*/
    GPIO_PinDirection(RS,OUTPUT);
    GPIO_PinDirection(RW,OUTPUT);
    GPIO_PinDirection(EN,OUTPUT);

    GPIO_PinDirection(D4,OUTPUT);
    GPIO_PinDirection(D5,OUTPUT);
    GPIO_PinDirection(D6,OUTPUT);
    GPIO_PinDirection(D7,OUTPUT);  
}


/**************************************************************************************************
void LCD_Init(uint8_t var_lcdNoOfLines_u8, uint8_t var_MaxCharsPerLine_u8)
***************************************************************************************************
 * Function name:  LCD_Init()
 * I/P Arguments:  uint8_t: Number of lines of LCD
                   uint8_t: Number of Chars per line
 * Return value	: none

 * description  :This function is used to initialize the lcd.
                 *It initializes the LCD for selected mode(4/8-bit) and Type(16x2/16x1 etc)
**************************************************************************************************/
void LCD_Init(uint8_t var_lcdNoOfLines_u8, uint8_t var_MaxCharsPerLine_u8)
{


	STR_LCDConfig.var_MaxSupportedChars_U8 = var_MaxCharsPerLine_u8; //Maintaian the LCD type
	STR_LCDConfig.var_MaxSupportedLines_U8 = var_lcdNoOfLines_u8;
	if(var_lcdNoOfLines_u8 > E_LcdLineTwo)
	{
		ARR_LcdLineNumAddress_U8[E_LcdLineOne] =  0x90 + (var_MaxCharsPerLine_u8 & 0x0fu);
		ARR_LcdLineNumAddress_U8[E_LcdLineThree] =  0xd0 + (var_MaxCharsPerLine_u8 & 0x0fu);
	}

	DELAY_ms(100);

	if(STR_LCDConfig.var_LcdMode_U8 == M_EightBitMode)
	{
		LCD_CmdWrite(CMD_LCD_EIGHT_BIT_MODE); // Initialize the LCD for 8-bit 5x7 matrix type
	}
	else if(STR_LCDConfig.var_LcdMode_U8 == M_FourBitMode)
	{
		lcd_Reset();
		LCD_CmdWrite(CMD_LCD_FOUR_BIT_MODE); // Initialize the LCD for 4-bit 5x7 matrix type 
	}

	LCD_CmdWrite(CMD_DISPLAY_ON_CURSOR_ON);	 // Display ON cursor ON
	LCD_Clear();	                         // Clear the LCD and go to First line First Position
}




/***************************************************************************************************
                         void LCD_Clear()
 ***************************************************************************************************
 * I/P Arguments: none.
 * Return value	: none

 * description  :This function clears the LCD and moves the cursor to beginning of first line
 ***************************************************************************************************/
void LCD_Clear()
{
	LCD_CmdWrite(CMD_LCD_CLEAR);	// Clear the LCD and go to First line First Position
	LCD_GoToLine(E_LcdLineZero);
}





/***************************************************************************************************
                         void LCD_GoToLine(uint8_t var_lineNumber_u8)
 ***************************************************************************************************
 * I/P Arguments: uint8_t: Line number.
 * Return value	: none

 * description  :This function moves the Cursor to beginning of the specified line.
        If the requested line number is out of range, it will not move the cursor.

     Note: The line numbers run from 0 to Maxlines-1,
	 	   To avoid the confusion the below enums has to be used for selecting lines
           For four line LCD the enums are as below:
		   E_LcdLineZero,
           E_LcdLineOne,
           E_LcdLineTwo,
           E_LcdLineThree,

 ***************************************************************************************************/
void LCD_GoToLine(uint8_t var_lineNumber_u8)
{
	if(var_lineNumber_u8 <= STR_LCDConfig.var_MaxSupportedLines_U8)
	{
		/* If the line number is within range then
	       Move the Cursor to beginning of the specified line */
		VAR_LcdTrackCursorPos_U8 = 0x00;
		VAR_LcdTrackLineNum_U8 = var_lineNumber_u8;
		LCD_CmdWrite(ARR_LcdLineNumAddress_U8[var_lineNumber_u8]);
	}
}






/***************************************************************************************************
                         void  LCD_GoToNextLine()
 ***************************************************************************************************
 * I/P Arguments: none
 * Return value	: none

 * description  :This function moves the Cursor to beginning of the next line.
        If the cursor is on last line and NextLine command is issued then 
		it will move the cursor to first line.
 ***************************************************************************************************/
void  LCD_GoToNextLine(void)
{
	/*Increment the current line number.
	  In case it exceeds the limit, rool it back to first line */
	VAR_LcdTrackLineNum_U8++;
	VAR_LcdTrackCursorPos_U8 = 0x00;
	if(VAR_LcdTrackLineNum_U8 > STR_LCDConfig.var_MaxSupportedLines_U8)
		VAR_LcdTrackLineNum_U8 = 0x01;
	LCD_CmdWrite(ARR_LcdLineNumAddress_U8[VAR_LcdTrackLineNum_U8]);
}






/***************************************************************************************************
                void LCD_SetCursor(char var_lineNumber_u8,char var_charNumber_u8)
 ***************************************************************************************************
 * I/P Arguments: char row,char col
                 row -> line number(line1=1, line2=2),
                        For 2line LCD the I/P argument should be either 1 or 2.
                 col -> char number.
                        For 16-char LCD the I/P argument should be between 0-15.
 * Return value	: none

 * description  :This function moves the Cursor to specified position

                   Note:If the Input(Line/Char number) are out of range 
				        then no action will be taken
 ***************************************************************************************************/
#if ( Enable_LCD_SetCursor    == 1 )
void LCD_SetCursor(uint8_t var_lineNumber_u8, uint8_t var_charNumber_u8)
{

	if((var_lineNumber_u8 <= STR_LCDConfig.var_MaxSupportedLines_U8) &&
			(var_charNumber_u8< STR_LCDConfig.var_MaxSupportedChars_U8))
	{
		/*If the line number and char are in range then
		   move the Cursor to specified Position*/
		VAR_LcdTrackCursorPos_U8 = var_charNumber_u8;
		VAR_LcdTrackLineNum_U8 = var_lineNumber_u8;
		LCD_CmdWrite(ARR_LcdLineNumAddress_U8[var_lineNumber_u8]+var_charNumber_u8);
	}
}
#endif









/***************************************************************************************************
                       void LCD_CmdWrite( uint8_t var_lcdCmd_u8)
 ***************************************************************************************************
 * I/P Arguments: 8-bit command supported by LCD.
 * Return value	: none

 * description :This function sends a command to LCD.
                Some of the commonly used commands are defined in lcd.h.
                For more commands refer the data sheet and send the supported command.				
				The behaviour is undefined if unsupported commands are sent.    
 ***************************************************************************************************/
void LCD_CmdWrite( uint8_t var_lcdCmd_u8)
{
	lcd_BusyCheck();
	if(STR_LCDConfig.var_LcdMode_U8 == M_EightBitMode)
     {
         lcd_SendLowerNibble(var_lcdCmd_u8);
     }
     else
     {
         lcd_SendHigherNibble(var_lcdCmd_u8);
         lcd_SendCmdSignals();
         var_lcdCmd_u8 = var_lcdCmd_u8 << 4;
     }

     lcd_SendHigherNibble(var_lcdCmd_u8);
     lcd_SendCmdSignals();
}







/***************************************************************************************************
                       void LCD_DisplayChar( char var_lcdData_u8)
 ***************************************************************************************************
 * I/P Arguments: ASCII value of the char to be displayed.
 * Return value	: none

 * description  : This function sends a character to be displayed on LCD.
                  Any valid ascii value can be passed to display respective character

 ***************************************************************************************************/
void LCD_DisplayChar(char var_lcdData_u8)
{
	if((VAR_LcdTrackCursorPos_U8>=STR_LCDConfig.var_MaxSupportedChars_U8) || (var_lcdData_u8=='\n'))
	{
		/* If the cursor has reached to end of line on page1
		OR NewLine command is issued Then Move the cursor to next line */
		LCD_GoToNextLine();
	}
	if(var_lcdData_u8!='\n') /* Display the character if its not newLine Char */
	{

		lcd_DataWrite(var_lcdData_u8); /* Display the data and keep track of cursor */
		VAR_LcdTrackCursorPos_U8++;
	}
}







/***************************************************************************************************
                       void LCD_DisplayString(char *ptr_stringPointer_u8)
 ***************************************************************************************************
 * I/P Arguments: String(Address of the string) to be displayed.
 * Return value	: none

 * description  :
               This function is used to display the ASCII string on the lcd.
                 1.The ptr_stringPointer_u8 points to the first char of the string
                    and traverses till the end(NULL CHAR)and displays a char each time.

 ***************************************************************************************************/
#if (Enable_LCD_DisplayString==1)
void LCD_DisplayString(char *ptr_stringPointer_u8)
{
	while((*ptr_stringPointer_u8)!=0)
		LCD_DisplayChar(*ptr_stringPointer_u8++); // Loop through the string and display char by char
}
#endif







/***************************************************************************************************
               void LCD_ScrollMessage(uint8_t var_lineNumber_u8, char *ptr_msgPointer_u8)
 ***************************************************************************************************
 * I/P Arguments: 
                  uint8_t  : Line number on which the message has to be scrolled
                  char *: pointer to the string to be scrolled					  

 * Return value	: none

 * description  :This function scrolls the given message on the specified line.
                 If the specified line number is out of range then the message
				 will be scrolled on first line
 ***************************************************************************************************/
#if ( Enable_LCD_ScrollMessage  == 1 )
void LCD_ScrollMessage(uint8_t var_lineNumber_u8, char *ptr_msgPointer_u8)
{
	unsigned char i,j;


	if(var_lineNumber_u8 > STR_LCDConfig.var_MaxSupportedLines_U8)
		var_lineNumber_u8 = E_LcdLineOne; // Select first line if the var_lineNumber_u8 is out of range

	LCD_CmdWrite(CMD_DISPLAY_ON_CURSOR_OFF);			 //Disable the Cursor

	for(i=0;ptr_msgPointer_u8[i];i++)
	{      
		/* Loop to display the complete string,	each time 16 chars are displayed and
		pointer is incremented to point to next char */


		LCD_GoToLine(var_lineNumber_u8);     //Move the Cursor to first line

		for(j=0;(j<STR_LCDConfig.var_MaxSupportedChars_U8) && (ptr_msgPointer_u8[i+j]);j++)
		{
			//Display first 16 Chars or till Null char is reached
			LCD_DisplayChar(ptr_msgPointer_u8[i+j]);
		}


		while( j<STR_LCDConfig.var_MaxSupportedChars_U8)
		{
			/*If the chars to be scrolled are less than MaxLcdChars,
			  then display remaining chars with blank spaces*/
			LCD_DisplayChar(' ');
			j++;
		}

		DELAY_ms(200);
	}
	LCD_CmdWrite(CMD_DISPLAY_ON_CURSOR_ON);			  // Finally enable the Cursor
}
#endif






/***************************************************************************************************
void LCD_DisplayNumber(NumericSystem_et e_typeOfNum_e8, uint32_t var_number_u32, uint8_t var_numOfDigitsToDisplay_u8)
 ***************************************************************************************************
 * Function name:  LCD_DisplayNumber()
 * I/P Arguments: 
                  NumericSystem_et :  specifies type of number ENUM_BINARY(2),ENUM_DECIMAL(10), ENUM_Hex(16)
                  uint32_t: Number to be displayed on the LCD.
                  uint8_t : Number of digits to be displayed
                  
 * Return value	: none

 * description  :This function is used to display a max of 10digit decimal/Hex number OR specified 
                 number of bits for binary number.
                
                1st parameter specifies type of number ENUM_BINARY(2),ENUM_DECIMAL(10), ENUM_Hex(16)                  
                3rd parameter specifies the number of digits from the right side to be displayed
                 The output for the input combinations is as below
               
    Binary:     1.(10,4) then 4-LSB will be displayed ie. 1010
				2.(10,8) then 8-LSB will be displayed ie. 00001010
				3.(10,2) then 2-LSB will be displayed ie. 10
                
    Decimal:    4.(10,12345,4) then 4-digits ie. 2345 will be displayed
				5.(ENUM_DECIMAL,12345,6) then 6-digits ie. 012345 will be displayed
				6.(10,12345,C_DisplayDefaultDigits_U8) then 12345 will be displayed.\
                
    Hex:        7.(16,0x12AB,3) then 3-digits ie. 2AB will be displayed
				8.(ENUM_Hex,0x12AB,6) then 6-digits ie. 0012AB will be displayed
				9.(ENUM_Hex,0x12AB,C_DisplayDefaultDigits_U8) then 12AB will be displayed.                
 ***************************************************************************************************/
#if ((Enable_LCD_DisplayNumber == 1) || (Enable_LCD_DisplayFloatNumber == 1) || (ENABLE_LCD_Printf==1))
void LCD_DisplayNumber(NumericSystem_et enm_typeOfNumber, uint32_t var_number_u32, uint8_t var_numOfDigitsToDisplay_u8)
{
	uint8_t i=0,a[10];
    
    if(E_BINARY == enm_typeOfNumber)
    {
        while(var_numOfDigitsToDisplay_u8!=0)
	    {
		  /* Start Extracting the bits from the specified bit positions.
	      Get the Acsii values of the bits and display */
		  i = util_GetBitStatus(var_number_u32,(var_numOfDigitsToDisplay_u8-1));
		  LCD_DisplayChar(util_Dec2Ascii(i));
		  var_numOfDigitsToDisplay_u8--;
	    }        
    }    
	else if(var_number_u32==0)
	{
		/* If the number is zero then update the array with the same for displaying */
		for(i=0;((i<var_numOfDigitsToDisplay_u8) && (i<C_MaxDigitsToDisplay_U8));i++)
	    	LCD_DisplayChar('0');
	}
	else
	{
		for(i=0;i<var_numOfDigitsToDisplay_u8;i++)
		{
			/* Continue extracting the digits from right side
			   till the Specified var_numOfDigitsToDisplay_u8 */
			if(var_number_u32!=0)
			{
				/* Extract the digits from the number till it becomes zero.
			    First get the remainder and divide the number by TypeOfNum(10-Dec, 16-Hex) each time.
                
                example for Decimal number: 
                If var_number_u32 = 123 then extracted remainder will be 3 and number will be 12.
				The process continues till it becomes zero or max digits reached*/
				a[i]=util_GetMod32(var_number_u32,enm_typeOfNumber);
				var_number_u32=var_number_u32/enm_typeOfNumber;
			}
			else if( (var_numOfDigitsToDisplay_u8 == C_DisplayDefaultDigits_U8) ||
					 (var_numOfDigitsToDisplay_u8 > C_MaxDigitsToDisplay_U8))
			{
				/* Stop the iteration if the Max number of digits are reached or 
			     the user expects exact(Default) digits in the number to be displayed */ 
				break;
			}
			else
			{
				/* In case user expects more digits to be displayed than the actual digits in number,
  			    then update the remaining digits with zero.
               Ex: var_num_u32 is 123 and user wants five digits then 00123 has to be displayed */
				a[i]=0;
			}
		}
        
         while(i!=0)
	    { 
		  /* Finally get the ascii values of the digits and display*/
		  LCD_DisplayChar(util_Hex2Ascii(a[i-1]));
		  i--;
	    }
	}
}
#endif








/*************************************************************************************************
            void  LCD_DisplayFloatNumber(double var_floatNum_f32)
 *************************************************************************************************
 * Function name:  LCD_DisplayFloatNumber()
 * I/P Arguments: float: float Number to be displayed on the LCD.

 * Return value	: none

 * description  :This function is used to display a floating point number
                 It supports 6digits of precision.

    Note: Float will be disabled by default as it takes huge controller resources
	     It can be enabled by changing value of Enable_LCD_DisplayFloatNumber to 1 in lcd.h
 **************************************************************************************************/
#if (Enable_LCD_DisplayFloatNumber == 1)  
void LCD_DisplayFloatNumber(double var_floatNum_f32)
{
	uint32_t var_temp_u32;
	/* Dirty hack to support the floating point by extracting the integer and fractional part.
      1.Type cast the number to int to get the integer part.
	  2.Display the extracted integer part followed by a decimal point(.)
	  3.Later the integer part is made zero by subtracting with the extracted integer value.
	  4.Finally the fractional part is multiplied by 100000 to support 6-digit precision */

	var_temp_u32 = (uint32_t) var_floatNum_f32;
	LCD_DisplayNumber(E_DECIMAL,var_temp_u32,C_DisplayDefaultDigits_U8);

	LCD_DisplayChar('.');

	var_floatNum_f32 = var_floatNum_f32 - var_temp_u32;
	var_temp_u32 = var_floatNum_f32 * 1000000;
	LCD_DisplayNumber(E_DECIMAL,var_temp_u32,C_DisplayDefaultDigits_U8);
}
#endif






/*************************************************************************************************
            void LCD_Printf(const char *argList, ...)
 *************************************************************************************************
 * Function name:  LCD_Printf()
 * I/P Arguments: variable length arguments similar to printf

 * Return value	: none

 * description  :This function is similar to printf function in C.
				 It takes the arguments with specified format and prints accordingly
				 The supported format specifiers are as below.
				 1. %c: character
				 2. %d: signed 16-bit number
				 3. %D: signed 32-bit number
				 4. %u: unsigned 16-bit number
				 5. %U: unsigned 32-bit number
				 6. %b: 16-bit binary number
				 7. %B: 32-bit binary number
				 8. %f: Float number
				 9. %x: 16-bit hexadecimal number
				 10. %X: 32-bit hexadecimal number
				 11. %s: String



  Extra feature is available to specify the number of digits to be displayed using printf.
	 ex: %4d: will display the lower four digits of the decimal number.
	     %12b: will display the 12-LSB of the number
		 %d: Will display the exact digits of the number

#####: In case of printing the 8-bit variables, it is recommended to type cast and promote them to uint16_t.
        uint8_t var_Num_u8;
		LCD_Printf("num1:%u",(uint16_t)var_Num_u8); 
 *************************************************************************************************/
#if ( Enable_LCD_Printf   == 1 ) 
void LCD_Printf(const char *argList, ...)
{
    const char *ptr;
    va_list argp;
    sint16_t var_num_s16;
    sint32_t var_num_s32;
    uint16_t var_num_u16;
    uint32_t var_num_u32;
    char *str;
    char  ch;
    uint8_t var_numOfDigitsToDisp_u8;
#ifdef Enable_LCD_DisplayFloatNumber
	double var_floatNum_f32;
#endif

    va_start(argp, argList);

    /* Loop through the list to extract all the input arguments */
    for(ptr = argList; *ptr != '\0'; ptr++)
    {

        ch= *ptr;
        if(ch == '%')         /*Check for '%' as there will be format specifier after it */
        {
            ptr++;
            ch = *ptr;
           if((ch>=0x30) && (ch<=0x39))
            {
               var_numOfDigitsToDisp_u8 = 0;
               while((ch>=0x30) && (ch<=0x39))
                {
                   var_numOfDigitsToDisp_u8 = (var_numOfDigitsToDisp_u8 * 10) + (ch-0x30);
                   ptr++;
                   ch = *ptr;
                }
            }
            else
            {
              var_numOfDigitsToDisp_u8 = C_MaxDigitsToDisplayUsingPrintf_U8;
            }                


            switch(ch)       /* Decode the type of the argument */
            {
            case 'C':
            case 'c':     /* Argument type is of char, hence read char data from the argp */
                ch = va_arg(argp, int);
                LCD_DisplayChar(ch);
                break;

            case 'd':    /* Argument type is of signed integer, hence read 16bit data from the argp */
                var_num_s16 = va_arg(argp, int);
#if (Enable_LCD_DisplayNumber == 1)
                if(var_num_s16<0)
                 { /* If the number is -ve then display the 2's complement along with '-' sign */ 
                   var_num_s16 = -var_num_s16;
                   LCD_DisplayChar('-');
                 }
                LCD_DisplayNumber(E_DECIMAL,var_num_s16,var_numOfDigitsToDisp_u8);
#endif
                break;
                
            case 'D':    /* Argument type is of integer, hence read 16bit data from the argp */
                var_num_s32 = va_arg(argp, sint32_t);
#if (Enable_LCD_DisplayNumber == 1)
                if(var_num_s32<0)
                 { /* If the number is -ve then display the 2's complement along with '-' sign */
                   var_num_s32 = -var_num_s32;
                   LCD_DisplayChar('-');
                 }
                LCD_DisplayNumber(E_DECIMAL,var_num_s32,var_numOfDigitsToDisp_u8);
#endif                
                break;    

            case 'u':    /* Argument type is of unsigned integer, hence read 16bit unsigned data */
                var_num_u16 = va_arg(argp, int);
#if (Enable_LCD_DisplayNumber == 1)
                LCD_DisplayNumber(E_DECIMAL,var_num_u16,var_numOfDigitsToDisp_u8);
#endif                
                break;
            
            case 'U':    /* Argument type is of integer, hence read 32bit unsigend data */
                var_num_u32 = va_arg(argp, uint32_t);
#if (Enable_LCD_DisplayNumber == 1)
                LCD_DisplayNumber(E_DECIMAL,var_num_u32,var_numOfDigitsToDisp_u8);
#endif                
                break;            

            case 'x':  /* Argument type is of hex, hence hexadecimal data from the argp */
                var_num_u16 = va_arg(argp, int);
#if (Enable_LCD_DisplayNumber == 1)
                LCD_DisplayNumber(E_HEX,var_num_u16,var_numOfDigitsToDisp_u8);
#endif                
                break;

            case 'X':  /* Argument type is of hex, hence hexadecimal data from the argp */
                var_num_u32 = va_arg(argp, uint32_t);
#if (Enable_LCD_DisplayNumber == 1)
                LCD_DisplayNumber(E_HEX,var_num_u32,var_numOfDigitsToDisp_u8);
#endif                
                break;

            
            case 'b':  /* Argument type is of binary,Read int and convert to binary */
                var_num_u16 = va_arg(argp, int);
#if (Enable_LCD_DisplayNumber == 1)
                if(var_numOfDigitsToDisp_u8 == C_MaxDigitsToDisplayUsingPrintf_U8)
                   var_numOfDigitsToDisp_u8 = 16;
                LCD_DisplayNumber(E_BINARY,var_num_u16,var_numOfDigitsToDisp_u8);
#endif                
                break;

            case 'B':  /* Argument type is of binary,Read int and convert to binary */
                var_num_u32 = va_arg(argp, uint32_t);
#if (Enable_LCD_DisplayNumber == 1)
                if(var_numOfDigitsToDisp_u8 == C_MaxDigitsToDisplayUsingPrintf_U8)
                   var_numOfDigitsToDisp_u8 = 16;                
                LCD_DisplayNumber(E_BINARY,var_num_u32,var_numOfDigitsToDisp_u8);
#endif                
                break;


            case 'F':
            case 'f': /* Argument type is of float, hence read double data from the argp */
                var_floatNum_f32 = va_arg(argp, double);
#if (Enable_LCD_DisplayFloatNumber == 1)                
                LCD_DisplayFloatNumber(var_floatNum_f32);
#endif
                break;


            case 'S':
            case 's': /* Argument type is of string, hence get the pointer to sting passed */
                str = va_arg(argp, char *);
#if (Enable_LCD_DisplayString == 1)
                LCD_DisplayString(str);
#endif                
                break;

            case '%':
                LCD_DisplayChar('%');
                break;
            }
        }
        else
        {
            /* As '%' is not detected display/transmit the char passed */
            LCD_DisplayChar(ch);
        }
    }

    va_end(argp);
}
#endif




static void lcd_SendHigherNibble(uint8_t dataByte)
{
    GPIO_PinWrite(STR_LCDConfig.LCD_D4,util_IsBitSet(dataByte,4));
    GPIO_PinWrite(STR_LCDConfig.LCD_D5,util_IsBitSet(dataByte,5));
    GPIO_PinWrite(STR_LCDConfig.LCD_D6,util_IsBitSet(dataByte,6));
    GPIO_PinWrite(STR_LCDConfig.LCD_D7,util_IsBitSet(dataByte,7));
}

static void lcd_SendLowerNibble(uint8_t dataByte)
{
    GPIO_PinWrite(STR_LCDConfig.LCD_D0,util_IsBitSet(dataByte,0));
    GPIO_PinWrite(STR_LCDConfig.LCD_D1,util_IsBitSet(dataByte,1));
    GPIO_PinWrite(STR_LCDConfig.LCD_D2,util_IsBitSet(dataByte,2));
    GPIO_PinWrite(STR_LCDConfig.LCD_D3,util_IsBitSet(dataByte,3));
}






/*************************************************************************************************
                       static void lcd_DataWrite( uint8_t dat)
 *************************************************************************************************
 * I/P Arguments: uint8_t: 8-bit value to be sent to LCD.
 * Return value	: none

 * description : This functions is used to send a byte of data to LCD.                 .    
 *************************************************************************************************/
static void lcd_DataWrite( uint8_t dataByte)
{
	lcd_BusyCheck();
	if(STR_LCDConfig.var_LcdMode_U8 == M_EightBitMode)
     {
         lcd_SendLowerNibble(dataByte);
     }
     else
     {
         lcd_SendHigherNibble(dataByte);
         lcd_SendDataSignals();
         dataByte = dataByte << 4;
     }

     lcd_SendHigherNibble(dataByte);
     lcd_SendDataSignals();
}







/*************************************************************************************************
                       static void lcd_BusyCheck()
 *************************************************************************************************
 * I/P Arguments: none.
 * Return value	: none

 * description : This functions is used check whether LCD is busy.
                 It waits till the LCD is busy by polling the LCD busy flag.
				 After completing the previous operation, LCDs clears its internal busy flag.
 *************************************************************************************************/
static void lcd_BusyCheck(void)
{
	uint8_t busyflag;
  if(STR_LCDConfig.LCD_RW != P_NC)                         //Perform Busy check if LCD_RW pin is used
   {
	GPIO_PinDirection(STR_LCDConfig.LCD_D7,INPUT); // Configure busy pin as input
    GPIO_PinWrite(STR_LCDConfig.LCD_RS,0);           // Select the Command Register by pulling RS LOW
	GPIO_PinWrite(STR_LCDConfig.LCD_RW,1);           // Select the Read Operation for busy flag by setting RW
	do
	{

			GPIO_PinWrite(STR_LCDConfig.LCD_EN,0);
			DELAY_us(10);
			GPIO_PinWrite(STR_LCDConfig.LCD_EN,1); 
			DELAY_us(10);
	    	busyflag = GPIO_PinRead(STR_LCDConfig.LCD_D7);


		if(STR_LCDConfig.var_LcdMode_U8 == 4)
		{
			/* Perform extra dummy read for 4-bit */ 	   	
			GPIO_PinWrite(STR_LCDConfig.LCD_EN,0);
			DELAY_us(10);
			GPIO_PinWrite(STR_LCDConfig.LCD_EN,1); 
			DELAY_us(10);
		}	
	}while(busyflag);

		GPIO_PinDirection(STR_LCDConfig.LCD_D7,OUTPUT); // Configure busy pin as Output
  }
else
 {
	/* Busy flag cannot be read as LCD_RW is not available hence Extra delay of 1ms is added 
	  to ensure the LCD completes previous operation and ready to receive new commands/data */
	DELAY_ms(1);  
 }
}






/*************************************************************************************************
                       static void lcd_Reset()
 *************************************************************************************************
 * I/P Arguments: none.
 * Return value	: none

 * description : This functions is used to reset the LCD. 
                 This is used only in 4-bit mode as the LCD by default boots in 8-Bit mode.
----------------------------------------------------------------------------------*/
static void lcd_Reset(void)
{
	/* LCD reset sequence for 4-bit mode, refer data sheet for more info */
	lcd_SendHigherNibble(0x30);
	lcd_SendCmdSignals();
	DELAY_ms(100);
	lcd_SendHigherNibble(0x30);
	lcd_SendCmdSignals();
	DELAY_us(200);
	lcd_SendHigherNibble(0x30);
	lcd_SendCmdSignals();
	DELAY_us(200);
	lcd_SendHigherNibble(0x20);
	lcd_SendCmdSignals();
	DELAY_us(200);
}






/*************************************************************************************************
                       static void lcd_SendNibble(uint8_t var)
 *************************************************************************************************
 * I/P Arguments: uint8_t: Higher nibble of the data to be send on LCD4-LCD7 data lines
 * Return value	: none

 * description : This functions is used to send the higher nibble of the data to LCD in 4-bit mode
 *************************************************************************************************/





/*************************************************************************************************
                       static void lcd_SendCmdSignals()
 *************************************************************************************************
 * I/P Arguments: none
 * Return value	: none

 * description : This functions generates the signals for sending the var_lcdCmd_u8 to LCD
 *************************************************************************************************/
static void lcd_SendCmdSignals(void)
{
	 GPIO_PinWrite(STR_LCDConfig.LCD_RS,0);
	 GPIO_PinWrite(STR_LCDConfig.LCD_RW,0);
	 GPIO_PinWrite(STR_LCDConfig.LCD_EN,1);
	 DELAY_us(10);
	 GPIO_PinWrite(STR_LCDConfig.LCD_EN,0);
  
}




/*************************************************************************************************
                       static void lcd_SendDataSignals()
 **************************************************************************************************
 * I/P Arguments: none
 * Return value	: none

 * description : This functions generates the signals for sending the Data to LCD
 *************************************************************************************************/
static void lcd_SendDataSignals(void)
{
	 GPIO_PinWrite(STR_LCDConfig.LCD_RS,1);
	 GPIO_PinWrite(STR_LCDConfig.LCD_RW,0);
	 GPIO_PinWrite(STR_LCDConfig.LCD_EN,1);
	 DELAY_us(10);
	 GPIO_PinWrite(STR_LCDConfig.LCD_EN,0);
}