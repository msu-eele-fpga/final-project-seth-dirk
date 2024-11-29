library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity clock_gen is
	port (
		clk				: in std_ulogic;
		rst				: in std_ulogic;
		base_period		: in unsigned(7 downto 0);
		clockBase		: out std_ulogic := '0';
		clock0			: out std_ulogic := '0';
		clock1			: out std_ulogic := '0';
		clock2			: out std_ulogic := '0';
		clock3			: out std_ulogic := '0';
		clock4			: out std_ulogic := '0'
	);
end entity clock_gen;

architecture clock_gen_arch of clock_gen is


	constant SYSTEM_CLOCK_FREQ   : natural := 50000000;
	constant N_BITS_SYS_CLK_FREQ : natural := natural(ceil(log2(real(SYSTEM_CLOCK_FREQ))));
	constant SYS_CLK_FREQ 		  : unsigned(N_BITS_SYS_CLK_FREQ - 1 downto 0) := to_unsigned(SYSTEM_CLOCK_FREQ, N_BITS_SYS_CLK_FREQ);

	constant N_BITS_CLK_CYCLES_FULL : natural := N_BITS_SYS_CLK_FREQ + 8;
	constant N_BITS_CLK_CYCLES      : natural := N_BITS_SYS_CLK_FREQ + 4;

	signal period_base_clk_full_prec : unsigned(N_BITS_CLK_CYCLES_FULL - 1 downto 0);
	signal period_base_clk				: unsigned(N_BITS_CLK_CYCLES - 1 downto 0);

	signal countBase_limit 	: natural;
	signal count0_limit	  	: natural;
	signal count1_limit		: natural;
	signal count2_limit		: natural;
	signal count3_limit		: natural;
	signal count4_limit		: natural;

	signal counterBase		: natural := 0;
	signal counter0			: natural := 0;
	signal counter1			: natural := 0;
	signal counter2			: natural := 0;
	signal counter3			: natural := 0;
	signal counter4			: natural := 0;
	
	signal clockBase_reg		: std_ulogic := '0';	signal clock0_reg			: std_ulogic := '0';
	signal clock1_reg			: std_ulogic := '0';
	signal clock2_reg			: std_ulogic := '0';
	signal clock3_reg			: std_ulogic := '0';
	signal clock4_reg			: std_ulogic := '0';

begin

	period_base_clk_full_prec <= SYS_CLK_FREQ * base_period;
	period_base_clk <= period_base_clk_full_prec(N_BITS_CLK_CYCLES_FULL - 1 downto 4);

	countBase_limit 	<= to_integer(period_base_clk);
	count0_limit		<= to_integer(shift_right((period_base_clk), 1));
	count1_limit		<= to_integer(shift_right((period_base_clk), 2));
	count2_limit		<= to_integer(shift_left((period_base_clk), 1));
	count3_limit	  	<= to_integer(shift_right((period_base_clk), 3));
	count4_limit		<= to_integer(shift_right((period_base_clk), 2));
	
clock_generator : process (clk, rst) is
	begin
		if rst = '1' then
			clockBase_reg <= '0'; clockBase <= '0';
			clock0_reg	  <= '0'; clock0	  <= '0';
			clock1_reg	  <= '0'; clock1	  <= '0';
			clock2_reg	  <= '0'; clock2	  <= '0';
			clock3_reg	  <= '0'; clock3	  <= '0';
			clock4_reg	  <= '0'; clock4	  <= '0';
		elsif rising_edge(clk) then
			if counterBase < countBase_limit then
				counterBase <= counterBase + 1;
			else
				counterBase <= 0;
				clockBase_reg <= not clockBase_reg;
				clockBase <= clockBase_reg;
			end if;

			if counter0 < count0_limit then
				counter0 <= counter0 + 1;
			else
				counter0 <= 1;
				clock0_reg <= not clock0_reg;
				clock0 <= clock0_reg;
			end if;

			if counter1 < count1_limit then
				counter1 <= counter1 + 1;
			else
				counter1 <= 1;
				clock1_reg <= not clock1_reg;
				clock1 <= clock1_reg;
			end if;

			if counter2 < count2_limit then 
				counter2 <= counter2 + 1;
			else
				counter2 <= 1;
				clock2_reg <= not clock2_reg;
				clock2 <= clock2_reg;
			end if;

			if counter3 < count3_limit then 
				counter3 <= counter3 + 1;
			else
				counter3 <= 1;
				clock3_reg <= not clock3_reg;
				clock3 <= clock3_reg;
			end if;

			if counter4 < count4_limit then 
				counter4 <= counter4 + 1;
			else
				counter4 <= 1;
				clock4_reg <= not clock4_reg;
				clock4 <= clock4_reg;
			end if;
		end if;
	end process clock_generator;
end architecture clock_gen_arch;