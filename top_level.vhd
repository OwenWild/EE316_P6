

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity top_level is
    port(
        CLOCK_50   : in  std_logic;
        
        LCD_SDA    : inout std_logic;
        LCD_SCL    : inout std_logic;
        led1_r     : out std_logic;
        led1_g     : out std_logic;
        led1_b     : out std_logic;
        BTN0       : in std_logic; -- Async reset 
        BTN1       : in std_logic; -- select between 2 analog sources
        BTN2       : in std_logic; -- secte between enable and disable
        BTN3       : in std_logic; -- unused atm
        jb         : out std_logic_vector(3 downto 0); -- pmod LEDs, intensity is based on the voltage, done with duty cycle
        US_Echo    : in std_logic;
        US_Trig    : in std_logic;
        buzzer_out : out std_logic
    );
    
    end top_level;

architecture Behavioral of top_level is

    signal Reset_Master  : std_logic;
    signal oReset        : std_logic; -- Power on Reset
    signal Reset         : std_logic; -- Connect this to the button
--- LCD signals 
    signal rx_data_valid : std_logic;
    signal rx_data       : std_logic_vector(7 downto 0);
    signal first_line    : std_logic_vector(127 downto 0);
    signal second_line   : std_logic_vector(127 downto 0);
    signal lcd_reset     : std_logic := '0';
    -- end lcd signals    
--- DUMMY
    signal PWM_en        : std_logic;
    signal oPWM          : std_logic;
    signal PWM_freq      : std_logic_vector(1 downto 0);
    signal PWM_data      : std_logic_vector(15 downto 0);
    signal PWM_address   : std_logic_vector(7 downto 0);
    -- end dummy
    
    component Reset_Delay IS	
    PORT (
        SIGNAL iCLK : IN std_logic;	
        SIGNAL oRESET : OUT std_logic
			);	
    END component;
    
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
    
    -- PWM component
    component PWM_generator is
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
    end component;


    -------    
    
begin
    Reset                 <= BTN0;
    Reset_Master          <= oReset or Reset;
    buzzer_out            <= oPWM;
    PWM_freq              <= "00";
    PWM_data(15 downto 8) <= "10000000";

    inst_Power_on_Reset: Reset_Delay
        PORT map(
            iCLK   => CLOCK_50,
            oRESET => oReset
	    );	
    
    Inst_lcd_user_logic: lcd_user_logic
        port map (
            iclk            => CLOCK_50,
            dataIn          => x"0000",
            FirstLineInput  => first_line,
            SecondLineInput => second_line,
            resets          => Reset_Master,
            oLCDSDA         => LCD_SDA,
            oLCDSCL         => LCD_SCL
        );
        
    inst_PWM_function : PWM_generator
        generic map(
            pwm_bits => 8,
            clock    => 125, --IN MHz
            div0     => 41667,
            div1     => 125000,
            div2     => 0,
            div3     => 0
       )
       port map(
            clk     => CLOCK_50,
            rst     => Reset_Master,
            en      => '1',
            freq    => PWM_freq,
            data    => PWM_data,
            address => PWM_address, -- Now correctly driven
            pwm_out => oPWM
       );
        
        


end Behavioral;
