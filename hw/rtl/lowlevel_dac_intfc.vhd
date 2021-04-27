

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity lowlevel_dac_intfc is
    Port ( 
            rst          : in STD_LOGIC;
            clk125       : in STD_LOGIC;
            data_word    : in STD_LOGIC_VECTOR(31 downto 0);
            sdata        : out STD_LOGIC;
            lrck         : out STD_LOGIC;
            bclk         : out STD_LOGIC;
            mclk         : out STD_LOGIC;
            latched_data : out STD_LOGIC
          );
end lowlevel_dac_intfc;

architecture Behavioral of lowlevel_dac_intfc is

constant TB0_MAX     : natural := 39; -- 125MHz/40 = 3.125MHz
constant TB1_MAX     : natural := 4;  -- 125MHz/5  = 25 MHz
signal TB0_count     : unsigned(5 downto 0):="000000";
signal TB1_count     : unsigned(2 downto 0):="000";
signal scount        : unsigned(5 downto 0):="000000";
signal en_3p125MHz   : std_logic:='0';
signal en_25MHz      : std_logic:='0';
signal Q             : std_logic;
signal pdata         : std_logic_vector(31 downto 0);
signal rdata         : std_logic:='0';
signal sreg          : std_logic_vector(31 downto 0):=x"00000000";
signal srld          : std_logic;
signal sr_ld         : std_logic;
signal sr_en         : std_logic;
signal reg_data_word : std_logic_vector(31 downto 0):=x"00000000";


begin
RDW:    process(clk125, rst)
            begin
                if rst = '1' then
                    reg_data_word <= x"00000000";
                elsif rising_edge(clk125) then
                    if sr_ld = '1' then
                        reg_data_word <= data_word;
                    end if;
                end if;
        end process;


---------------------------------------------------------
-- Timebase generator - 3.125MHz
---------------------------------------------------------
TBG0:    process(clk125,rst)
            begin
                if rst = '1' then
                    TB0_count <= "000000";
                elsif rising_edge(clk125) then
                    if TB0_count = TB0_MAX then
                        TB0_count <= "000000";
                        en_3p125MHz <= '1';
                    else
                        TB0_count <= TB0_count + 1;
                        en_3p125MHz <= '0';
                    end if;
                end if;
        end process;

---------------------------------------------------------
-- Timebase generator - 12.5 MHz
---------------------------------------------------------
TBG1:    process(clk125,rst)
            begin
                if rst = '1' then
                    TB1_count <= "000";
                elsif rising_edge(clk125) then
                    if TB1_count = TB1_MAX then
                        TB1_count <= "000";
                        en_25MHz <= '1';
                    else
                        TB1_count <= TB1_count + 1;
                        en_25MHz <= '0';
                    end if;
                end if;
        end process;
        
-- Generate 12.5MHz output
TGL:   process(clk125, rst)
            begin
                if rst = '1' then
                    Q <= '0';
                elsif rising_edge(clk125) then
                    if en_25MHz = '1' then
                        Q <= not Q;
                    end if;
                end if;
        end process;
        
----------------------------------------------------------
-- Shift Counter
----------------------------------------------------------
CNT:   process(clk125, rst)
            begin
                if rst = '1' then
                    scount <= "000000";
                elsif rising_edge(clk125) then
                    if en_3p125MHz = '1' then
                        scount <= scount + 1;
                    end if;
                end if;
        end process;
        
 ----------------------------------------------------------
 -- Shift Register
 ----------------------------------------------------------
 -- rdata *** Added to correct mistake in interpretation of spec. ***
 RDT:   process(clk125,rst)
            begin
                if rst = '1' then
                    rdata <= '0'; --x"00000000";
                elsif rising_edge(clk125) then
                    if sr_ld = '1' then
                        rdata <= reg_data_word(0);
                    end if;
                end if;
        end process;
        
 -- modify pdata to meet the requirement for sdata
--pdata(31)          <= data_word(0);
--pdata(30 downto 0) <= data_word(31 downto 1);
pdata(31)          <= rdata;                  -- previous word
pdata(30 downto 0) <= reg_data_word(31 downto 1); -- next word
 
 sr_en <= '1' when en_3p125MHz = '1' and scount(0) = '1' else '0';
 srld <= '1' when en_3p125MHz = '1' and scount = "111111" else '0';
 
 -- delay the shift register and latched data signal by one clock
 REG0:  process(clk125)
            begin
                if rising_edge(clk125) then
                    sr_ld <= srld;
                end if;
        end process;
 
 SRG:  process(clk125, rst)
            begin
                if rst = '1' then
                    sreg <= x"00000000";
                elsif rising_edge(clk125) then
                    if sr_ld = '1' then
                        sreg <= pdata;
                    elsif sr_en = '1' then
                        sreg(31 downto 1) <= sreg(30 downto 0);
                    end if;
                end if;
        end process;
                    
-------------------------------------------------------------
-- Outputs
-------------------------------------------------------------       
mclk         <= Q; --(12.5MHz with 50% duty cycle)
bclk         <= scount(0);
lrck         <= not(scount(5));
sdata        <= sreg(31);                        
latched_data <= sr_ld; 

end Behavioral;
