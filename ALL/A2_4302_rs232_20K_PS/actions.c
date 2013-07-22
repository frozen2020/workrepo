#include "constants.h"
#include "extern.h"
#include "Proto.h"

/*************************** ACTIONS ON PARAMETER CHANGES **************************/
void clearstring()
{  
    clearWarning=1;
	//clearScreen();
	if(*PADATDIR & 0x0020)	//KEYSWITCH
	{
	wait(2);
	//PLACE = 65;
	PLACE = 85;
	message(msg16);
	}
	else RESET = 1;
}

/*1. shutterOff */
void shutterOff()
{
	//SHT_ON LED OFF:
	*PBDATDIR = *PBDATDIR | 0x0008; 	
	(*PEDATDIR = *PEDATDIR | 0x0014);  /* Shutter off */
	*CMPR5 = 800; /* Fan HALF SPEED */
	(*PFDATDIR = *PFDATDIR | 0x0003);	
}	

/*2. qswonAction */
void qswonAction()
{ 
	if( QSWON )
  	{
  	//QS_ON LED ON:
  	*PBDATDIR = *PBDATDIR & 0xFFFD;   	
  	(*PFDATDIR = *PFDATDIR & ~0x0010);/* Turn on QSWITCH DRIVER */
	}
	else
	{	
   	*PBDATDIR = *PBDATDIR | 0x0002;
   	(*PFDATDIR = *PFDATDIR | 0x0010); /* Turn off QSWITCH DRIVER */
   	}
}   	 	

/*2.1 */
void alarmAction()
{ 
	QSWON=0;
  	*PBDATDIR = *PBDATDIR | 0x0002;
   	(*PFDATDIR = *PFDATDIR | 0x0010); /* Turn off QSWITCH DRIVER */   	
   	RF_OFF=1;
   	diodeAction();
}   	 						

/*2.2 */
void resetAction()
{ 
	//QSWON=0;
  	//*PBDATDIR = *PBDATDIR & 0xFFFD;   	
  	//(*PFDATDIR = *PFDATDIR & ~0x0010);/* Turn on QSWITCH DRIVER */
  	QSWON=0;
  	*PBDATDIR = *PBDATDIR | 0x0002;
   	(*PFDATDIR = *PFDATDIR | 0x0010); /* Turn off QSWITCH DRIVER */  
   	RF_OFF=1;
   	//*GPTCONA = 0x6046;//++xiao
   	diodeAction(); 	
}   	 						

/*3. pulseRateAction */
void pulseRateAction()
{
	if( PULSERATE > 999 )
  	{
  	*T2CON = 0X9042;  //timer 2 control
   	*T2PR = (40000000 / PULSERATE);
   	*T2CMPR = PWMTime /20 ;  //40,1us: gen pulse PWM
	}
	else
	{
   	*T2CON = 0X9642;
   	*T2PR = (625000 / PULSERATE);
   	*T2CMPR = 1;
   	}
}   					

/*4. keyswOffAction*/
void keyswOffAction (short *pSaveQSExt, short * pSaveQSOn, short * pkey)
{
	*pSaveQSExt = QSWEXT;
	*pSaveQSOn = QSWON;
	DIODECUR = 0;
	*T3CMPR = 0;
	
	//RF_OFF=1; //?
	
	*CMPR4 = 0;							/* LASER FAN OFF */
	*CMPR5 = 1000; 						/* DRIVER Fan HALF SPEED */
	*T1CMPR = 0;						
	port00 = 0x0008;
	wait(20);
	*pkey = 0;
	SHTON = 0;
	*PBDATDIR = *PBDATDIR | 0x0008; 
	(*PEDATDIR = *PEDATDIR | 0x0014); 	/* Shutter off */
	QSWEXT = 0;							/* QSwitch to internal. */	
	//QS_EXT OFF:
	*PADATDIR = *PADATDIR | 0x0080;	/* QSwitch internal LED */
	
	QSWON = 0;							/* QSwitchto off mode. */	
	*PBDATDIR = *PBDATDIR | 0x0002; 
	(*PFDATDIR = *PFDATDIR | 0x0010); 	/* Turn off the RF power. */
	WriteFlag = 1;
	return;
}

