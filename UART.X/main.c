/*
 * File:   sw_uart.c
 * Author: Dries
 *
 * Created on 12 maart 2013, 14:49
 */




/*****************************************************
		Includes & Defines
**************************************************** */

#include <p18cxxx.h>
#include <delays.h>
#include <timers.h>
#include <usart.h>
#include <sw_uart.h>
#include <string.h>


#define BUF_SIZE 32
#define ID 0x55

#define SWREAD PORTbits.RB5
#define SWWRITE PORTbits.RB4




/*****************************************************
		Function Prototypes
**************************************************** */

void initChip();
void InterruptHandlerHigh(void);

void ReadGPS(void);
void WriteXbee(void);
void WriteTest(static const rom char*);
void WriteData(void);
void WriteGPS(char[]);
void initGPS(void);
void read_gps_2( void );


/*************************************************
  RESET VECTORS: REMOVE IF NOT USING BOOTLOADER!!!
**************************************************/


extern void _startup (void);        
#pragma code _RESET_INTERRUPT_VECTOR = 0x000800
void _reset (void)
{
    _asm goto _startup _endasm
}
#pragma code

#pragma code _HIGH_INTERRUPT_VECTOR = 0x000808
void _high_ISR (void)
{
   InterruptHandlerHigh();
}


#pragma code _LOW_INTERRUPT_VECTOR = 0x000818
void _low_ISR (void)
{
   InterruptHandlerHigh();
} 
#pragma code
/* END OF VECTOR REMAPPING*/


/*********************************************************
 Global Variables
**********************************************************/

unsigned int DelayCount;
char buff[200];
char instr[5][5];
int sw_flag = 0;


unsigned char gpsReceive = 0;

/*********************************************************
	Interrupt Handler
**********************************************************/
#pragma interrupt InterruptHandlerHigh
void InterruptHandlerHigh(void)
{
    INTCONbits.GIE = 0;

        
       read_gps_2();
		PIR1bits.RCIF = 0;






    
                    //Enable low priotity interrupts
    INTCONbits.GIE = 1;

}

#pragma code page

/*************************************************
			Initialize the CHIP
**************************************************/


/* DelayTXBitUART for sw_uart fucntions */
void DelayTXBitUART(void)
{
	/* delay for ((((2*Fosc)/(4*baud))+1)/2) - 12 cycles */
	/* dleay for ((((2*20,000,000)/(4*9600))+1)/2)-12 ~= 509 cycles */
	Delay100TCYx(12);
        Delay10TCYx(3);
        Nop();
	Nop();
        Nop();
	Nop();
        Nop();
	Nop();
        Nop();
	Nop();

}


/* DelayRXHalfBitUART for sw_uart fucntions */
void DelayRXHalfBitUART(void)
{
	/* delay for ((((2*Fosc)/(8*baud))+1)/2)-9 cycles */
	/* delay for ((((2*20,000,000)/(8*9600))+1)/2)-9 = 252 cycles */
	Delay100TCYx(6);
        Delay10TCYx(1);
	Nop();
	Nop();
        Nop();
	Nop();
        Nop();
	Nop();
}


/* DelayRXBitUART for sw_uart fucntions */
void DelayRXBitUART(void)
{
	/* delay for ((((2*Fosc)/(4*baud))+1)/2) - 14 cylces */
	/* delay for ((((2*20,000,000)/(4*9600))+1)/2)-14 ~= 507 cycles */
	Delay100TCYx(12);
        Delay10TCYx(3);
        Nop();
	Nop();
        Nop();
	Nop();
        Nop();
	Nop();

}

void initChip(){
    ADCON1 = 0x00;		//Turn off ADcon
	CMCON = 0x00;		//Turn off Comparator
	PORTA = 0x00;
	TRISA = 0x00;
	PORTB = 0x00;
	TRISB = 0xF0;
	PORTC = 0x00;
	TRISC = 0x00;
        TRISCbits.TRISC6 = 1;
        TRISCbits.TRISC7 = 1;
	PORTCbits.RC1 = 1;


        RCONbits.IPEN = 1;                  //Set to priority mode

        IPR1bits.RCIP=1;                    //USART Receive Interrupt Priority 0 = Low priority

        PIR1bits.RCIF = 0;                  //The flag for the USART_RX interrupt needs to be set to zero.
        PIE1bits.RCIE= 1;
        INTCONbits.GIE = 0; //DISABLED               //Enable high priority Interrupts
        INTCONbits.PEIE = 0;                //Enable low priotity interrupts

        PORTAbits.RA5 = 1;
         PORTAbits.RA4 = 1;
          PORTAbits.RA3 = 1;

}

