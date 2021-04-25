----------------------------------------------------------------------------------
-- This circuit generate one pulse when the rising edge of the input signal is detected.
-- The pulse is one clock wide and delayed by two system clock periods.

-- Author:
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity pulse_RE2X is Port 
    ( 
        CLK       : in STD_LOGIC;
        SIGNAL_IN : in STD_LOGIC;
        PULSE_OUT : out STD_LOGIC
    );
end pulse_RE2X;

architecture Behavioral of pulse_RE2X is

signal q :std_logic := '0';
signal q0:std_logic := '0';

begin

DFF:    process(CLK)
            begin
                if rising_edge(CLK) then
                    q0 <= SIGNAL_IN;
                    q  <= q0;
                end if;
        end process;
        
        PULSE_OUT <= (not q) and q0;

end Behavioral;