library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity async_conditioner is
	port (
		clk 	: in std_ulogic;
		rst 	: in std_ulogic;
		async : in std_ulogic;
		sync 	: out std_ulogic
	);
end entity async_conditioner;

architecture async_conditioner_arch of async_conditioner is 

	component synchronizer is
	port (
		clk		: in	std_ulogic;
		async	: in	std_ulogic;
		sync	: out	std_ulogic
	);
	end component synchronizer;

	component debouncer is
	generic (
		clk_period : time := 20 ns;
		debounce_time : time := 100 ms
	);
	port (
		clk 			: in std_ulogic;
		rst 			: in std_ulogic;
		input 		: in std_ulogic;
		debounced : out std_ulogic
	);
	end component debouncer;

	component one_pulse is
	port (
		clk : in std_ulogic;
		rst : in std_ulogic;
		input : in std_ulogic;
		pulse : out std_ulogic
	);
	end component one_pulse;

	signal sync_output 		 : std_ulogic := '0';
	signal debounce_output : std_ulogic := '0';

begin

	SYNCHRONIZER_UNIT : component synchronizer 
	port map (
		clk => clk,
		async => async,
		sync => sync_output
	);

	DEBOUNCER_UNIT : component debouncer 
	generic map (
		clk_period 		=> 20 ns,
		debounce_time => 100 ms
	)
	port map (
		clk			 	=> clk,
		rst 			=> rst,
		input 		=> sync_output,
		debounced => debounce_output
	);

	ONE_PULSE_UNIT : component one_pulse 
	port map (
		clk 	=> clk,
		rst 	=> rst,
		input => debounce_output,
		pulse => sync
	);

end architecture async_conditioner_arch;