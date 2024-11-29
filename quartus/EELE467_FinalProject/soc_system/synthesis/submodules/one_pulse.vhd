library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity one_pulse is
	port (
		clk : in std_ulogic;
		rst : in std_ulogic;
		input : in std_ulogic;
		pulse : out std_ulogic
	);
end entity one_pulse;

architecture one_pulse_arch of one_pulse is

	--instantiate signals for pulsing
	signal pulsed 			: boolean := false; 
	signal current_value : std_ulogic := '0';

begin

	one_pulse : process(clk)
	begin

		--reset signals when reset is asserted
		if rst = '1' then
			pulse  		  <= '0';
			current_value <= '0';
			pulsed 		  <= false;
		elsif rising_edge(clk) then
			--if the "pulsed" signal has been asserted, wait until input returns to 0
			if pulsed then
				pulse  		  <= '0';
				current_value <= '0';
				if input = '0' then
				pulsed <= false;
				end if;				
			elsif current_value /= input then
				pulse  	     <= input;
				current_value <= input;
				pulsed 		  <= true;
			end if;
		end if;
	end process one_pulse;

end architecture one_pulse_arch;