/*5. qswextAction */
void qswextAction ()
{
	if( QSWEXT )
   	{
   	*T2CON = 0x8042;			/* STOP COUNTER */		
   	*GPTCONA = 0x6042;			/* TURN OFF LASER */
   	*T2CNT = 0;
   	//*EVAIFRB = 0x0002;			
   	//*EVAIMRB = 0x0002;			/* CLEAR ANY PENDING INTERRUPT */
   	*XINT1CR = 0x8000;			
	TFLAG = 0;
   	*PADATDIR = *PADATDIR | 0xC100; 
   	//QS_EXT ON:
	*PADATDIR = *PADATDIR & 0xFF7F;  	
	
   	(*PBDATDIR = *PBDATDIR & ~0x0080);
   	HOLD[0] = 'E';
   	HOLD[1] = 'X';
  	//wrtchrs(2);
  	//HOLD[0] = HOLD[1] = HOLD[2] = HOLD[3] = HOLD[4] = ' ';
  	PLACE = 88;
  	wrtchrs(2);
	}
	else
	{
   	*XINT1CR = 0x8000;			/* DISABLE EXTERNAL INTERRUPT */	
   	*GPTCONA = 0x6046;
   	*T2CON = 0x9042;
   	*EVAIMRA = 0x0080;
   	
   	//QS_EXT OFF:
	*PADATDIR = *PADATDIR | 0x0080;	/* QSwitch internal LED */
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																				
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																				
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																				
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																				
	(*PBDATDIR = *PBDATDIR | 0x0080);
   	HOLD[0] = 'I';
   	HOLD[1] = 'N';
  	//wrtchrs(2);
  	PLACE = 88;
  	wrtchrs(2);
  	PLACE = 71;
  	num2ascii(PULSERATE, 1);
	*T4CMPR = 100;			/* Maximum output for driver. */
	}
	return;
}					

/*6. shtonAction */
void shtonAction()
{
    PLACE=97;
	if( SHTON )
   	{
   	*PBDATDIR = *PBDATDIR & 0xFFF7;
   	(*PEDATDIR = *PEDATDIR & ~0x0004); /* Shutter on */
   	HOLD[0] = 'N';
   	HOLD[1] = ' ';
   	wrtchrs(2);
	}
	else
	{
   	*PBDATDIR = *PBDATDIR | 0x0008;
	(*PEDATDIR = *PEDATDIR | 0x0004); /* Shutter off */
   	HOLD[0] = 'F';
   	HOLD[1] = 'F';
   	wrtchrs(2);
	}
	return;
}						


