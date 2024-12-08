library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity led_array is
	port (
		clk 		: in std_ulogic;
		rst 	: in std_ulogic;
		-- avalon memory-mapped slave interface
		avs_read 		: in std_ulogic;
		avs_write 		: in std_ulogic;
		avs_address 	: in std_ulogic;
		avs_readdata 	: out std_ulogic_vector(31 downto 0);
		avs_writedata 	: in std_ulogic_vector(31 downto 0);
		-- external I/O; export to top-level
		led 				: out std_ulogic_vector(7 downto 0)
		);
end entity led_array;

architecture led_array_arch of led_array is
	
	
	signal led_reg				 		 : std_ulogic_vector(31 downto 0) := (others => '0');
	
begin

	avalon_register_read : process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when '0'	=> avs_readdata	<= led_reg;
				when others => avs_readdata 	<= (others => '0');
			end case;
		end if;
	end process;
	
	avalon_register_write : process(clk, rst)
	begin
		if rst = '1' then
			led_reg <= (others => '0');
		elsif rising_edge(clk) and avs_write = '1' then
			case avs_address is
				when '0' 	=> led_reg 	<= avs_writedata;
				when others => null;
			end case;
		end if;
	end process;
	
	led <= led_reg(7 downto 0);

end architecture;