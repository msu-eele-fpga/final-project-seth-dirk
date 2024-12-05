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
		avs_address 	: in std_ulogic;
		avs_readdata 	: out std_ulogic_vector(31 downto 0);
		avs_writedata 	: in std_ulogic_vector(31 downto 0);
		-- external I/O; export to top-level
		buzzer_out 		: out std_ulogic
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
	signal period_ms			: integer range 1 to 31 := 1;
	signal period_fp			: integer range 2**26 to 31*(2**26):= 2**26;
	signal reg_vol	 			: std_ulogic_vector(31 downto 0) := (others => '0'); --0% duty cycle
	signal vol_fp				: integer range 0 to 2**19 := 1/2*(2**19);			  --50% duty cycle
	signal reg_base_pitch	: std_ulogic_vector(31 downto 0) := (others => '0'); --0 Hz pitch
	
begin

	BUZZER_PITCH : component pwm_controller
	port map(
		clk 				=> clk,
		rst 				=> rst,
		period 			=> unsigned(period),
		duty_cycle 		=> std_logic_vector(reg_vol(19 downto 0)),
		output		 	=> buzzer_out
	);
	
	buzzer_register_read : process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when '0'		=> avs_readdata	<= reg_vol;
				when '1' 	=> avs_readdata 	<= reg_base_pitch;
				when others => avs_readdata 	<= (others => '0');
			end case;
		end if;
	end process;
	
	buzzer_register_write : process(clk, rst)
	begin
		if rst = '1' then		
			reg_vol				<= (others => '0');			--0% duty cycle
			reg_base_pitch		<= (others => '0');	--0 Hz
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when '0' 	=> reg_vol <= avs_writedata;
				when '1' 	=> 
					--Fixed point operations to write buzzer period from frequency in Hz
					reg_base_pitch	<= avs_writedata;
						period_ms <= 1000/to_integer(unsigned(reg_base_pitch));
						period_fp <= period_ms * 2**26;
						period <= std_ulogic_vector(to_unsigned(period_fp, 32));
						reg_vol <= std_ulogic_vector(to_unsigned(vol_fp, 32)); --50% duty cycle
				when others => null;
			end case;
		end if;
	end process;

end architecture;