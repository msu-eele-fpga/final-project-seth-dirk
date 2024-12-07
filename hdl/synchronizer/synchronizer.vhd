library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity synchronizer is
port (
clk : in std_ulogic;
async : in std_ulogic;
sync : out std_ulogic
);
end entity synchronizer;

architecture synchronizer_arch of synchronizer is

	signal meta	: std_ulogic := 'U';
	
begin

DOUBLEFLOP: process(clk)
	begin
		
		if(rising_edge(clk)) then
			meta <= async;
			sync <= meta;
		end if;
	end process;
		
end architecture;