/*6. defaultAction */
void defaultAction(short * perrFlag, short * pwrFlag)
{ 
	if (ENVACT > ENVSET)	/* Turn on beeper. */
	{	
	alarmAction();
	DIODECUR = 0;	
	BEEPER = 1;
	port00 = 01;
	wait(20);
	PLACE = 0;
	clearScreen();
	message(msg10);
	PLACE = 70;
	message(msg14);
	while((ENVACT > ENVSET) && KEYSW_ON)
	{
	if( !ADTIMER )	tempctl();
	}
	clearstring();
	TPFLAG = 1;
	FUNCTION = 0;	
	}	
	else if(ENVACT < ENV_MINTEMP)	/* Turn on beeper. */
 	{ 	
 	alarmAction();
	DIODECUR = 0;
	BEEPER = 1;
	clearScreen();
	message(msg10);
	PLACE = 70;
	message(msg15);
	while((ENVACT < ENV_MINTEMP) && KEYSW_ON)
	{
	if( !ADTIMER )	tempctl();
	}
	clearstring();
	TPFLAG = 1;
	FUNCTION = 0;	
	}
	else if (LASACT > (LASSET + 20)) /* Turn on beeper. */
	{
	alarmAction();
	DIODECUR = 0;
	BEEPER = 1;
	clearScreen();
	wait(20);
	message(msg11);
	PLACE = 70;
	message(msg14);
	while((LASACT > LASSET) && KEYSW_ON)
	{
	if( !ADTIMER )	tempctl();
	}
	clearstring();
	TPFLAG = 1;
	FUNCTION = 0;	
	}
	else if(LASACT < LAS_MINTEMP) /* Turn on beeper. */
	{
	alarmAction();
	DIODECUR = 0;
	BEEPER = 1;
	clearScreen();
	message(msg11);
	PLACE = 70;
	message(msg15);
	while((LASACT < LAS_MINTEMP) && KEYSW_ON)
	{
	if( !ADTIMER )	tempctl();
	}
	clearstring();
	TPFLAG = 1;
	FUNCTION = 0;
	}
	else if (!(*PADATDIR & 0x0010))    /* INTERLOCK OPEN ERROR */
	{
	alarmAction();
	DIODECUR = 0;
	BEEPER = 1;
	clearScreen();	
	message(msg12);
	while((!(*PADATDIR & 0x0010)) && KEYSW_ON);
	clearstring();
	TPFLAG = 1;
	FUNCTION = 0;
	}
	else if(D1_ACT > 400)
	{	
	alarmAction();
	BEEPER = 1;
	DIODECUR = 0;				/* TURN OFF LASER */
	clearScreen();
	wait(20);
	message(msg13);
	PLACE = 70;
	message(msg14);
	while((D1_ACT > 400) && KEYSW_ON)
	{
	if( !ADTIMER )	tempctl();
	}
	clearstring();
	TPFLAG = 1;
	FUNCTION = 0;
	}
	else if(D1_ACT < 110)
	{
	alarmAction();
	BEEPER = 1;
	DIODECUR = 0;				/* TURN OFF LASER */
	clearScreen();
	wait(20);
	message(msg13);
	PLACE = 70;
	message(msg15);
	while((D1_ACT < 110) && KEYSW_ON)
	{
	if( !ADTIMER )	tempctl();
	}
	clearstring();
	TPFLAG = 1;
	FUNCTION = 0;
	}
	
	if( TPFLAG && (*perrFlag) )
	{
	alarmAction();
	*perrFlag = 0;
	DIODECUR = 0;						/* Set diode current to zero. */
	Diode_SetPt = 0;
	SHTON = 0;							/* Turn off the shutter */
	*PBDATDIR = *PBDATDIR | 0x0008;
	(*PEDATDIR = *PEDATDIR | 0x0004); 	/* Shutter off */
   	QSWEXT = 0;							/* Set the QSwitch to internal. */
	//QS_EXT OFF:
	*PADATDIR = *PADATDIR | 0x0080;
	
	(*PBDATDIR = *PBDATDIR | 0x0080);
  	QSWON = 0;							/* Set the QSwitch On to off mode. */
  	*PBDATDIR = *PBDATDIR | 0x0002; 
	//*GPTCONA = 0x6042;//++xiao
	(*PFDATDIR = *PFDATDIR | 0x0010); 	/* Turn off the RF power. */
	}
	if( RESET )
	{
	resetAction();
	RESET = 0;
	TPFLAG = 0;
	*pwrFlag = 1;
	*perrFlag = 1;
	FUNCTION = 0;
	PLACE = 0;
	message(msg1);
	PLACE = 64;
	message(msg2);
	PLACE = 71;
	num2ascii(PULSERATE, 1);
	PLACE = 25;
	num2ascii(DIODECUR, 0);
	PLACE = 88;
	HOLD[0] = 'I';
   	HOLD[1] = 'N';
  	wrtchrs(2);
	PLACE = 102;
	HOLD[0] = 'O';
   	HOLD[1] = 'C';
   	wrtchrs(2);
	}   
	return;			 
}   			

