-- EELE467 Final Project Rotary Encoder 
-- Dirk Kaiser 2024
-- This will instantiate the state machine for the rotary encoder as well as necessary avalon register
-- altera vhdl_input_version vhdl_2008

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity rotary_avalon is
port (
clk : in std_ulogic;
rst : in std_ulogic;
-- avalon memory-mapped slave interface
avs_read : in std_ulogic;
avs_address : in std_ulogic_vector(1 downto 0);
avs_readdata : out std_ulogic_vector(31 downto 0);
-- external input pins from rotary encoder; import from top-level
A : in std_ulogic;
B : in std_ulogic;
push_button : in std_ulogic
);
end entity rotary_avalon;

architecture rotary_avalon_arch of rotary_avalon is 

---------------------- Component Declearations ----------------------------
component async_conditioner is
	port (
		clk		: in std_ulogic;
		rst		: in std_ulogic;
		async	: in std_ulogic;
		sync	: out std_ulogic
		);
end component async_conditioner;

------------------------ Signal Declearations ----------------------------
-- creating registers
signal output_reg : std_ulogic_vector(31 downto 0) := (others => '0');
signal enable_reg : std_ulogic_vector(31 downto 0) := (others => '0');

-- counting variable for encoder state
signal int : integer range 0 to 7 := 0;

-- pushbutton signal
signal pb : std_ulogic;

-- signal for counting enable state
signal en : std_ulogic := '0';

--------------------------------------------------------------------------

begin

------------------------ Enable Signal -----------------------------------
CONDITIONER : component async_conditioner
	port map (
		clk => clk,
		rst => rst,
		async => push_button,
		sync => pb
		);

enable : process(pb,rst)
	begin
		if rst = '1' then
			en <= '0';
		elsif pb = '1' then
			en <= not en;
		end if;
	end process;

enable_reg(31) <= en;
------------------------- Rotary Encoder ---------------------------------
quadrature : process(A,B,rst)
	begin
		if rst = '1' then
			int <= 0;
		elsif rising_edge(A) and B = '1' then
			int <= int + 1;
		elsif rising_edge(A) and B = '0' then
			int <= int - 1;
		end if;
	end process;

output_reg <= std_ulogic_vector(to_unsigned(int,32));

------------------------- Avalon Bus --------------------------------------
avalon_register_read : process(clk)
	begin
		if rising_edge(clk) and avs_read = '1' then
			case avs_address is
				when "00" => avs_readdata <= output_reg;
				when "01" => avs_readdata <= enable_reg;
				when others => avs_readdata <= (others => '0');
			end case;
		end if;
	end process;
	
	
end architecture;