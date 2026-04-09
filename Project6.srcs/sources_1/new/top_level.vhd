

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity top_level is
    port(
        CLOCK_50   : in  std_logic;
        
        LCD_SDA    : inout std_logic;
        LCD_SCL    : inout std_logic;
        led1_r : out std_logic;
        led1_g : out std_logic;
        led1_b : out std_logic;
        BTN0 : in std_logic; -- Async reset 
        BTN1 : in std_logic; -- select between 2 analog sources
        BTN2 : in std_logic; -- secte between enable and disable
        BTN3 : in std_logic; -- unused atm
        jb : out std_logic_vector(3 downto 0); -- pmod LEDs, intensity is based on the voltage, done with duty cycle
        US_Echo : in std_logic;
        US_Trig : in std_logic;
        buzzer_out : out std_logic
    );
    
    end top_level;

architecture Behavioral of top_level is

--- LCD signals 
    signal rx_data_valid : std_logic;
    signal rx_data       : std_logic_vector(7 downto 0);
    signal first_line    : std_logic_vector(127 downto 0);
    signal second_line   : std_logic_vector(127 downto 0);
    signal lcd_reset     : std_logic := '0';
    -- end lcd signals
    
    
    
    
    
    -- LCD component
    component lcd_user_logic is
        port(
            iclk            : in    std_logic;
            dataIn          : in    std_logic_vector(15 downto 0);
            FirstLineInput  : in    std_logic_vector(127 downto 0);
            SecondLineInput : in    std_logic_vector(127 downto 0);
            resets          : in    std_logic;
            oLCDSDA         : inout std_logic;
            oLCDSCL         : inout std_logic);
    end component;
    
    -------
    
    
    
    
    
begin


    Inst_lcd_user_logic: lcd_user_logic
        port map (
            iclk            => CLOCK_50,
            dataIn          => x"0000",
            FirstLineInput  => first_line,
            SecondLineInput => second_line,
            resets          => lcd_reset,
            oLCDSDA         => LCD_SDA,
            oLCDSCL         => LCD_SCL
        );
        
        
        


end Behavioral;