/*7. keyswFirstOnAction */
void keyswFirstOnAction(short * psettleDelay, short * pSaveQSExt,short * pSaveQSOn)
{
	
	ReadData();
	
	*PEDATDIR = *PEDATDIR | 0x0010;
	
	WriteFlag = 0;
	//*psettleDelay = 200;	// Wait 2 minutes for laser temp to settle.
	*psettleDelay = 3000;	// Wait 2 minutes for laser temp to settle.
	PLACE = 0;
	port00 = 01;
	wait(20);
	message(msg17);	
	/*WAIT FOR SETTLING DELAY*/
	while((*psettleDelay) && ( KEYSW_ON ))
	{
	wait(10);
	if( !ADTIMER )	tempctl();
	(*psettleDelay)--;
	}
	if(!KEYSW_ON)
	{
	port00 = 01;
	wait(20);
	}
	else
	{
	FUNCTION = 0;
	port00 = 0x000C;
	wait(1);
	PLACE = 0;				// Key switch was just turned on
	message(msg1);			// SIGN ON MESSAGE
	PLACE = 64;
	message(msg2);
	PLACE = 71;
	num2ascii(PULSERATE, 1);
	*PBDATDIR = *PBDATDIR | 0x0008;
	//(*PEDATDIR = *PEDATDIR & ~0X0010);	// ENABLE POWER SUPPLY
	(*PEDATDIR = *PEDATDIR | 0x0004); /* Shutter off */
	*CMPR4 = 2000;							/* LASER FAN ON */
	*CMPR5 = 0; 						/* DRIVER Fan on */ 
	TPFLAG = 0;
	QSWEXT = (*pSaveQSExt);
	if( QSWEXT )
		{
		//QS_EXT ON:
		*PADATDIR = *PADATDIR & 0xFF7F;  			
		(*PBDATDIR = *PBDATDIR & ~0x0080);
		PLACE = 88;
		HOLD[0] = 'E';
		HOLD[1] = 'X';
		wrtchrs(2);		
		/* In external mode do not display frequency. */
		HOLD[0] = HOLD[1] = HOLD[2] = HOLD[3] = HOLD[4] = ' ';
		PLACE = 71;
		wrtchrs(5);
		}
		//QSWON = (*pSaveQSOn);
		
		QSWON=0;		
		*PBDATDIR = *PBDATDIR | 0x0002; 
		(*PFDATDIR = *PFDATDIR | 0x0010); 	/* Turn off the RF power. */
	
		if( QSWON )
			{
 			(*PFDATDIR = *PFDATDIR & ~0x0010);	/* Turn on QSWITCH DRIVER */
		 	*PBDATDIR = *PBDATDIR & 0xFFFD;
		 	}
		 	
		/* don't ramp when key switch on. Current is controller by Diode_On button */
	    DIODECUR = Diode_SetPt;
	    PLACE = 25;
		num2ascii(DIODECUR, 0);
				 	
		/*
		while(DIODECUR < Diode_SetPt)			// ramp the diode current
			{
			DIODECUR++;      				
			if(DIMAX > 250)	*T3CMPR = (DIODECUR<<2);
			else *T3CMPR = (DIODECUR<<2);
		 	wait(100);
			PLACE = 25;
		 	num2ascii(DIODECUR, 0);
		 	} */
		}
		OLD_KEYSW_ON = 1;
		return;
}					

