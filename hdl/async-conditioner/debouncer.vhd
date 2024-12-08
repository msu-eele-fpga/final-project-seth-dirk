-- HW 6 Debouncer
-- Dirk Kaiser 2024

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity debouncer is
generic (
clk_period : time := 20 ns;
debounce_time : time
);
port (
clk : in std_ulogic;
rst : in std_ulogic;
input : in std_ulogic;
debounced : out std_ulogic
);
end entity debouncer;

architecture debouncer_arch of debouncer is

constant COUNTER_LIMIT : integer := (debounce_time/clk_period); -- constant for max number of counts = debounce time
signal count : natural := 0; -- debounce count signal
signal past : std_ulogic;

begin

	process (clk, rst)
	begin
		if (rst = '1') then
			debounced <= '0';
			count <= 0;
			past <= '0';
			
		elsif (rising_edge(clk)) then
			if (count = 0 and past /= input) then
				past <= input;
				debounced <= input;
				count <= count + 1;
			elsif count > 0 and count < COUNTER_LIMIT - 1 then
				count <= count + 1; -- count up for debounce time
			else
				count <= 0;				
			end if;
		end if;
	end process;
	
end architecture;