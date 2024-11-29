library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity pattern_gen is
	port (
		base_clk	 	: in std_ulogic;
		pat_clk   	: in std_ulogic;
		rst  			: in std_ulogic;
		sel  			: in natural;
		led 			: out std_ulogic_vector(7 downto 0)
	);
end entity pattern_gen;

architecture pattern_gen_arch of pattern_gen is

	--count signals for each pattern
	signal count0		 : natural := 0;
	signal count1		 : natural := 0;
	signal count2		 : natural := 0;
	signal count3		 : natural := 0;
	signal count4		 : natural := 0;

	--led counter signals
	signal upCounter 		: natural range 0 to 127 := 0;
	signal downCounter	: natural range 0 to 127 := 0;

	--led signal for each pattern
	signal led7 : std_ulogic := '0';
	signal led0 : std_ulogic_vector(6 downto 0) := "0000001";
	signal led1 : std_ulogic_vector(6 downto 0) := "0000011";
	signal led2 : std_ulogic_vector(6 downto 0) := "0000000";
	signal led3 : std_ulogic_vector(6 downto 0) := "1111111";
	signal led4 : std_ulogic_vector(6 downto 0) := "0001000";


begin

	--blink led 7 at base rate
	LED_BASE : process (base_clk, rst) is
	begin
		if rst = '1' then
			led7 		<= '0';
		elsif rising_edge(base_clk) then
			led7 <= not led7;
		end if;
	end process;

	--shift one led to the right at 1/2 base rate
	PATTERN0 : process (pat_clk, rst, sel) is
	begin
		if rst = '1' then
			led0 <= "0000001";
		elsif rising_edge(pat_clk) then
			led0 <= led0(0) & led0(6 downto 1);
		end if;
	end process;

	--shift two leds, side by side, to the left at 1/4 base rate
	PATTERN1 : process (pat_clk, rst, sel) is
	begin
		if rst = '1' then
			led1 <= "0000011";
		elsif rising_edge(pat_clk) then
			led1 <= led1(5 downto 0) & led1 (6);
		end if;
	end process;

	--output of a 7 bit up counter at 2x base rate
	PATTERN2 : process (pat_clk, rst) is
	begin
		if rst = '1' then
			led2 <= "0000000";
			upCounter <= 0;
		elsif rising_edge(pat_clk) then
			if upCounter < 128 then
				led2 <= std_ulogic_vector(to_unsigned(upCounter, 7));
				upCounter <= upCounter + 1;
			else 
				led2 <= std_ulogic_vector(to_unsigned(upCounter, 7));
				upCounter <= 0;
			end if;
		end if;
	end process;

	--output of a 7 bit down counter at 1/8 base rate
	PATTERN3 : process (pat_clk, rst, sel) is
	begin
		if rst = '1' then
			led3 <= "1111111";
		elsif rising_edge(pat_clk) then
			if downCounter > 0 then
				led3 <= std_ulogic_vector(to_unsigned(downCounter, 7));
				downCounter <= downCounter - 1;
			else 
				led3 <= std_ulogic_vector(to_unsigned(downCounter, 7));
				downCounter <= 127;
			end if;
		end if;
	end process;

	--user defined: toggle leds and have opposite neighbors at 4x base rate
	PATTERN4 : process (pat_clk, rst, sel) is
	begin
		if rst = '1' then
			led4 <= "0001000";
		elsif rising_edge(pat_clk) then
			led4(6 downto 4) <= led4(5 downto 3);
			led4(2 downto 0) <= led4(3 downto 1);
			led4(3)			  <= led4(0);
		end if;	
	end process;

	MUTLIPLEX : process (sel, led0, led1, led2, led3, led4)
	begin
		case sel is
			when 1 =>
				led(6 downto 0) <= led0;
			when 2 =>
				led(6 downto 0) <= led1;
			when 3 =>
				led(6 downto 0) <= led2;
			when 4 =>
				led(6 downto 0) <= led3;
			when 5 =>
				led(6 downto 0) <= led4;
			when others =>
				led(6 downto 0) <= "0000000";
		end case;
	end process;
	
	led(7) <= led7;

end architecture pattern_gen_arch;