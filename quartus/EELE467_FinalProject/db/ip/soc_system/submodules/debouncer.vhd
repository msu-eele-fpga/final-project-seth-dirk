library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity debouncer is
	generic (
		clk_period : time := 20 ns;
		debounce_time : time
	);
	port (
		clk 		: in std_ulogic;
		rst 		: in std_ulogic;
		input 		: in std_ulogic;
		debounced 	: out std_ulogic
	);
end entity debouncer;

architecture debouncer_arch of debouncer is

	--instantiate signals for debouncing and timing
	signal ignore		: boolean := false;
	signal count		: natural := 0;
	signal past_value 	: std_ulogic := '0';

begin

	debouncer : process(clk) is
	begin
		--reset values when reset signal is high
		if rst = '1' then
			debounced 	<= '0';
			past_value 	<= '0';
			ignore		<= false;
			count		<= 1;
		elsif rising_edge(clk) then
			--ignore the input while the for loop counts for debounce time
			if ignore then
				if count <= ((debounce_time/clk_period)-1) then
					debounced <= past_value;
					count <= count + 1;
						--if time is up, reset the counter and turn off ignore signal
						if count = ((debounce_time/clk_period)-1) then
							ignore <= false;
							count <= 1;
						end if;
				end if;
			--ignore signal is off, so wait for action on the input
			--if the input switches, set the debounce signal to the input and raise the ignore flag
			else
				if past_value /= input then
					debounced  <= input;
					past_value <= input;
					ignore 	   <= true;
				end if;
			end if;
		end if;
	end process debouncer;

end architecture debouncer_arch;