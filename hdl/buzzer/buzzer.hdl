library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity buzzer is
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
		buzzer_out 		: out std_ulogic;
		);
end entity buzzer;

architecture buzzer_arch of buzzer is

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

	signal period		  		: std_ulogic_vector(31 downto 0) := (26 => '1', others => '0'); --1 ms period
	signal reg_pitch	 		: std_ulogic_vector(31 downto 0) := (others => '0'); --0% duty cycle
	signal reg_base_pitch	: std_ulogic_vector(31 downto 0) := (others => '0'); --0 Hz pitch
	
begin

	BUZZER_PITCH : component pwm_controller
	port map(
		clk 				=> clk,
		rst 				=> rst,
		period 			=> unsigned(period),
		duty_cycle 		=> std_logic_vector(reg_pitch(19 downto 0)),
		output		 	=> buzzer_out
	);
	
	buzzer_register_read : process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when "00"	=> avs_readdata	<= reg_pitch;
				when "01" 	=> avs_readdata 	<= reg_base_pitch;
				when others => avs_readdata 	<= (others => '0');
			end case;
		end if;
	end process;
	
	buzzer_register_write : process(clk, rst)
	begin
		if rst = '1' then		
			reg_pitch	<= (others => '0');			--0% duty cycle
			reg_base_pitch		<= (others => '0');	--0 Hz
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when "00" 	=> reg_pitch 		<= avs_writedata;
				when "01" 	=> reg_base_pitch	<= avs_writedata;
				when others => null;
			end case;
		end if;
	end process;

end architecture;