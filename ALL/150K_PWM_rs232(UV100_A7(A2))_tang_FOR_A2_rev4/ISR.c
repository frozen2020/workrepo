#include "constants.h"
#include "extern.h"
#include "Proto.h"

/********************** INTERRUPT SERVICE ROUTINES *********************/
/*1. TIMER 1 INTERRUPT ROUTINE   */
interrupt void timer1_isr(void)
{
	*EVAIFRA = 0x0080;      /* clear TIMER1 PERIOD INTERRUPT flagS */
	if( INTEST ) INTEST--;
	if( ADTIMER ) ADTIMER--;
	if(LOOP) LOOP--;
	if(BEEPER)
		{
		if(BTIME > 2) BTIME = 0;
		else BTIME++;
		if(!BTIME) *PFDATDIR = (*PFDATDIR ^ BEEPER_BIT);
	 	}
} 	

/*2. TRIGGER INTERRUPT ROUTINE   */   
interrupt void xint1(void)  //extern trigger: FREQ++, clear interrup
{  
    if((*XINT2CR) & 0x0004)
		{
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//*T4CMPR = FPS_LEVEL;
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		*XINT2CR = 0x8001;
		}
	else
		{
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//*T4CMPR = 100;
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		*XINT2CR = 0x8005;
		}	
}   
	
    //if((*PIRQR0)&0x0004)   //xint1
	//{
//		if( QSWEXT )
//		{
//		*XINT1CR = *XINT1CR | 0x8001;
 //		FREQ++;
   //		}   		
   	//	*PIACKR0=(*PIACKR0)|0x0004;
   	//}
//}
/*   	
   	else if( (*PIRQR0)&0x0008)   //xint2
   	{
   	if(*XINT2CR & 0x0004)
		{
		*T4CMPR = FPS_LEVEL<<2;
		*XINT2CR = 0x8001;
		}
	else
		{
		*T4CMPR = 400;
		*XINT2CR = 0x8005;
		}
	*PIACKR0=(*PIACKR0)|0x0008;		
	}   	
}		 /* End xint1() */


/*3. GATE INTERRUPT ROUTINE   */  
//interrupt void xint2(void)   //1. clear int. 2.set T4 compare register at array index.
//{    
//	if(*XINT2CR & 0x0004)
//		{
//		*T4CMPR = FPS_LEVEL<<2;
//		*XINT2CR = 0x8001;
//		}
//	else
//		{
//		*T4CMPR = 400;
//		*XINT2CR = 0x8005;
//		}
//} 	

/*4 RS232 INTERRUPT*/
interrupt	void    uarttr(void)    		//�жϷ������
{	switch (*PIVR)			//�����ж������Ĵ���PVIR��ֵ�����ǽ��ջ��Ƿ����ж�
	{
        case 6:    sciPolling();break;	//���PIVR=6�������˽����жϣ�ִ�н��շ������
    	case 7:	   echo2(); break;//���IFR����Ӧ���жϱ�־ 
	}
}