void initGPS(){


	int i = 0;
	int maxInstr;

	for(i = 0; i<maxInstr; i++){
		WriteGPS(instr[i]);
	}
}



/*************************************************
                        MAIN
**************************************************/

void main(void)
{



initChip();
//initGPS();

OpenUART();
OpenUSART( USART_TX_INT_OFF &
   USART_RX_INT_ON &
   USART_ASYNCH_MODE &
   USART_EIGHT_BIT &
   USART_CONT_RX &
   USART_BRGH_HIGH, 1200 );

BAUDCONbits.BRG16 = 1;

INTCONbits.GIE = 1;                //Enable high priority Interrupts
INTCONbits.PEIE = 1;



while(1){

   	
	//read_gps_2();
	WriteData();

    }
}

void ReadGPS(){
	int i = 0;
	char current_char;
    do
    {
        current_char = ReadUSART();
		buff[i] = current_char;
		i++;

    } while(current_char != '\n');


}


void WriteXbee(){
    int i = -1;
   do
        { // Transmit a byte
        i++;
        putsUART(&buff);


        } while( buff[i] != '\0' );
  }

void WriteGPS(char bufferGPS[]){
	char buffer[100];
     putsUSART(&bufferGPS);
	 memset(buffer,0,100);
}


void WriteTest(static const rom char *GPS){
    int i = 0;
     do{
        WriteUSART(*GPS);
        
        while(BusyUSART());

        } while( *GPS++ != '\n' );

       Delay10KTCYx(500000000000000);
        Delay10KTCYx(500000000000000);
      
          

}

void read_gps_2( void ){
	char buffer[100];
	int i = 3;
	char c;

	buffer[0] = '$';
	buffer[1] = '0';
	buffer[2] = '1';
	//memset(buffer,0,100);
	while(i < 100){


		
		while(!DataRdyUSART());
		
	

		//c = ReadUSART();
		c = RCREG;
		
		WriteUSART(c);
		

		//buffer[i] = c ;
/*
		if(c == 0xa){ // check if char is carriage return

		putsUSART(&buffer);
		memset(buffer,0,100);
		i=0;
		break;
		}
*/
		i++;
	}
	PIR1bits.RCIF = 0;
//putsUSART(&buffer);

}

