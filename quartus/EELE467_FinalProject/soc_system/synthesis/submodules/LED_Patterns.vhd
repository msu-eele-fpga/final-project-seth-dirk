library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity led_patterns is
   port(
		clk             : in  std_ulogic;                         -- system clock
      rst             : in  std_ulogic;                         -- system reset (assume active high, change at top level if needed)
      push_button     : in  std_ulogic;                         -- Pushbutton to change state (assume active high, change at top level if needed)
      switches        : in  std_ulogic_vector(3 downto 0);      -- Switches that determine the next state to be selected
      hps_led_control : in  boolean;                         		-- Software is in control when asserted (=1)
      base_period     : in  unsigned(7 downto 0);      					-- base transition period in seconds, fixed-point data type (W=8, F=4).
      led_reg         : in  std_ulogic_vector(7 downto 0);      -- LED register
      led             : out std_ulogic_vector(7 downto 0)       -- LEDs on the DE10-Nano board
    );   
end entity led_patterns;

architecture my_architecture of led_patterns is

	component async_conditioner is
  	port (
		clk   : in std_ulogic;
      rst   : in std_ulogic;
     	async : in std_ulogic;
      sync  : out std_ulogic
      );
	end component async_conditioner;

	component pattern_gen is
		port (
			base_clk	 	: in std_ulogic;
			pat_clk		: in std_ulogic;
			rst  			: in std_ulogic;
			sel  			: in natural;
			led 			: out std_ulogic_vector(7 downto 0)
		);
	end component;

	component clock_gen is
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
	end component clock_gen;

	signal countDisplay_limit 	: natural := 50000000;--DisplayTime / system_clock_period;
	signal countDisplay   		: natural := 0;
	signal should_display 		: boolean := false;

	signal sel 			 : natural := 0;										--select line for the pattern generator
	signal pb			 : std_ulogic := '0';								--conditioned push button signal

	type state_type is (DisplaySW, State0, State1, State2, State3, State4);
	signal state : state_type := State0;
	signal PrevState : state_type;											--keep track of previous state for when switches > 5

	--various clock signals from the clock generator
	signal clockBase 	 : std_ulogic;
	signal clock0		 : std_ulogic;
	signal clock1		 : std_ulogic;
	signal clock2		 : std_ulogic;
	signal clock3		 : std_ulogic;
	signal clock4		 : std_ulogic;
	
	--clock signal that gets passed into the pattern generator
	signal pat_clk	 : std_ulogic; 
	
	--signal to hold led_pattern output
	signal led_pat  : std_ulogic_vector(7 downto 0) := "00000000";
	--signal to hold led display based on state machine
	signal led_hold : std_ulogic_vector(7 downto 0) := "00000000";

begin

	ASYNC_COND : component async_conditioner
		port map (
			clk   => clk,
			rst   => rst,
			async => push_button,
			sync  => pb
		);

	PATTERN_GENERATOR : component pattern_gen
		port map (
			base_clk => clockBase,
			pat_clk	=> pat_clk,
			rst 		=> rst,
			sel 		=> sel,
			led		=> led_pat
		);

	CLOCK_GENERATOR : component clock_gen
		port map(
			clk				=> clk,
			rst 				=> rst,
			base_period 	=> base_period,
			clockBase		=> clockBase,
			clock0			=> clock0,   
			clock1			=> clock1, 
			clock2			=> clock2, 
			clock3			=> clock3, 
			clock4			=> clock4
		);

	state_logic : process(clk, rst)
	begin
			if rst = '1' then
				state <= State0;
			elsif rising_edge(clk) then
				case state is		
					when DisplaySW =>
						if countDisplay < (countDisplay_limit - 1) then
							countDisplay <= countDisplay + 1;
							should_display <= true;
						else
							countDisplay <= 0;
							should_display <= false;
						end if;		
			
						if should_display = true then
							state <= DisplaySW;
						elsif switches = "0000" then
							state <= State0;
						elsif switches = "0001" then
							state <= State1;					
						elsif switches = "0010" then
							state <= State2;
						elsif switches = "0011" then
							state <= State3;
						elsif switches = "0100" then
							state <= State4;
						else
							state <= PrevState;		
						end if;
										
					when State0 =>
						if push_button = '1' then
							state <= DisplaySW;
						else
							state <= State0;
						end if;
					when State1 =>
						if push_button = '1' then
							state <= DisplaySW;
						else
							state <= State1;
						end if;						
					when State2 =>
						if push_button = '1' then
							state <= DisplaySW;
						else
							state <= State2;
						end if;
					when State3 =>
						if push_button = '1' then
							state <= DisplaySW;
						else
							state <= State3;
						end if;
					when State4 =>
						if push_button = '1' then
							state <= DisplaySW;
						else
							state <= State4;
						end if;
					when others =>
						state <= DisplaySW;				
				end case;	
			end if;
	end process state_logic;
	
	output_logic : process (state, push_button)
	begin
		case state is		
					when DisplaySW =>
						led_hold(6 downto 0) <= "000" & switches;
						led_hold(7) 			<= led_pat(7);
						sel 				 		<= 0;	 				
 					when State0 =>
						pat_clk 		<= clock0;
						PrevState 	<= state;
						led_hold		<= led_pat;
						sel 			<= 1;				
					when State1 =>
						pat_clk 		<= clock1;
						PrevState 	<= state;
						led_hold		<= led_pat;
						sel 			<= 2;				
					when State2 =>
						pat_clk 		<= clock2;
						PrevState 	<= state;
						led_hold 	<= led_pat;
						sel 			<= 3;					
					when State3 =>
						pat_clk 		<= clock3;
						PrevState 	<= state;
						led_hold		<= led_pat;
						sel 			<= 4;					
					when State4 =>
						pat_clk 		<= clock4;
						PrevState 	<= state;
						led_hold		<= led_pat;
						sel 			<= 5;					
					when others =>
						pat_clk 		<= clockBase;
						led_hold		<= led_pat;
						sel 	  		<= 0;					
				end case;
			end process output_logic;
		
	HPS_LED_CONTROLLER : process(hps_led_control)
	begin
		if hps_led_control = false then
			led <= led_hold;
		else
			led <= led_reg;
		end if;
	end process;
   
end my_architecture;