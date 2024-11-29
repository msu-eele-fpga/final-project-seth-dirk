library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity RGB_LED_Control is
	port (
		clk 		: in std_ulogic;
		rst 		: in std_ulogic;
		-- avalon memory-mapped slave interface
		avs_read 		: in std_ulogic;
		avs_write 		: in std_ulogic;
		avs_address 	: in std_ulogic_vector(1 downto 0);
		avs_readdata 	: out std_ulogic_vector(31 downto 0);
		avs_writedata 	: in std_ulogic_vector(31 downto 0);
		-- external I/O; export to top-level
		red_out 			: out std_ulogic;
		blue_out			: out std_ulogic;
		green_out		: out std_ulogic
		);
end entity RGB_LED_Control;

architecture RGB_LED_Control_arch of RGB_LED_Control is

	component pwm_controller is
		generic (
			CLK_PERIOD : time := 20 ns
		);
		port (
			clk : in std_logic;
			rst : in std_logic;
			-- PWM repetition period in milliseconds;
			-- datatype (W.F) is individually assigned
			period : in unsigned(32 - 1 downto 0);
			-- PWM duty cycle between [0 1]; out-of-range values are hard-limited
			-- datatype (W.F) is individually assigned
			duty_cycle : in std_logic_vector(20 - 1 downto 0);
			output : out std_logic
		);
	end component pwm_controller;

	signal reg_period	  		: std_ulogic_vector(31 downto 0) := (26 => '1', others => '0'); --1 ms period
	signal reg_red_duty 		: std_ulogic_vector(31 downto 0) := (others => '0'); --0% duty cycle
	signal reg_green_duty	: std_ulogic_vector(31 downto 0) := (others => '0'); --0% duty cycle
	signal reg_blue_duty	  	: std_ulogic_vector(31 downto 0) := (others => '0'); --0% duty cycle
	
begin

	RED_CONTROL : component pwm_controller
	port map(
		clk 				=> clk,
		rst 				=> rst,
		period 			=> unsigned(reg_period),
		duty_cycle 		=> std_logic_vector(reg_red_duty(19 downto 0)),
		output		 	=> red_out
	);
	
	GREEN_CONTROL : component pwm_controller
	port map(
		clk 				=> clk,
		rst 				=> rst,
		period 			=> unsigned(reg_period),
		duty_cycle 		=> std_logic_vector(reg_green_duty(19 downto 0)),
		output		 	=> green_out
	);
	
	BLUE_CONTROL : component pwm_controller
	port map(
		clk 				=> clk,
		rst 				=> rst,
		period 			=> unsigned(reg_period),
		duty_cycle 		=> std_logic_vector(reg_blue_duty(19 downto 0)),
		output		 	=> blue_out
	);

	rgb_register_read : process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when "00"	=> avs_readdata	<= reg_period;
				when "01" 	=> avs_readdata 	<= reg_red_duty;
				when "10"	=> avs_readdata	<= reg_green_duty;
				when "11"	=> avs_readdata	<= reg_blue_duty;
				when others => avs_readdata 	<= (others => '0');
			end case;
		end if;
	end process;
	
	rgb_register_write : process(clk, rst)
	begin
		if rst = '1' then
			reg_period 			<= (26 => '1', others => '0');	--1 ms period		
			reg_red_duty		<= (others => '0');	--0% duty cycle
			reg_green_duty		<= (others => '0');	--0% duty cycle
			reg_blue_duty		<= (others => '0');	--0% duty cycle
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when "00" 	=> reg_period 		<= avs_writedata;
				when "01" 	=> reg_red_duty	<= avs_writedata;
				when "10" 	=> reg_green_duty	<= avs_writedata;
				when "11"	=> reg_blue_duty	<= avs_writedata;
				when others => null;
			end case;
		end if;
	end process;

end architecture;