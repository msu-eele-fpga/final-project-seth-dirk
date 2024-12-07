-- HW 6 Async Signal Conditioner
-- Dirk Kaiser 2024

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity async_conditioner is
	port (
		clk		: in std_ulogic;
		rst		: in std_ulogic;
		async	: in std_ulogic;
		sync	: out std_ulogic
		);
end entity async_conditioner;

architecture conditioner_arch of async_conditioner is 

-- Component Declaration:
component synchronizer is
port (
      clk   : in    std_logic;
      async : in    std_ulogic;
      sync  : out   std_ulogic
    );
end component;

component debouncer is
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
end component;

component one_pulse is
	port (
		clk		: in std_ulogic;
		rst		: in std_ulogic;
		input	: in std_ulogic;
		pulse 	: out std_ulogic
		);
end component;

signal synchronizer_output : std_ulogic := '0';
signal debounce_output : std_ulogic := '0';

constant DEBOUNCE_TIME	: time	:= 1000 ns;
constant CLK_PERIOD		: time	:= 20 ns;

begin

SYNCRONIZER : component synchronizer
	port map (
		clk => clk,
		async => async,
		sync => synchronizer_output
		);

DEBOUNCE : component debouncer
	generic map (
		clk_period    => CLK_PERIOD,
        debounce_time => DEBOUNCE_TIME
      )
      port map (
        clk       => clk,
        rst       => rst,
        input     => synchronizer_output,
        debounced => debounce_output
      );
	  
PULSE : component one_pulse
	port map (
		clk => clk,
		rst => rst,
		input => debounce_output,
		pulse => sync
		);

end architecture;