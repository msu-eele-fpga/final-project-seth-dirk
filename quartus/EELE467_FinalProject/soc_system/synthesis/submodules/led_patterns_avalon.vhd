library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity led_patterns_avalon is
	port (
		clk : in std_ulogic;
		reset 	: in std_ulogic;
		-- avalon memory-mapped slave interface
		avs_read 		: in std_ulogic;
		avs_write 		: in std_ulogic;
		avs_address 	: in std_ulogic_vector(1 downto 0);
		avs_readdata 	: out std_ulogic_vector(31 downto 0);
		avs_writedata 	: in std_ulogic_vector(31 downto 0);
		-- external I/O; export to top-level
		push_button 	: in std_ulogic;
		switches 		: in std_ulogic_vector(3 downto 0);
		led 				: out std_ulogic_vector(7 downto 0)
		);
end entity led_patterns_avalon;

architecture led_patterns_avalon_arch of led_patterns_avalon is

	component led_patterns is
   port(
		clk             : in  std_ulogic;                         -- system clock
      rst             : in  std_ulogic;                         -- system reset (assume active high, change at top level if needed)
      push_button     : in  std_ulogic;                         -- Pushbutton to change state (assume active high, change at top level if needed)
      switches        : in  std_ulogic_vector(3 downto 0);      -- Switches that determine the next state to be selected
      hps_led_control : in  boolean;                         	 -- Software is in control when asserted (=1)
      base_period     : in  unsigned(7 downto 0);      			 -- base transition period in seconds, fixed-point data type (W=8, F=4).
      led_reg         : in  std_ulogic_vector(7 downto 0);      -- LED register
      led             : out std_ulogic_vector(7 downto 0)       -- LEDs on the DE10-Nano board
    );   
	end component led_patterns;
	
	signal hps_led_control_convert : boolean := false;
	signal reg_hps_led_control 	 : std_ulogic_vector(31 downto 0) := (others => '0');
	signal reg_base_period	  		 : std_ulogic_vector(31 downto 0) := x"00000004";
	signal reg_led_reg				 : std_ulogic_vector(31 downto 0) := (others => '0');
	
begin

	BOOLEAN_CONVERT : process (reg_hps_led_control)
	begin
		if reg_hps_led_control(0) = '1' then
			hps_led_control_convert <= true;
		elsif reg_hps_led_control(0) = '0' then
			hps_led_control_convert <= false;
		end if;
	end process;

	LED_PATTERN_COMPONENT : component led_patterns
		port map (
			clk 					=> clk,
			rst 					=> reset,
			push_button			=> push_button,
			switches 			=> switches,
			hps_led_control 	=> hps_led_control_convert,
			base_period 		=> unsigned(reg_base_period(7 downto 0)),
			led_reg 				=> reg_led_reg(7 downto 0),
			led 					=> led
		);

	avalon_register_read : process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when "00"	=> avs_readdata	<= reg_base_period;
				when "01" 	=> avs_readdata 	<= reg_led_reg;
				when "10"	=> avs_readdata	<= reg_hps_led_control;
				when others => avs_readdata 	<= (others => '0');
			end case;
		end if;
	end process;
	
	avalon_register_write : process(clk, reset)
	begin
		if reset = '1' then
			reg_hps_led_control 	<= (others => '0');			
			reg_base_period		<= x"00000004";
			reg_led_reg			 	<= (others => '0');
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when "00" 	=> reg_base_period 		<= avs_writedata;
				when "01" 	=> reg_led_reg	 			<= avs_writedata;
				when "10" 	=> reg_hps_led_control 	<= avs_writedata;
				when others => null;
			end case;
		end if;
	end process;
	
	

end architecture led_patterns_avalon_arch;