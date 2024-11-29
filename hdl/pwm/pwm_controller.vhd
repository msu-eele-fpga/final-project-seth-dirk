library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity pwm_controller is
	generic (
		CLK_PERIOD : time := 20 ns
	);
	port (
		clk 			: in std_logic;
		rst 			: in std_logic;
		-- PWM repetition period in milliseconds;
		-- datatype (W.F) is individually assigned
		period 		: in unsigned(32 - 1 downto 0);
		-- PWM duty cycle between [0 1]; out-of-range values are hard-limited
		-- datatype (W.F) is individually assigned
		duty_cycle 	: in std_logic_vector(20 - 1 downto 0);
		output 		: out std_logic
	);
end entity pwm_controller;

architecture pwm_controller_arch of pwm_controller is

	constant SYSTEM_CLOCK_FREQ   : natural := integer(real(1 ms / CLK_PERIOD));
	constant N_BITS_SYS_CLK_FREQ : natural := natural(ceil(log2(real(SYSTEM_CLOCK_FREQ))));
	constant SYS_CLK_FREQ 		   : unsigned(N_BITS_SYS_CLK_FREQ - 1 downto 0) := to_unsigned(SYSTEM_CLOCK_FREQ, N_BITS_SYS_CLK_FREQ);

	constant N_BITS_CLK_CYCLES_FULL : natural := N_BITS_SYS_CLK_FREQ + 32;
	constant N_BITS_CLK_CYCLES      : natural := N_BITS_SYS_CLK_FREQ + 6;

	signal period_clk_full_prec : unsigned(N_BITS_CLK_CYCLES_FULL - 1 downto 0);
	signal period_clk				    : unsigned(N_BITS_CLK_CYCLES - 1 downto 0);
	signal duty_clk 						: unsigned(N_BITS_CLK_CYCLES + 19 downto 0);

	signal count_period 	: natural := 0;
	signal count_duty			: natural := 0;
	signal period_limit 	: natural;
	signal duty_limit		  : natural;


begin

	--Calculate the number of clock cycles associated with the period input
	period_clk_full_prec <= (SYS_CLK_FREQ) * period;
	period_clk <= period_clk_full_prec(N_BITS_CLK_CYCLES_FULL - 1 downto 26);
	duty_clk 		<= unsigned(duty_cycle) * period_clk;
	 
	--Get the counting limit for the period and the duty cycle
	period_limit 	<= to_integer(period_clk);
	duty_limit		<= to_integer(duty_clk(N_BITS_CLK_CYCLES + 19 downto 19));


	--Turn output on until the duty cycle percentage is reached
	--Turn output off until the period limit is reached
	PWM_CONTROL : process(clk,rst)
	begin
		if rst = '1' then
			output 				<= '1';
			count_period 	<= 0;
			count_duty 		<= 0;
		elsif rising_edge(clk) then
			if count_period < period_limit then
				count_period <= count_period + 1;
				if count_duty < duty_limit then
					count_duty <= count_duty + 1;
				else
					output <= '0';
				end if;
			else
				count_duty	 <= 0;
				count_period <= 0;
				output 			 <= '1';
			end if;
		end if;
	end process;
			
end architecture; 