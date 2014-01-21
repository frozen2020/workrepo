LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;



ENTITY pulse_picker_div IS
	PORT
	(
		
    ENABLE                :  IN STD_LOGIC;           --上升沿触发读取DATA和RS
    
    PD_CLK_IN             :  IN STD_LOGIC;           --光电探测器输入

    GATE                  :  IN STD_LOGIC;           
 
    DATA                  :  IN STD_LOGIC_VECTOR(7 downto 0);

    RS                    :  IN STD_LOGIC_VECTOR(1 downto 0);    --01：DATA为脉冲频率;10：DATA为脉冲宽度；11：DATA为跟随延迟。00：DATA为FF则为跟随模式，为0则为自发生模式，F0为打开，0F为关闭。

    CLK                   :  IN STD_LOGIC;           --晶振输入

    PWM_OUT               :  OUT   STD_LOGIC;        --PWM输出
    
    PWM_TEST			  :  OUT   STD_LOGIC;        --PWMtest
            
    RF_ON                 :  OUT   STD_LOGIC    --选脉冲开关 1开，0关
	);	
END pulse_picker_div;

ARCHITECTURE a OF pulse_picker_div IS
	  SIGNAL	tmp_c1	: STD_LOGIC;                  --TTL输出
    SIGNAL  pulserate   : STD_LOGIC_VECTOR(12 downto 0):="0001000000000";    --频率
    SIGNAL  ratetmp   : STD_LOGIC_VECTOR(12 downto 0);    --频率计数
   
    SIGNAL  pulsewide   : STD_LOGIC_VECTOR(3 downto 0):="0001";    --脉宽
    SIGNAL  widetmp   : STD_LOGIC_VECTOR(3 downto 0);    --脉宽计数

    SIGNAL  pulsedelay    : STD_LOGIC_VECTOR(3 downto 0):="0001";     -- TTL延时
    SIGNAL  delaytmp  : STD_LOGIC_VECTOR(3 downto 0);      --TTL延迟触发计数
    SIGNAL  delayflag1   : STD_LOGIC;      -- TTL延迟标志，0为未检测到光电脉冲，1为正在输出PWM。
    SIGNAL  delayflag2 :    STD_LOGIC; --0为等待触发，1为开始计时
    SIGNAL  tmpflag1: STD_LOGIC;
    SIGNAL  tmpflag2: STD_LOGIC;
    --SIGNAL  mode   : STD_LOGIC;    --0自发生，1跟随
    --SIGNAL  switch : STD_LOGIC;    --0关，1开
    SIGNAL  pwmflag :STD_LOGIC;    --1为开始计数占空比。
BEGIN




PROCESS(RS,ENABLE,DATA)
  BEGIN
       if(ENABLE'event and ENABLE ='1')then
            if(RS=0) then
              if (DATA(7 downto 4)="1111") then                
                RF_ON<='1';
           ----     ratetmp<=pulserate;
            ----    widetmp<=pulsewide;
            ----    delaytmp<="0000";
              elsif (DATA(3 downto 0)="1111") then
                RF_ON<='0';
            ----    ratetmp<=pulserate;
             ----   delaytmp<="0000";
            ----    widetmp<=pulsewide;
            -- elsif (DATA(1 downto 0)="10") then
             -- mode<='1';
            --  elsif (DATA(1 downto 0)="01") then
             -- mode<='0';
              end if ; 
             elsif (RS=1) then 
             pulserate(12 downto 5)<=DATA;
			       pulserate(4 downto 0)<="00000";
             elsif (RS=2) then 
             pulsewide<=DATA(3 downto 0);
             elsif (RS=3) then 
             pulsedelay<=DATA(3 downto 0);    
            end if;
       end if;
END PROCESS;



PROCESS(PD_CLK_IN,delayflag2,delayflag1)
  BEGIN
    -- if CLK'event and CLK ='1' then
    --   CLK_tmp='1';
    -- end if ;
    if(PD_CLK_IN'event and PD_CLK_IN ='0')then
      --if (mode='1'and delayflag1='0' and delayflag="00") then
      if ( delayflag1='0' and tmpflag2='0'and delayflag2='0') then
          delayflag1<='1';
      --end if;
      else if (delayflag1='1'and tmpflag2='1')  then
          delayflag1<='0';
          end if ;
        end if;
    end if;
    -- else
    -- delayflag2<='0';
    -- delayflag<='0';
    -- delaytmp<="0000";
end process;
PROCESS(CLK)
  BEGIN
  
if(CLK'event and CLK ='0') then
    if (delayflag1='1' and tmpflag2='0' and delayflag2='0') then 
          if (delaytmp=pulsedelay) then
            delaytmp<="0000";
            delayflag2<='1';
          else delaytmp<=delaytmp+1;
		  end if ;
	else if (delayflag2='1') then
          if (ratetmp="0000000000000") then
            tmp_c1<='1';
            pwmflag<='1';      
          end if ; 
        
          if (pwmflag='1') then
		  widetmp<=widetmp+1;
          if (widetmp=pulsewide) then
          tmp_c1 <='0';
          widetmp<="0000";
          pwmflag<='0'; 
          end if;
          
         end if;
          ratetmp<=ratetmp+1; 
          if (ratetmp=pulserate) then
          ratetmp<="0000000000000";
          delayflag2<='0';
          tmpflag2<='1';
          end if; 
        end if;  
      
     end if; 
       if (delayflag1='0' and tmpflag2='1')then
          tmpflag2<='0'; 
	   	end if;   
	end if;
   
END PROCESS;

-- PROCESS(CLK)
--  BEGIN
--         if(CLK'event and CLK ='1') then
--         CLK_tmp<='1';
-- 		end if;
-- END PROCESS;

process(CLK,GATE,tmp_c1)
begin
  --if(GATE='1') then
  --PWM_OUT <= '1';
  --else
  PWM_OUT <= tmp_c1;
  PWM_TEST <= CLK;
  --end if;
  
end process;

END a;

