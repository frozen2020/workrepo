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
            
    RF_ON                 :  OUT   STD_LOGIC         --选脉冲开关 1开，0关

	);	
END pulse_picker_div;

ARCHITECTURE a OF pulse_picker_div IS

 --   SIGNAL	tmp_c	: STD_LOGIC;
    SIGNAL	tmp_c1	: STD_LOGIC;                  --TTL输出
    SIGNAL  pulserate   : STD_LOGIC_VECTOR(12 downto 0):="0001000000000";    --频率
    SIGNAL  ratetmp   : STD_LOGIC_VECTOR(12 downto 0);    --频率计数
   
    SIGNAL  pulsewide   : STD_LOGIC_VECTOR(3 downto 0):="0001";    --脉宽
    SIGNAL  widetmp   : STD_LOGIC_VECTOR(3 downto 0);    --脉宽计数

    SIGNAL  pulsedelay    : STD_LOGIC_VECTOR(3 downto 0):="0001";     -- TTL延时
    SIGNAL  delaytmp  : STD_LOGIC_VECTOR(3 downto 0);      --TTL延迟触发计数
    SIGNAL  delayflag   : STD_LOGIC;      -- TTL延迟标志，0为未检测到光电脉冲，1为正在输出PWM。
    SIGNAL  delayflag2 :    STD_LOGIC; --0为等待触发，1为开始计时
    SIGNAL  mode   : STD_LOGIC;    --0自发生，1跟随
    SIGNAL  switch : STD_LOGIC;    --0关，1开
    SIGNAL  pwmflag :STD_LOGIC;    --1为开始计数占空比。

BEGIN




PROCESS(RS,ENABLE,DATA)
  BEGIN
       if(ENABLE'event and ENABLE ='1')then
            if(RS=0) then
              if (DATA="11111111") then
                mode<='1';
           --     ratetmp<=pulserate;
            --    widetmp<=pulsewide;
            --    delaytmp<="0000";
              elsif (DATA="00000000") then
                mode<='0';
            --    ratetmp<=pulserate;
             --   delaytmp<="0000";
            --    widetmp<=pulsewide;
              elsif (DATA="11110000") then
              switch<='1';
              elsif (DATA="00001111") then
              switch<='0';
              end if ; 
             elsif (RS=1) then 
             pulserate(12 downto 5)<=DATA;
            -- ratetmp<=pulserate;

             elsif (RS=2) then 
             pulsewide<=DATA(3 downto 0);
            -- widetmp<=pulsewide;
             elsif (RS=3) then 
             pulsedelay<=DATA(3 downto 0);    
            end if;
       end if;
END PROCESS;


-- PROCESS(widetmp,ratetmp,pulserate,pulsewide)
--   BEGIN
--           if (ratetmp>=pulserate) then
--             ratetmp<="0000000000000000";
--             tmp_c1 <='0'; 
--           elsif (widetmp>=pulsewide) then
--             widetmp<="00000000";
--             tmp_c1 <='1';
--           end if;
-- END PROCESS;


PROCESS(PD_CLK_IN,mode,delayflag)
  BEGIN
      if (mode='1') then
        if (delayflag='0') then
          if(PD_CLK_IN'event and PD_CLK_IN ='1')then
            delayflag2<='1';
          end if;
        end if ;
       else
		delayflag2<='0';
      end if;
      -- if expression then
      --      delayflag2<='0';
      -- end if ;
END PROCESS;

PROCESS(CLK_tmp,GATE,mode,pulsedelay,delayflag2,delaytmp)
  BEGIN
   
      if (mode='1') then
        if (delayflag2='1' and delayflag='0') then
          if(CLK'event and CLK ='1') then
            delaytmp<=delaytmp+1;
          end if;
          if (delaytmp>=pulsedelay) then
              delayflag<='1';
              delaytmp<="0000";
          end if ;
        end if;
        -- if (delayflag="10") then
        --   if(CLK'event and CLK ='1') then
        --     ratetmp<=ratetmp+1;
        --     widetmp<=widetmp+1;
        --   end if;
        -- end if;
      else
          delayflag<='0';
          delaytmp<="0000";
      END IF;
END PROCESS;

PROCESS(CLK_tmp,GATE,mode,pulsedelay,delayflag)
  BEGIN
    if (delayflag='1' or mode='0') then
        if(CLK'event and CLK ='1') then
          if (ratetmp="0000000000000") then
            tmp_c1<='1';
            pwmflag<='1';
          end if ;
          ratetmp<=ratetmp+1; 
          if (ratetmp>=pulserate) then
          ratetmp<="0000000000000";
            if (mode='1') then
            delayflag<='0';
            end if;
          end if; 
          if(pwmflag='1') then
          widetmp<=widetmp+1;
            if (widetmp>=pulsewide) then
            widetmp<="0000";
            tmp_c1 <='0';
            pwmflag<='0';
            end if;
          end if;
          -- CLK_tmp<='0';
        end if;   
    end if;
END PROCESS;

-- PROCESS(CLK)
--  BEGIN
--         if(CLK'event and CLK ='1') then
--         CLK_tmp<='1';
-- 		end if;
-- END PROCESS;

process(tmp_c1,switch,GATE)
begin
  if(GATE='1') then
  PWM_OUT <= '1';
  else
  PWM_OUT <= not tmp_c1;
  end if;
RF_ON   <= switch ;
end process;

END a;

