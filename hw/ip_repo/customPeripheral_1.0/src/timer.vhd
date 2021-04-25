----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 04/21/2021 12:09:33 AM
-- Design Name: 
-- Module Name: timer - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity timer is
Port ( clk         : in std_logic;
       rstn        : in std_logic;
       counter_out : out std_logic_vector(31 downto 0)
);
end timer;

architecture Behavioral of timer is
signal counter_data : unsigned(31 downto 0);
begin

process(clk)
begin
    if(rstn = '0') then
        counter_data <= (others => '0');
    elsif(rising_edge(clk)) then
        counter_data <= counter_data + 1;
    end if;
end process;

counter_out <= std_logic_vector(counter_data);

end Behavioral;
