LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;



ENTITY pulse_picker_nondelay IS
	PORT
	(

		PD_CLK_IN		      :  IN	STD_LOGIC;

		GATE    		      :  IN	STD_LOGIC;
        
        LOCK                  :  IN STD_LOGIC;
       
        WIDTH_SET         :  IN	STD_LOGIC_VECTOR(5 downto 0);

        CP                    :  OUT   STD_LOGIC;

        CLK_OUT               :  OUT   STD_LOGIC

	);	
END pulse_picker_nondelay;

ARCHITECTURE a OF pulse_picker_nondelay IS


    SIGNAL	tmp_c1	: STD_LOGIC;
    SIGNAL	tmp1  	: STD_LOGIC_VECTOR(12 downto 0);
    SIGNAL	tmp2  	: STD_LOGIC_VECTOR(5 downto 0);



BEGIN


PROCESS(PD_CLK_IN,GATE)

BEGIN
   IF GATE = '1' THEN 
           tmp1 <="0000000000000";
           tmp_c1 <='1';
   else
       if(PD_CLK_IN'event and PD_CLK_IN ='1')then
             if(tmp1=19)then
               tmp1<="0000000000000";                 --400��Ƶ
               tmp_c1<= not  tmp_c1 ;
             else
              tmp1<=tmp1+1;
              tmp_c1<= tmp_c1;
             end if;
           end if;
    END  IF;


END PROCESS;






PROCESS (PD_CLK_IN,tmp_c1)
	BEGIN	
	IF tmp_c1= '1' THEN 
        tmp2<="111111";
        else
           if(PD_CLK_IN'event and PD_CLK_IN ='0')then
             if(tmp2=10)then                            --10���������
               tmp2<= tmp2;
             else
              tmp2<=tmp2+1;
             end if;
           end if;
    END  IF;
END PROCESS;



process (tmp_c1)
begin

CP<= tmp_c1;
end process;







process(tmp2)
begin
if (tmp2<10)then  
CLK_OUT<='1';
else
CLK_OUT<='0';
end if;
end process;

END a;