/*8. functionAction */
void functionAction(short * pFuncFlg)
{   //if(FUNCTION!=10)
	//{
/*   ENTER FUNCTION DATA   */
	if( (!(*PFDATDIR & 0x0002)) && (FUNCTION > 1) )
	{
	wait(10);
	if( !(*PFDATDIR & 0x0002) )
	{
	if(FUNCTION == 3) D1_SET = ADJUST;
	
	else if((FUNCTION == 2) && (MODE_SEL==1)) 
	{FPS_ON=ADJUST;
	MODE_DONE=0;
	if((FPS_ON==0)||(FPS_ON==1))
	ADJUST=100;
	else
	ADJUST=FPS_LEVEL;
	}
   	else if((FUNCTION == 2) && (MODE_SEL==0)) 
   	{FPS_LEVEL=ADJUST;
   	port00=0x000C;
   	wait(20);
   	MODE_DONE=1;
   	}	
	else if(FUNCTION == 4) ENVSET = ADJUST;
	else if(FUNCTION == 5) LASSET = ADJUST;
	else if(FUNCTION == 6) XTL1SET = ADJUST;
	else if(FUNCTION == 7) XTL2SET = ADJUST;
	else DIMAX = ADJUST;
	
	Sctr = 20;
	while( !(*PFDATDIR & 0x0002) )
	{
	wait(500);
	if(Sctr) Sctr--;
	else
		{
		*pFuncFlg = 1;
		BEEPER = 1;
		wait(100);
		BEEPER = 0;
		while( !(*PFDATDIR & 0x0002) );
		}
	}
	
	if(FPS_ON==2)
   	*XINT2CR = 0x8005;
   	else
   	{
   	*XINT2CR = 0x8000; 
   	*T4CMPR = 100;  
   	}	 	   	
   		
	if((FUNCTION == 2) && (MODE_SEL==1))
		{   MODE_SEL=0;
			port00 = 39+128;
	 		wait(2);
			port00 = 0x000F;			
		}
	else	
		{
		wait(20);
		WriteData();
		port00 = 14;
		}	
	}
	}

/* SET ROW 80 STROBE */
	*PCDATDIR = ~0x0F80;	
	wait(1);
/*   FUNCTION SELECT INCREMENT   */
	if( !(*PCDATDIR & 0x0001) )
	{
	wait(10);
	if(	!(*PCDATDIR & 0x0001))
	{ 									
	FUNCTION++;
	if(FUNCTION==2) MODE_SEL=1;	
	
	if(!(*pFuncFlg))
		{
		if(FUNCTION > 3) FUNCTION = 3;
		}
		else 
		{
		if( FUNCTION < 4) FUNCTION = 8;	
		if( FUNCTION > 8 ) FUNCTION = 4;
 		}
  		WriteData();
  		if( FUNCTION == 3 ) ADJUST = D1_SET;
  		
  		else if((FUNCTION == 2) && (MODE_SEL==1)) ADJUST = FPS_ON;
   	    else if((FUNCTION == 2) && (MODE_SEL==0)) ADJUST = FPS_LEVEL;
  		
  		else if( FUNCTION == 4 ) ADJUST = ENVSET;
  		else if( FUNCTION == 5 ) ADJUST = LASSET;
  		else if( FUNCTION == 6 ) ADJUST = XTL1SET;
		else if( FUNCTION == 7 ) ADJUST = XTL2SET;
		else if( FUNCTION == 8 ) ADJUST = DIMAX;
 		dspfun();
 		while( !(*PCDATDIR & 0x0001) );
 		}	
 	}		
		
/*   FUNCTION SELECT DECREMENT   */
	if( !(*PCDATDIR & 0x0002) )
	{
 	wait(10);
 	if( !(*PCDATDIR & 0x0002) )
 		{
		FUNCTION--;
		if(FUNCTION==2) MODE_SEL=1;	
		if(!(*pFuncFlg))
		{
		if(FUNCTION < 1) FUNCTION = 1;
		Sctr = 20;
		while( !(*PCDATDIR & 0x0002) )
			{
			wait(500);
			if(Sctr) Sctr--;
			else
			{
			Time = 0;
			BEEPER = 1;
			wait(100);
			BEEPER = 0;
			}
		}
	}
	else 
		{
		if( FUNCTION < 4) FUNCTION = 8;	
		if( FUNCTION > 8 ) FUNCTION = 4;
 		}
	WriteData();
	if( FUNCTION == 3 ) ADJUST = D1_SET;
	
	else if((FUNCTION == 2) && (MODE_SEL==1)) ADJUST = FPS_ON;
   	else if((FUNCTION == 2) && (MODE_SEL==0)) ADJUST = FPS_LEVEL;
   		
  	else if( FUNCTION == 4 ) ADJUST = ENVSET;
  	else if( FUNCTION == 5 ) ADJUST = LASSET;
  	else if( FUNCTION == 6 ) ADJUST = XTL1SET;
	else if( FUNCTION == 7 ) ADJUST = XTL2SET;
	else ADJUST = DIMAX;
 	dspfun();
 	while( !(*PCDATDIR & 0x0002) );
	}
	}
		
/*   FUNCTION VALUES INCREASE   */
  	UPDRAT = 10;
  	while( (!(*PFDATDIR & 0x0001)) && (FUNCTION > 1)&&(*PFDATDIR & 0x0002)) // ++)
  		{
   		port00 = 15;
   		wait(20);
   		if((FUNCTION == 2) && (MODE_SEL==0))
   		{
   			if(FPS_ON==2)
   				ADJUST=ADJUST+5;   		
   			else
   				ADJUST=100;
   		}
   		//****************
		else if((FUNCTION == 2) && (MODE_SEL==1))
		{
			;
		}
   		else
   		{ 
   			ADJUST++;
   		}
   		//;
		//****************
// 		if((FUNCTION == 3) && (ADJUST > 350)) ADJUST = 300;
   		if((FUNCTION == 3) && (ADJUST > 400)) ADJUST = 350;
   	//fsp//
   	//************************************************************************
   		//else if((FUNCTION == 2) && (MODE_SEL==1) && (ADJUST > 2)) ADJUST = 2;
   	//************************************************************************
   		else if((FUNCTION == 2) && (MODE_SEL==0) && (ADJUST > 100)) ADJUST = 100;
   		//########################################################
   		//else if((FUNCTION == 4) && (ADJUST > 350)) ADJUST = 350;
   		else if((FUNCTION == 4) && (ADJUST > 400)) ADJUST = 400;
   		//########################################################
   		else if((FUNCTION == 5) && (ADJUST > 450)) ADJUST = 450;
   		else if((FUNCTION == 6) || (FUNCTION == 7))
   		{
   		if(ADJUST < 260) ADJUST = 260;        //400
   		else if(ADJUST > 600) ADJUST = 600;
   		}
   		else if((FUNCTION == 8) && (ADJUST > 480)) ADJUST = 480;
   		if((FUNCTION==2)&&(MODE_SEL==1))
   		{
   			//***********************
   			/*PLACE = 25;   		
	 		HOLD[0]=ADJUST+'0';
   	 		wrtchrs(1);
   	 		FPS_ON=ADJUST;
   	 		if(FPS_ON==2)
   	 			*XINT2CR = 0x8005;
   	 		else
   	 		{
   	 			*XINT2CR = 0x8000; 
   	 			*T4CMPR = 100;  
   	 		}*/
   	 		;
   	 		//***********************	 	   	 	
   	 	}   		
   		else if((FUNCTION==2)&&(MODE_SEL==0))
   		{
   			PLACE = 33;
	 		num2ascii(((short)(ADJUST/5))*10, 0);
	 		FPS_LEVEL=ADJUST;
	 	}
   		else if(FUNCTION==8)          //   +++
   		{                             //   +++
   		  PLACE = 80;                 //   +++
   		  num2ascii(ADJUST, 0);       //   +++
   		  PLACE = 95;                 //   ++
   		  num2ascii(QSWEN, 3);        //   ++
   		}                             //   ++
   		else
   		{
   		PLACE = 35;   		
   		num2ascii(ADJUST,0);
   		}
   		if( UPDRAT ) wait(500);
		else wait(50);
		if( UPDRAT ) UPDRAT--;
		}		
/*   DECREASE FUNCTION VALUE   */ 
  		UPDRAT = 10;
  		while( (!(*PFDATDIR & 0x0002)) && (FUNCTION > 1)&&(*PFDATDIR & 0x0001))//++
  		{
    	port00 = 15;
    	wait(20);
    	
    	if((FUNCTION == 2) && (MODE_SEL==0))
    	{
    		if(FPS_ON==2)
    		{
    			if(ADJUST >5) ADJUST=ADJUST-5;
    	  		else ADJUST=0;
    	  	}   		
   			else
   				ADJUST=100;
   		}
   		//****************************************
   		else if((FUNCTION == 2) && (MODE_SEL==1))
		{
			;
		}
   		else
   		{ 
   			if ( ADJUST ) ADJUST--;
   		}    	    	
		//else 
		//{
			//****************************
			//if ( ADJUST ) ADJUST--;
			//;
			//****************************
		//}
		//**********************************
		/*if((FUNCTION == 2) && (MODE_SEL==1)) 
		{
			if (ADJUST <0) FPS_ON=0;
		}
		else if((FUNCTION == 3) || (FUNCTION == 4) || (FUNCTION == 5))*/
		if((FUNCTION == 3) || (FUNCTION == 4) || (FUNCTION == 5))
   	    //**********************************		
   			{
   			if(ADJUST < 150) ADJUST = 150;
   			}
   		if((FUNCTION == 6) || (FUNCTION == 7))
   			{
   			if(ADJUST < 260) ADJUST = 260;    //400
   			}
   		if(FUNCTION==2)
   		{	
   			if(MODE_SEL==1)
   			{
   				//******************
   				/*PLACE = 25;
	 			HOLD[0]=ADJUST+'0';
	 			wrtchrs(1);
	 			FPS_ON=ADJUST;*/
	 			;
	 			//******************
	 		}
	 		else
	 		{	 	
	 			PLACE = 33;
	 	    	num2ascii(((short)(ADJUST/5))*10, 0);	 		 		 		
	 	    	FPS_LEVEL=ADJUST;
	 		}
	 	}
	 	else if(FUNCTION==8)          //   +++
   		{                             //   +++
   		  PLACE = 80;                 //   +++
   		  num2ascii(ADJUST, 0);       //   +++
   		  PLACE = 95;
   		  num2ascii(QSWEN, 3);
   		} 
	 	else
	 	{   		
   		PLACE = 35;
   		num2ascii(ADJUST, 0);
   		}
		if( UPDRAT ) wait(500);
		else wait(50);
		if( UPDRAT ) UPDRAT--;
		}
		
/*   CHANGE QSWEN VALUE   */ 	         //新增加的组合键	
UPDRAT=10;		                                                            //  ++
if(FUNCTION == 8)
{
 while( (!(*PFDATDIR & 0x0002)) && (!(*PFDATDIR & 0x0001))) //  ++		
 {
  QSWEN=!QSWEN;                         //  ++
  PLACE = 95;                             
  num2ascii(QSWEN, 3);                  
  if( UPDRAT ) wait(500);
  else wait(50);
  if( UPDRAT ) UPDRAT--;
 }
}             
//*******************************************************
/*   CHANGE QSWEN VALUE   */ 	         //新增加的组合键2	
UPDRAT=10;		                                                            //  ++
if((FUNCTION == 2) && (MODE_SEL==1))
{
 while( (!(*PFDATDIR & 0x0002)) && (!(*PFDATDIR & 0x0001))) //  ++		
 {
  port00 = 15;
  wait(20);
  ADJUST++;
  if (ADJUST>2) ADJUST=0; 
  //******************************
  PLACE = 25;   		
  HOLD[0]=ADJUST+'0';
  wrtchrs(1);
  FPS_ON=ADJUST;
  if(FPS_ON==2)
   	 *XINT2CR = 0x8005;
  else
  {
   	 *XINT2CR = 0x8000; 
   	 *T4CMPR = 100;  
  }
  //******************************               
  if( UPDRAT ) wait(500);
  else wait(50);
  if( UPDRAT ) UPDRAT--;
 }
}
//*******************************************************		
		
		if(FPS_ON==1)
		*PEDATDIR = *PEDATDIR | 0x0040; 
		else
		*PEDATDIR = *PEDATDIR & 0xFFBF; 		
//	}
}	

/*9.  diodeOff */
void diodeAction()
{
	if(RF_OFF)
	*PEDATDIR = *PEDATDIR | 0x0010; 	/* Turn off the RF power and LED will be ON ?. */
	else
	{	
	//before ramp, set current to zero
	DIODECUR=0;
	*T3CMPR = 0;
	*PEDATDIR = *PEDATDIR & 0xFFEF; 	/* Turn ON the RF power and the LED will be OFF ? */
	
	while(DIODECUR < Diode_SetPt)			// ramp the diode current
			{
			DIODECUR++;      				
			if(DIMAX > 250)	*T3CMPR = (DIODECUR<<2);
			else *T3CMPR = (DIODECUR<<2);
		 	wait(100);
			PLACE = 25;
		 	num2ascii(DIODECUR, 0);
		 	}
	}
}	


/*10 */
void clearScreen()
{
	PLACE = 0;
	port00 = 01;
	wait(20);
	port00 = 01;
	wait(20);
	port00 = 01;
	wait(20);
}	