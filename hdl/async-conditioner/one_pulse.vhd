-- HW 6 One Pulse
-- Dirk Kaiser 2024

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity one_pulse is
	port (
		clk		: in std_ulogic;
		rst		: in std_ulogic;
		input	: in std_ulogic;
		pulse 	: out std_ulogic
		);
end entity one_pulse;

architecture one_pulse_arch of one_pulse is

signal enable : std_ulogic := '0'; -- pulse enable
signal past : std_ulogic := '0'; -- signal to check if state has changed

begin

	process (clk, rst)
	begin
		if (rst = '1') then
			pulse <= '0';
			past <= '0';
			enable <= '0';
			
		elsif (rising_edge(clk)) then
			past <= input;
			if (enable = '0' and past /= input) then
				pulse <= input;
				enable <= '1';
			else
				pulse <= '0';
				enable <= '0';				
			end if;
		end if;
	end process;

end architecture;

