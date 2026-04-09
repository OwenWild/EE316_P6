--Joshua Smith
--Version 1.1
-- Added more genarics for more control over frequencies

library IEEE;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;

entity PWM_generator is
  generic(
    pwm_bits : integer := 8;
    clock    : integer := 125; --IN MHz
    div0     : integer := 41667;
    div1     : integer := 125000;
    div2     : integer := 0;
    div3     : integer := 0
  );
  port (
    clk     : in std_logic;
    rst     : in std_logic;
    en      : in std_logic;
    freq    : in std_logic_vector(1 downto 0);
    data    : in std_logic_vector(15 downto 0);
    address : out std_logic_vector(7 downto 0); -- Now correctly driven
    pwm_out : out std_logic
  );
end PWM_generator;

architecture behavioral of PWM_generator is

    -- Internal signals using generics for width
    signal cnt_limit : integer range 0 to 3255 := 1;
    signal pwm_cnt   : unsigned(pwm_bits - 1 downto 0) := (others => '0');
    signal clk_cnt   : integer range 0 to 3255 := 0;
    signal addr_reg  : unsigned(7 downto 0) := (others => '0');
    signal sine_duty : unsigned(pwm_bits - 1 downto 0);

begin

    -- 1. Correctly drive the external output port
    address <= std_logic_vector(addr_reg);

    -- 2. Map input data to duty cycle based on generic width
    -- Assuming duty cycle is the top 8 bits of the data bus
    sine_duty <= resize(unsigned(data(15 downto 8)), pwm_bits);

    -- 3. Frequency divider logic
    -- Just the begings of logic for a clock Divider
    cnt_limit <= (clock/div0) when freq = "00" else
                 (clock/div1) when freq = "01" else
                 (clock/div2)  when freq = "10" else
                 (clock/div3)  when freq = "11" else 1;

    ----------------------------------------------------------
    -- Combined Process: Handles Clock division and Address
    ----------------------------------------------------------
    -- Consolidating addr_reg here prevents multiple driver errors
    CONTROL_PROC : process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                clk_cnt  <= 0;
                addr_reg <= (others => '0');
            elsif en = '1' then
                -- Clock Divider
                if clk_cnt < cnt_limit - 1 then
                    clk_cnt <= clk_cnt + 1;
                else
                    clk_cnt <= 0;
                   
                    -- Sample address increments only when the PWM cycle completes
                    -- and the clock divider resets
                    if pwm_cnt = (2**pwm_bits - 1) then
                        addr_reg <= addr_reg + 1;
                    end if;
                end if;
            end if;
        end if;
    end process;

    ----------------------------------------------------------
    -- PWM Generation Process
    ----------------------------------------------------------
   
    PWM_PROC : process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                pwm_cnt <= (others => '0');
                pwm_out <= '0';
            elsif en = '1' then
                -- Only update PWM counter when the frequency divider hits zero
                if cnt_limit = 1 or clk_cnt = 0 then
                   
                    -- Standard Wrap-around logic for 2^n states
                    if pwm_cnt < (2**pwm_bits - 1) then
                        pwm_cnt <= pwm_cnt + 1;
                    else
                        pwm_cnt <= (others => '0');
                    end if;

                    -- Output Comparison
                    if pwm_cnt < sine_duty then
                        pwm_out <= '1';
                    else
                        pwm_out <= '0';
                    end if;
                end if;
            end if;
        end if;
    end process;

end behavioral;
		