void WriteData(){
	
	char data[100];

WriteTest("$01$GPGGA,081813,5052.5396,N,00442.4935,E,1,7,49.0,101.9000015258789,M,,,,*1e\r\n");
WriteTest("$01$GPGGA,081814,5052.5397,N,00442.4942,E,1,7,42.0,101.30000305175781,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,081819,5052.5395,N,00442.4944,E,1,8,24.0,99.9000015258789,M,,,,*25\r\n");
WriteTest("$01$GPGGA,081823,5052.5322,N,00442.5031,E,1,8,16.0,81.4000015258789,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,081824,5052.5336,N,00442.4996,E,1,8,14.0,89.0999984741211,M,,,,*27\r\n");
WriteTest("$01$GPGGA,081827,5052.5335,N,00442.4999,E,1,8,17.0,88.5999984741211,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,081829,5052.5335,N,00442.4999,E,1,8,17.0,88.9000015258789,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,081832,5052.5333,N,00442.4998,E,1,7,16.0,89.0999984741211,M,,,,*26\r\n");
WriteTest("$01$GPGGA,081833,5052.5334,N,00442.5001,E,1,7,16.0,88.4000015258789,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,081838,5052.5332,N,00442.5000,E,1,8,24.0,88.5,M,,,,*22\r\n");
WriteTest("$01$GPGGA,081840,5052.5333,N,00442.4995,E,1,8,32.0,89.5999984741211,M,,,,*22\r\n");
WriteTest("$01$GPGGA,081841,5052.5336,N,00442.4993,E,1,8,31.0,91.4000015258789,M,,,,*2a\r\n");
WriteTest("$01$GPGGA,081845,5052.5336,N,00442.4993,E,1,8,30.0,91.5,M,,,,*23\r\n");
WriteTest("$01$GPGGA,081846,5052.5336,N,00442.4993,E,1,6,30.0,91.5,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,081849,5052.5339,N,00442.5022,E,1,7,24.0,83.0999984741211,M,,,,*22\r\n");
WriteTest("$01$GPGGA,081852,5052.5345,N,00442.5045,E,1,8,20.0,76.5999984741211,M,,,,*26\r\n");
WriteTest("$01$GPGGA,081854,5052.5344,N,00442.5045,E,1,7,19.0,76.5,M,,,,*28\r\n");
WriteTest("$01$GPGGA,081856,5052.5343,N,00442.5046,E,1,6,19.0,76.0999984741211,M,,,,*26\r\n");
WriteTest("$01$GPGGA,081900,5052.5343,N,00442.5047,E,1,7,18.0,76.0,M,,,,*29\r\n");
WriteTest("$01$GPGGA,081901,5052.5343,N,00442.5048,E,1,7,18.0,75.80000305175781,M,,,,*17\r\n");
WriteTest("$01$GPGGA,081902,5052.5343,N,00442.5049,E,1,7,19.0,75.69999694824219,M,,,,*1e\r\n");
WriteTest("$01$GPGGA,081907,5052.5343,N,00442.5050,E,1,7,35.0,75.5,M,,,,*21\r\n");
WriteTest("$01$GPGGA,081909,5052.5344,N,00442.5052,E,1,7,45.0,75.0,M,,,,*28\r\n");
WriteTest("$01$GPGGA,081911,5052.5344,N,00442.5054,E,1,6,58.0,74.5,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,081914,5052.5343,N,00442.5055,E,1,6,71.0,74.19999694824219,M,,,,*1d\r\n");
WriteTest("$01$GPGGA,081917,5052.5342,N,00442.5055,E,1,5,66.0,74.30000305175781,M,,,,*1c\r\n");
WriteTest("$01$GPGGA,081918,5052.5342,N,00442.5056,E,1,5,65.0,74.19999694824219,M,,,,*15\r\n");
WriteTest("$01$GPGGA,081922,5052.5343,N,00442.5063,E,1,5,76.0,72.5,M,,,,*24\r\n");
WriteTest("$01$GPGGA,081923,5052.5343,N,00442.5064,E,1,5,83.0,72.19999694824219,M,,,,*13\r\n");
WriteTest("$01$GPGGA,081925,5052.5342,N,00442.5064,E,1,5,84.0,72.4000015258789,M,,,,*24\r\n");
WriteTest("$01$GPGGA,081930,5052.5339,N,00442.5072,E,1,5,118.0,71.0,M,,,,*15\r\n");
WriteTest("$01$GPGGA,081931,5052.5338,N,00442.5074,E,1,5,128.0,70.5,M,,,,*14\r\n");
WriteTest("$01$GPGGA,081934,5052.5337,N,00442.5083,E,1,6,79.0,68.69999694824219,M,,,,*15\r\n");
WriteTest("$01$GPGGA,081936,5052.5335,N,00442.5086,E,1,6,55.0,68.19999694824219,M,,,,*19\r\n");
WriteTest("$01$GPGGA,081939,5052.5335,N,00442.5095,E,1,7,39.0,68.0,M,,,,*21\r\n");
WriteTest("$01$GPGGA,081940,5052.5335,N,00442.5096,E,1,7,40.0,67.69999694824219,M,,,,*14\r\n");
WriteTest("$01$GPGGA,081944,5052.5344,N,00442.5123,E,1,7,35.0,67.9000015258789,M,,,,*26\r\n");
WriteTest("$01$GPGGA,081946,5052.5343,N,00442.5122,E,1,7,33.0,68.5,M,,,,*2a\r\n");
WriteTest("$01$GPGGA,081947,5052.5343,N,00442.5121,E,1,7,36.0,68.5999984741211,M,,,,*21\r\n");
WriteTest("$01$GPGGA,081951,5052.5157,N,00442.5723,E,1,9,20.0,89.19999694824219,M,,,,*14\r\n");
WriteTest("$01$GPGGA,081952,5052.5165,N,00442.5726,E,1,9,16.0,90.0,M,,,,*20\r\n");
WriteTest("$01$GPGGA,081955,5052.5175,N,00442.5752,E,1,9,11.0,92.5999984741211,M,,,,*29\r\n");
WriteTest("$01$GPGGA,081958,5052.5110,N,00442.5749,E,1,5,10.0,87.19999694824219,M,,,,*13\r\n");
WriteTest("$01$GPGGA,082000,5052.5103,N,00442.5784,E,1,7,11.0,89.80000305175781,M,,,,*17\r\n");
WriteTest("$01$GPGGA,082002,5052.5096,N,00442.5801,E,1,7,8.0,89.30000305175781,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082006,5052.5069,N,00442.5802,E,1,8,9.0,99.30000305175781,M,,,,*21\r\n");
WriteTest("$01$GPGGA,082007,5052.5065,N,00442.5806,E,1,7,8.0,98.80000305175781,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082008,5052.5062,N,00442.5813,E,1,7,11.0,98.80000305175781,M,,,,*18\r\n");
WriteTest("$01$GPGGA,082012,5052.5051,N,00442.5835,E,1,5,9.0,98.80000305175781,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082014,5052.5043,N,00442.5849,E,1,6,8.0,98.80000305175781,M,,,,*20\r\n");
WriteTest("$01$GPGGA,082016,5052.5035,N,00442.5862,E,1,6,9.0,98.5999984741211,M,,,,*11\r\n");
WriteTest("$01$GPGGA,082019,5052.5019,N,00442.5885,E,1,6,14.0,97.9000015258789,M,,,,*27\r\n");
WriteTest("$01$GPGGA,082022,5052.5016,N,00442.5897,E,1,6,20.0,96.5,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082023,5052.5019,N,00442.5902,E,1,7,21.0,95.30000305175781,M,,,,*19\r\n");
WriteTest("$01$GPGGA,082027,5052.5053,N,00442.5947,E,1,9,24.0,92.0,M,,,,*26\r\n");
WriteTest("$01$GPGGA,082028,5052.5065,N,00442.5956,E,1,9,22.0,91.30000305175781,M,,,,*11\r\n");
WriteTest("$01$GPGGA,082030,5052.5099,N,00442.5977,E,1,9,16.0,90.5999984741211,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,082036,5052.5096,N,00442.5958,E,1,6,8.0,86.69999694824219,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082038,5052.5105,N,00442.5914,E,1,4,8.0,86.80000305175781,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082041,5052.5110,N,00442.5866,E,1,6,11.0,86.0,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,082043,5052.5111,N,00442.5838,E,1,6,13.0,85.19999694824219,M,,,,*19\r\n");
WriteTest("$01$GPGGA,082046,5052.5110,N,00442.5791,E,1,6,15.0,84.5,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082047,5052.5099,N,00442.5792,E,1,5,15.0,84.5999984741211,M,,,,*20\r\n");
WriteTest("$01$GPGGA,082051,5052.4933,N,00442.5960,E,1,6,10.0,85.5,M,,,,*27\r\n");
WriteTest("$01$GPGGA,082053,5052.4913,N,00442.5948,E,1,6,10.0,85.19999694824219,M,,,,*16\r\n");
WriteTest("$01$GPGGA,082054,5052.4903,N,00442.5942,E,1,6,9.0,85.19999694824219,M,,,,*22\r\n");
WriteTest("$01$GPGGA,082058,5052.4870,N,00442.5928,E,1,9,8.0,83.4000015258789,M,,,,*18\r\n");
WriteTest("$01$GPGGA,082059,5052.4864,N,00442.5930,E,1,9,7.0,82.80000305175781,M,,,,*21\r\n");
WriteTest("$01$GPGGA,082102,5052.4849,N,00442.5927,E,1,9,8.0,83.19999694824219,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082105,5052.4836,N,00442.5920,E,1,9,9.0,83.19999694824219,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082107,5052.4824,N,00442.5912,E,1,9,10.0,82.5,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,082110,5052.4806,N,00442.5898,E,1,9,9.0,82.0,M,,,,*17\r\n");
WriteTest("$01$GPGGA,082114,5052.4779,N,00442.5875,E,1,7,9.0,80.80000305175781,M,,,,*28\r\n");
WriteTest("$01$GPGGA,082115,5052.4773,N,00442.5869,E,1,5,10.0,80.5999984741211,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,082116,5052.4769,N,00442.5864,E,1,5,11.0,79.9000015258789,M,,,,*21\r\n");
WriteTest("$01$GPGGA,082120,5052.4752,N,00442.5843,E,1,7,11.0,80.19999694824219,M,,,,*17\r\n");
WriteTest("$01$GPGGA,082122,5052.4744,N,00442.5835,E,1,5,12.0,79.80000305175781,M,,,,*19\r\n");
WriteTest("$01$GPGGA,082124,5052.4731,N,00442.5831,E,1,6,13.0,78.80000305175781,M,,,,*1a\r\n");
WriteTest("$01$GPGGA,082127,5052.4722,N,00442.5810,E,1,7,10.0,78.0,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082130,5052.4712,N,00442.5811,E,1,7,8.0,77.80000305175781,M,,,,*28\r\n");
WriteTest("$01$GPGGA,082131,5052.4710,N,00442.5809,E,1,7,8.0,78.0,M,,,,*1e\r\n");
WriteTest("$01$GPGGA,082137,5052.4699,N,00442.5814,E,1,8,7.0,76.4000015258789,M,,,,*13\r\n");
WriteTest("$01$GPGGA,082138,5052.4699,N,00442.5814,E,1,8,8.0,76.4000015258789,M,,,,*13\r\n");
WriteTest("$01$GPGGA,082139,5052.4699,N,00442.5814,E,1,6,7.0,76.4000015258789,M,,,,*13\r\n");
WriteTest("$01$GPGGA,082143,5052.4698,N,00442.5814,E,1,7,7.0,76.4000015258789,M,,,,*1e\r\n");
WriteTest("$01$GPGGA,082145,5052.4694,N,00442.5813,E,1,6,6.0,76.30000305175781,M,,,,*22\r\n");
WriteTest("$01$GPGGA,082147,5052.4666,N,00442.5800,E,1,7,7.0,76.0,M,,,,*17\r\n");
WriteTest("$01$GPGGA,082150,5052.4647,N,00442.5793,E,1,5,10.0,75.0999984741211,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082153,5052.4641,N,00442.5790,E,1,5,12.0,74.4000015258789,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082154,5052.4639,N,00442.5789,E,1,5,14.0,74.30000305175781,M,,,,*1b\r\n");
WriteTest("$01$GPGGA,082158,5052.4636,N,00442.5783,E,1,5,24.0,73.4000015258789,M,,,,*27\r\n");
WriteTest("$01$GPGGA,082159,5052.4636,N,00442.5782,E,1,5,27.0,73.19999694824219,M,,,,*13\r\n");
WriteTest("$01$GPGGA,082201,5052.4636,N,00442.5781,E,1,5,33.0,73.19999694824219,M,,,,*1b\r\n");
WriteTest("$01$GPGGA,082205,5052.4638,N,00442.5780,E,1,7,24.0,72.30000305175781,M,,,,*13\r\n");
WriteTest("$01$GPGGA,082206,5052.4640,N,00442.5777,E,1,7,20.0,72.5,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,082209,5052.4641,N,00442.5770,E,1,7,17.0,72.9000015258789,M,,,,*22\r\n");
WriteTest("$01$GPGGA,082211,5052.4641,N,00442.5768,E,1,5,16.0,72.9000015258789,M,,,,*21\r\n");
WriteTest("$01$GPGGA,082218,5052.4628,N,00442.5454,E,1,6,14.0,70.5,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082219,5052.4609,N,00442.5444,E,1,8,12.0,71.69999694824219,M,,,,*1f\r\n");
WriteTest("$01$GPGGA,082223,5052.4554,N,00442.5398,E,1,5,14.0,73.69999694824219,M,,,,*12\r\n");
WriteTest("$01$GPGGA,082225,5052.4528,N,00442.5371,E,1,7,13.0,74.19999694824219,M,,,,*1d\r\n");
WriteTest("$01$GPGGA,082226,5052.4519,N,00442.5355,E,1,7,12.0,74.0999984741211,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082230,5052.4498,N,00442.5313,E,1,7,10.0,74.19999694824219,M,,,,*14\r\n");
WriteTest("$01$GPGGA,082231,5052.4498,N,00442.5305,E,1,6,9.0,74.30000305175781,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082234,5052.4508,N,00442.5279,E,1,6,9.0,75.0999984741211,M,,,,*1f\r\n");
WriteTest("$01$GPGGA,082237,5052.4528,N,00442.5250,E,1,4,12.0,74.4000015258789,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082239,5052.4534,N,00442.5223,E,1,4,14.0,74.80000305175781,M,,,,*12\r\n");
WriteTest("$01$GPGGA,082241,5052.4538,N,00442.5195,E,1,4,17.0,74.69999694824219,M,,,,*16\r\n");
WriteTest("$01$GPGGA,082245,5052.4546,N,00442.5125,E,1,4,26.0,74.5,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,082246,5052.4550,N,00442.5095,E,1,4,30.0,74.5,M,,,,*27\r\n");
WriteTest("$01$GPGGA,082247,5052.4555,N,00442.5078,E,1,5,29.0,74.4000015258789,M,,,,*25\r\n");
WriteTest("$01$GPGGA,082251,5052.4580,N,00442.5019,E,1,5,29.0,74.4000015258789,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082253,5052.4595,N,00442.4997,E,1,5,29.0,74.4000015258789,M,,,,*25\r\n");
WriteTest("$01$GPGGA,082255,5052.4610,N,00442.4980,E,1,5,30.0,74.4000015258789,M,,,,*23\r\n");
WriteTest("$01$GPGGA,082259,5052.4616,N,00442.4929,E,1,7,16.0,74.19999694824219,M,,,,*1b\r\n");
WriteTest("$01$GPGGA,082302,5052.4625,N,00442.4905,E,1,7,12.0,74.30000305175781,M,,,,*18\r\n");
WriteTest("$01$GPGGA,082305,5052.4635,N,00442.4902,E,1,6,11.0,74.69999694824219,M,,,,*1a\r\n");
WriteTest("$01$GPGGA,082310,5052.4662,N,00442.4881,E,1,6,16.0,74.5,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082311,5052.4667,N,00442.4876,E,1,6,20.0,74.5,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082313,5052.4677,N,00442.4865,E,1,6,26.0,74.5,M,,,,*23\r\n");
WriteTest("$01$GPGGA,082318,5052.4709,N,00442.4831,E,1,6,37.0,74.30000305175781,M,,,,*1c\r\n");
WriteTest("$01$GPGGA,082321,5052.4734,N,00442.4807,E,1,6,36.0,74.19999694824219,M,,,,*1a\r\n");
WriteTest("$01$GPGGA,082323,5052.4755,N,00442.4792,E,1,6,29.0,74.30000305175781,M,,,,*14\r\n");
WriteTest("$01$GPGGA,082327,5052.4812,N,00442.4763,E,1,8,24.0,74.4000015258789,M,,,,*20\r\n");
WriteTest("$01$GPGGA,082330,5052.4842,N,00442.4733,E,1,6,19.0,74.30000305175781,M,,,,*17\r\n");
WriteTest("$01$GPGGA,082333,5052.4868,N,00442.4703,E,1,7,19.0,73.9000015258789,M,,,,*25\r\n");
WriteTest("$01$GPGGA,082338,5052.4910,N,00442.4701,E,1,6,22.0,74.0999984741211,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082339,5052.4919,N,00442.4702,E,1,6,21.0,74.0999984741211,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082341,5052.4930,N,00442.4705,E,1,6,20.0,74.0999984741211,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,082346,5052.4935,N,00442.4650,E,1,7,12.0,92.5,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082350,5052.4927,N,00442.4576,E,1,6,9.0,96.80000305175781,M,,,,*26\r\n");
WriteTest("$01$GPGGA,082353,5052.4927,N,00442.4534,E,1,8,8.0,96.80000305175781,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082357,5052.4927,N,00442.4482,E,1,9,7.0,94.4000015258789,M,,,,*12\r\n");
WriteTest("$01$GPGGA,082402,5052.4918,N,00442.4461,E,1,8,6.0,91.80000305175781,M,,,,*2b\r\n");
WriteTest("$01$GPGGA,082403,5052.4920,N,00442.4461,E,1,8,7.0,91.19999694824219,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082408,5052.4940,N,00442.4456,E,1,5,9.0,90.0999984741211,M,,,,*14\r\n");
WriteTest("$01$GPGGA,082410,5052.4952,N,00442.4458,E,1,3,10.0,90.9000015258789,M,,,,*26\r\n");
WriteTest("$01$GPGGA,082412,5052.4971,N,00442.4454,E,1,4,11.0,89.5,M,,,,*26\r\n");
WriteTest("$01$GPGGA,082420,5052.4997,N,00442.4482,E,1,11,6.0,89.5,M,,,,*26\r\n");
WriteTest("$01$GPGGA,082425,5052.5021,N,00442.4496,E,1,11,6.0,90.5999984741211,M,,,,*27\r\n");
WriteTest("$01$GPGGA,082428,5052.5045,N,00442.4472,E,1,11,7.0,90.80000305175781,M,,,,*19\r\n");
WriteTest("$01$GPGGA,082432,5052.5067,N,00442.4471,E,1,11,16.0,92.0999984741211,M,,,,*1c\r\n");
WriteTest("$01$GPGGA,082434,5052.5071,N,00442.4493,E,1,11,19.0,92.4000015258789,M,,,,*1b\r\n");
WriteTest("$01$GPGGA,082436,5052.5073,N,00442.4529,E,1,11,18.0,93.30000305175781,M,,,,*2a\r\n");
WriteTest("$01$GPGGA,082441,5052.5076,N,00442.4607,E,1,7,16.0,93.5,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082442,5052.5081,N,00442.4619,E,1,7,16.0,93.69999694824219,M,,,,*1c\r\n");
WriteTest("$01$GPGGA,082445,5052.5097,N,00442.4651,E,1,7,15.0,93.80000305175781,M,,,,*19\r\n");
WriteTest("$01$GPGGA,082450,5052.5133,N,00442.4704,E,1,8,10.0,97.5999984741211,M,,,,*27\r\n");
WriteTest("$01$GPGGA,082454,5052.5153,N,00442.4763,E,1,8,10.0,98.5,M,,,,*27\r\n");
WriteTest("$01$GPGGA,082459,5052.5201,N,00442.4798,E,1,9,7.0,98.0999984741211,M,,,,*14\r\n");
WriteTest("$01$GPGGA,082502,5052.5215,N,00442.4827,E,1,9,6.0,98.5999984741211,M,,,,*11\r\n");
WriteTest("$01$GPGGA,082508,5052.5270,N,00442.4864,E,1,4,4.0,98.9000015258789,M,,,,*1d\r\n");
WriteTest("$01$GPGGA,082510,5052.5288,N,00442.4885,E,1,5,6.0,98.9000015258789,M,,,,*1f\r\n");
WriteTest("$01$GPGGA,082514,5052.5329,N,00442.4924,E,1,4,13.0,99.0,M,,,,*2b\r\n");
WriteTest("$01$GPGGA,082516,5052.5342,N,00442.4943,E,1,4,19.0,99.0,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,082517,5052.5348,N,00442.4952,E,1,4,23.0,99.0,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082522,5052.5378,N,00442.4989,E,1,5,51.0,98.9000015258789,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,082524,5052.5383,N,00442.5006,E,1,5,68.0,98.9000015258789,M,,,,*28\r\n");
WriteTest("$01$GPGGA,082526,5052.5386,N,00442.5017,E,1,5,88.0,99.0,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082530,5052.5397,N,00442.5058,E,1,5,130.0,99.0,M,,,,*1a\r\n");
WriteTest("$01$GPGGA,082532,5052.5403,N,00442.5082,E,1,4,157.0,99.0,M,,,,*15\r\n");
WriteTest("$01$GPGGA,082535,5052.5415,N,00442.5114,E,1,4,198.0,98.80000305175781,M,,,,*2a\r\n");
WriteTest("$01$GPGGA,082538,5052.5421,N,00442.5129,E,1,4,205.0,98.30000305175781,M,,,,*22\r\n");
WriteTest("$01$GPGGA,082542,5052.5432,N,00442.5178,E,1,5,108.0,97.4000015258789,M,,,,*18\r\n");
WriteTest("$01$GPGGA,082543,5052.5433,N,00442.5185,E,1,5,100.0,97.30000305175781,M,,,,*23\r\n");
WriteTest("$01$GPGGA,082547,5052.5414,N,00442.5212,E,1,5,51.0,96.5999984741211,M,,,,*2a\r\n");
WriteTest("$01$GPGGA,082548,5052.5412,N,00442.5216,E,1,5,48.0,96.69999694824219,M,,,,*1f\r\n");
WriteTest("$01$GPGGA,082550,5052.5401,N,00442.5211,E,1,6,40.0,96.5,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082553,5052.5397,N,00442.5186,E,1,6,35.0,96.80000305175781,M,,,,*16\r\n");
WriteTest("$01$GPGGA,082555,5052.5402,N,00442.5158,E,1,6,34.0,95.9000015258789,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082557,5052.5414,N,00442.5124,E,1,7,26.0,94.80000305175781,M,,,,*17\r\n");
WriteTest("$01$GPGGA,082600,5052.5398,N,00442.5075,E,1,7,16.0,94.30000305175781,M,,,,*18\r\n");
WriteTest("$01$GPGGA,082602,5052.5373,N,00442.5044,E,1,9,13.0,94.0999984741211,M,,,,*22\r\n");
WriteTest("$01$GPGGA,082603,5052.5361,N,00442.5031,E,1,9,12.0,93.5999984741211,M,,,,*21\r\n");
WriteTest("$01$GPGGA,082609,5052.5309,N,00442.4934,E,1,8,17.0,91.30000305175781,M,,,,*1f\r\n");
WriteTest("$01$GPGGA,082610,5052.5303,N,00442.4918,E,1,8,15.0,91.0,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082612,5052.5294,N,00442.4884,E,1,8,16.0,90.5999984741211,M,,,,*2b\r\n");
WriteTest("$01$GPGGA,082615,5052.5285,N,00442.4847,E,1,8,15.0,90.4000015258789,M,,,,*20\r\n");
WriteTest("$01$GPGGA,082617,5052.5280,N,00442.4829,E,1,8,13.0,90.19999694824219,M,,,,*1e\r\n");
WriteTest("$01$GPGGA,082623,5052.5242,N,00442.4802,E,1,9,14.0,90.0999984741211,M,,,,*2a\r\n");
WriteTest("$01$GPGGA,082627,5052.5202,N,00442.4785,E,1,8,13.0,94.0999984741211,M,,,,*28\r\n");
WriteTest("$01$GPGGA,082629,5052.5185,N,00442.4775,E,1,8,12.0,94.0999984741211,M,,,,*24\r\n");
WriteTest("$01$GPGGA,082630,5052.5174,N,00442.4777,E,1,8,11.0,93.80000305175781,M,,,,*1b\r\n");
WriteTest("$01$GPGGA,082634,5052.5148,N,00442.4770,E,1,9,14.0,93.4000015258789,M,,,,*29\r\n");
WriteTest("$01$GPGGA,082635,5052.5143,N,00442.4764,E,1,9,15.0,93.19999694824219,M,,,,*10\r\n");
WriteTest("$01$GPGGA,082641,5052.5118,N,00442.4745,E,1,8,13.0,94.0999984741211,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082646,5052.5110,N,00442.4737,E,1,4,9.0,98.0,M,,,,*11\r\n");
WriteTest("$01$GPGGA,082649,5052.5095,N,00442.4730,E,1,6,9.0,98.0,M,,,,*17\r\n");
WriteTest("$01$GPGGA,082651,5052.5082,N,00442.4711,E,1,6,9.0,97.4000015258789,M,,,,*1d\r\n");
WriteTest("$01$GPGGA,082654,5052.5058,N,00442.4673,E,1,3,10.0,96.19999694824219,M,,,,*11\r\n");
WriteTest("$01$GPGGA,082655,5052.5051,N,00442.4661,E,1,4,11.0,96.0999984741211,M,,,,*2e\r\n");
WriteTest("$01$GPGGA,082656,5052.5044,N,00442.4646,E,1,4,13.0,96.0,M,,,,*22\r\n");
WriteTest("$01$GPGGA,082700,5052.5021,N,00442.4585,E,1,6,14.0,95.4000015258789,M,,,,*20\r\n");
WriteTest("$01$GPGGA,082701,5052.5019,N,00442.4570,E,1,6,14.0,95.0999984741211,M,,,,*25\r\n");
WriteTest("$01$GPGGA,082704,5052.5020,N,00442.4540,E,1,7,11.0,95.30000305175781,M,,,,*19\r\n");
WriteTest("$01$GPGGA,082710,5052.5011,N,00442.4479,E,1,7,11.0,94.80000305175781,M,,,,*1f\r\n");
WriteTest("$01$GPGGA,082714,5052.4982,N,00442.4458,E,1,8,19.0,95.0,M,,,,*2f\r\n");
WriteTest("$01$GPGGA,082715,5052.4974,N,00442.4454,E,1,7,22.0,95.0,M,,,,*2c\r\n");
WriteTest("$01$GPGGA,082719,5052.4937,N,00442.4486,E,1,7,22.0,95.30000305175781,M,,,,*10\r\n");
WriteTest("$01$GPGGA,082721,5052.4924,N,00442.4497,E,1,7,18.0,95.5,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082722,5052.4919,N,00442.4499,E,1,7,17.0,95.5999984741211,M,,,,*2d\r\n");
WriteTest("$01$GPGGA,082726,5052.4904,N,00442.4487,E,1,7,19.0,95.80000305175781,M,,,,*1e\r\n");
WriteTest("$01$GPGGA,082727,5052.4901,N,00442.4481,E,1,7,24.0,95.80000305175781,M,,,,*12\r\n");
WriteTest("$01$GPGGA,082730,5052.4891,N,00442.4461,E,1,7,46.0,96.0999984741211,M,,,,*2a\r\n");


}
     
	

