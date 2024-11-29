library IEEE;
use ieee.std_logic_1164.all;

entity synchronizer is
	port (
		clk	: in	std_ulogic;
		async	: in	std_ulogic;
		sync	: out	std_ulogic
	);
end entity synchronizer;

architecture synchronizer_arch of synchronizer is

	signal output_meta : std_ulogic;
	
begin

	process(clk)
	begin
		if rising_edge(clk) then
			output_meta <= async;
			sync 			<= output_meta;
		end if;
	end process;

end architecture synchronizer_arch;
	
		
		