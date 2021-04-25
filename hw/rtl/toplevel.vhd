

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity toplevel is Port 
    ( 
     -- Clock and reset 
        reset_pb 	: in STD_LOGIC;
		sysclk   	: in STD_LOGIC;
     -- LEDS and DIPS
	    led  	    : out STD_LOGIC_VECTOR (3 downto 0);
	    sw          : in  STD_LOGIC_VECTOR (3 downto 0);
	 -- Audio Codec
	    ac_bclk     : out   STD_LOGIC;
	    ac_mclk     : out   STD_LOGIC;
	    ac_muten    : out   STD_LOGIC;
	    ac_pbdat    : out   STD_LOGIC;
	    ac_pblrc    : out   STD_LOGIC;
	    ac_recdat   : in    STD_LOGIC;  -- not used
	    ac_reclrc   : out   STD_LOGIC;  -- '0'
	    ac_scl      : inout STD_LOGIC;
	    ac_sda      : inout STD_LOGIC;
	 -- Uart
--	    uart_rx     : in  STD_LOGIC;
--	    uart_tx     : out STD_LOGIC;
	    
	    DDR_addr : inout STD_LOGIC_VECTOR ( 14 downto 0 );
        DDR_ba : inout STD_LOGIC_VECTOR ( 2 downto 0 );
        DDR_cas_n : inout STD_LOGIC;
        DDR_ck_n : inout STD_LOGIC;
        DDR_ck_p : inout STD_LOGIC;
        DDR_cke : inout STD_LOGIC;
        DDR_cs_n : inout STD_LOGIC;
        DDR_dm : inout STD_LOGIC_VECTOR ( 3 downto 0 );
        DDR_dq : inout STD_LOGIC_VECTOR ( 31 downto 0 );
        DDR_dqs_n : inout STD_LOGIC_VECTOR ( 3 downto 0 );
        DDR_dqs_p : inout STD_LOGIC_VECTOR ( 3 downto 0 );
        DDR_odt : inout STD_LOGIC;
        DDR_ras_n : inout STD_LOGIC;
        DDR_reset_n : inout STD_LOGIC;
        DDR_we_n : inout STD_LOGIC;
        FIXED_IO_ddr_vrn : inout STD_LOGIC;
        FIXED_IO_ddr_vrp : inout STD_LOGIC;
        FIXED_IO_mio : inout STD_LOGIC_VECTOR ( 53 downto 0 );
        FIXED_IO_ps_clk : inout STD_LOGIC;
        FIXED_IO_ps_porb : inout STD_LOGIC;
        FIXED_IO_ps_srstb : inout STD_LOGIC
	  -- DDS test ports (for testbench simulation)
	   --s_axis_dds_config_tvalid : IN STD_LOGIC;
       --s_axis_dds_config_tdata  : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
       --m_axis_dds_data_tvalid   : OUT STD_LOGIC;
       --m_axis_dds_data_tdata    : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)  
	    );
end toplevel;

architecture Behavioral of toplevel is

-------------------------------------------------
-- Components
-------------------------------------------------

--zedboard
component clk_wiz_0
port
 (-- Clock in ports
  -- Clock out ports
  clk_out1          : out    std_logic;
  clk_in1           : in     std_logic
 );
end component;

component proc_system1 is
  port (
    gpio_rtl_0_tri_i : in STD_LOGIC_VECTOR ( 3 downto 0 );
    gpio_rtl_1_tri_o : out STD_LOGIC_VECTOR ( 3 downto 0 );
    Tuner_tri_o : out STD_LOGIC_VECTOR ( 31 downto 0 );
    DDS_config_tri_o : out STD_LOGIC_VECTOR ( 31 downto 0 );
    iic_rtl_0_scl_i : in STD_LOGIC;
    iic_rtl_0_scl_o : out STD_LOGIC;
    iic_rtl_0_scl_t : out STD_LOGIC;
    iic_rtl_0_sda_i : in STD_LOGIC;
    iic_rtl_0_sda_o : out STD_LOGIC;
    iic_rtl_0_sda_t : out STD_LOGIC;
    FIXED_IO_mio : inout STD_LOGIC_VECTOR ( 53 downto 0 );
    FIXED_IO_ddr_vrn : inout STD_LOGIC;
    FIXED_IO_ddr_vrp : inout STD_LOGIC;
    FIXED_IO_ps_srstb : inout STD_LOGIC;
    FIXED_IO_ps_clk : inout STD_LOGIC;
    FIXED_IO_ps_porb : inout STD_LOGIC;
    DDR_cas_n : inout STD_LOGIC;
    DDR_cke : inout STD_LOGIC;
    DDR_ck_n : inout STD_LOGIC;
    DDR_ck_p : inout STD_LOGIC;
    DDR_cs_n : inout STD_LOGIC;
    DDR_reset_n : inout STD_LOGIC;
    DDR_odt : inout STD_LOGIC;
    DDR_ras_n : inout STD_LOGIC;
    DDR_we_n : inout STD_LOGIC;
    DDR_ba : inout STD_LOGIC_VECTOR ( 2 downto 0 );
    DDR_addr : inout STD_LOGIC_VECTOR ( 14 downto 0 );
    DDR_dm : inout STD_LOGIC_VECTOR ( 3 downto 0 );
    DDR_dq : inout STD_LOGIC_VECTOR ( 31 downto 0 );
    DDR_dqs_n : inout STD_LOGIC_VECTOR ( 3 downto 0 );
    DDR_dqs_p : inout STD_LOGIC_VECTOR ( 3 downto 0 );
    S00_AXIS_0_tdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    S00_AXIS_0_tstrb : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S00_AXIS_0_tlast : in STD_LOGIC;
    S00_AXIS_0_tvalid : in STD_LOGIC;
    S00_AXIS_0_tready : out STD_LOGIC;
    Clk : in STD_LOGIC;
    reset_rtl_0 : in STD_LOGIC
  );
  end component;
  

  
  component IOBUF is
  port (
    I : in STD_LOGIC;
    O : out STD_LOGIC;
    T : in STD_LOGIC;
    IO : inout STD_LOGIC
  );
  end component IOBUF;
  
  component lowlevel_dac_intfc is
    Port ( 
            rst          : in STD_LOGIC;
            clk125       : in STD_LOGIC;
            data_word    : in STD_LOGIC_VECTOR(31 downto 0);
            sdata        : out STD_LOGIC;
            lrck         : out STD_LOGIC;
            bclk         : out STD_LOGIC;
            mclk         : out STD_LOGIC;
            latched_data : out STD_LOGIC
          );
end component;

COMPONENT dds_compiler_0
  PORT (
    aclk                 : IN STD_LOGIC;
    aresetn              : IN STD_LOGIC;
    s_axis_config_tvalid : IN STD_LOGIC;
    s_axis_config_tdata  : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    m_axis_data_tvalid   : OUT STD_LOGIC;
    m_axis_data_tdata    : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
  );
END COMPONENT;

COMPONENT pulse_RE2X is Port 
    ( 
        CLK       : in STD_LOGIC;
        SIGNAL_IN : in STD_LOGIC;
        PULSE_OUT : out STD_LOGIC
    );
end COMPONENT;

COMPONENT ila_0 is
PORT (
	       clk    : IN STD_LOGIC;
	       probe0 : IN STD_LOGIC_VECTOR(0 DOWNTO 0); 
	       probe1 : IN STD_LOGIC_VECTOR(0 DOWNTO 0); 
	       probe2 : IN STD_LOGIC_VECTOR(31 DOWNTO 0); 
	       probe3 : IN STD_LOGIC_VECTOR(0 DOWNTO 0); 
	       probe4 : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
	       probe5 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
	       probe6 : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
	       probe7 : IN STD_LOGIC_VECTOR(15 DOWNTO 0) 
     );
END COMPONENT  ;

COMPONENT FIR1
  PORT (
    aclk               : IN STD_LOGIC;
    s_axis_data_tvalid : IN STD_LOGIC;
    s_axis_data_tready : OUT STD_LOGIC;
    s_axis_data_tdata  : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    m_axis_data_tvalid : OUT STD_LOGIC;
    m_axis_data_tdata  : OUT STD_LOGIC_VECTOR(79 DOWNTO 0)
  );
END COMPONENT;

COMPONENT FIR2
  PORT (
    aclk               : IN STD_LOGIC;
    s_axis_data_tvalid : IN STD_LOGIC;
    s_axis_data_tready : OUT STD_LOGIC;
    s_axis_data_tdata  : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    m_axis_data_tvalid : OUT STD_LOGIC;
    m_axis_data_tdata  : OUT STD_LOGIC_VECTOR(79 DOWNTO 0)
  );
END COMPONENT;

COMPONENT DDS_Tuner
  PORT (
    aclk                 : IN STD_LOGIC;
    aresetn              : IN STD_LOGIC;
    s_axis_config_tvalid : IN STD_LOGIC;
    s_axis_config_tdata  : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    m_axis_data_tvalid   : OUT STD_LOGIC;
    m_axis_data_tdata    : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

COMPONENT mult_gen_0
  PORT (
         CLK : IN STD_LOGIC;
           A : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
           B : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
           P : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;
-------------------------------------------------------------------
-- Signals
-------------------------------------------------------------------  
  signal iic_rtl_0_scl_i      : STD_LOGIC;
  signal iic_rtl_0_scl_o      : STD_LOGIC;
  signal iic_rtl_0_scl_t      : STD_LOGIC;
  signal iic_rtl_0_sda_i      : STD_LOGIC;
  signal iic_rtl_0_sda_o      : STD_LOGIC;
  signal iic_rtl_0_sda_t      : STD_LOGIC;
  signal reset                : STD_LOGIC;
  signal M_AXIS_0_tdata       : STD_LOGIC_VECTOR(31 downto 0);
  signal M_AXIS_0_tready      : STD_LOGIC;
  signal M_AXIS_0_tvalid      : STD_LOGIC;
  signal almost_empty_0       : STD_LOGIC;
  signal almost_full_0        : STD_LOGIC;
  signal gpio_rtl_0_tri_i     : STD_LOGIC_VECTOR(3 downto 0);
  signal gpio_rtl_1_tri_o     : STD_LOGIC_VECTOR(3 downto 0);
  signal axis_wr_data_count_0 : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal axis_rd_data_count_0 : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal sdata                : STD_LOGIC;
  signal lrck                 : STD_LOGIC;
  signal bclk                 : STD_LOGIC;
  signal mclk                 : STD_LOGIC;
  signal latched_data         : STD_LOGIC;
  
  -- dds
  signal M_AXIS_dds_tdata         : STD_LOGIC_VECTOR(15 downto 0);
  signal M_AXIS_dds_tvalid        : STD_LOGIC;
  signal DDS_config               : STD_LOGIC_VECTOR(31 downto 0);
  signal M_AXIS_DDS_config_tvalid : STD_LOGIC;
  signal M_AXIS_DDS_config_tdata  : STD_LOGIC_VECTOR(31 downto 0);
  signal write_to_file            : STD_LOGIC;
  
  signal M_AXIS_FIR1_tvalid       : STD_LOGIC;
  signal M_AXIS_FIR1_tdata        : STD_LOGIC_VECTOR(79 downto 0);
  signal M_AXIS_FIR2_tvalid       : STD_LOGIC;
  signal M_AXIS_FIR2_tdata        : STD_LOGIC_VECTOR(79 downto 0);
  signal DAC_data_left            : STD_LOGIC_VECTOR(15 downto 0);
  signal DAC_data_right           : STD_LOGIC_VECTOR(15 downto 0);
  
  signal Tuner                    : STD_LOGIC_VECTOR(31 downto 0);
  signal M_AXIS_Tuner_tvalid      : STD_LOGIC;
  signal M_AXIS_Tuner_tdata       : STD_LOGIC_VECTOR(31 downto 0);
  signal M_AXIS_Tuner_out_tvalid  : STD_LOGIC;
  signal cosine                   : STD_LOGIC_VECTOR(15 downto 0);
  signal sine                     : STD_LOGIC_VECTOR(15 downto 0);
  signal I                        : STD_LOGIC_VECTOR(31 downto 0);
  signal Q                        : STD_LOGIC_VECTOR(31 downto 0);
  
  -- Test signals
  signal dcnt            : unsigned(2 downto 0);
  signal data_word       : std_logic_vector(31 downto 0);
  
  
  signal axi_stream_data : std_logic_vector(31 downto 0);
  signal axi_stream_last : std_logic;
  signal axi_stream_valid: std_logic;
  signal axi_stream_ready: std_logic;
  
  signal sys_clk_125 : std_logic;
begin
reset <= not reset_pb;

clk : clk_wiz_0
   port map ( 
  -- Clock out ports  
   clk_out1 => sys_clk_125,
   -- Clock in ports
   clk_in1 => sysclk
 );

---------------------------------------------------------------
-- I2C Tri-state buffers
---------------------------------------------------------------
iic_rtl_0_scl_iobuf: component IOBUF
     port map (
      I => iic_rtl_0_scl_o,
      IO => ac_scl,
      O => iic_rtl_0_scl_i,
      T => iic_rtl_0_scl_t
    );
iic_rtl_0_sda_iobuf: component IOBUF
     port map (
      I => iic_rtl_0_sda_o,
      IO => ac_sda,
      O => iic_rtl_0_sda_i,
      T => iic_rtl_0_sda_t
    );

------------------------------------------------------------------------
-- Microblaze Microcontroller system
------------------------------------------------------------------------
proc_system_i: component proc_system1
     port map (
      Clk => sys_clk_125,
      DDR_addr(14 downto 0) => DDR_addr(14 downto 0),
      DDR_ba(2 downto 0) => DDR_ba(2 downto 0),
      DDR_cas_n => DDR_cas_n,
      DDR_ck_n => DDR_ck_n,
      DDR_ck_p => DDR_ck_p,
      DDR_cke => DDR_cke,
      DDR_cs_n => DDR_cs_n,
      DDR_dm(3 downto 0) => DDR_dm(3 downto 0),
      DDR_dq(31 downto 0) => DDR_dq(31 downto 0),
      DDR_dqs_n(3 downto 0) => DDR_dqs_n(3 downto 0),
      DDR_dqs_p(3 downto 0) => DDR_dqs_p(3 downto 0),
      DDR_odt => DDR_odt,
      DDR_ras_n => DDR_ras_n,
      DDR_reset_n => DDR_reset_n,
      DDR_we_n => DDR_we_n,
      DDS_config_tri_o(31 downto 0) => DDS_Config,
      FIXED_IO_ddr_vrn => FIXED_IO_ddr_vrn,
      FIXED_IO_ddr_vrp => FIXED_IO_ddr_vrp,
      FIXED_IO_mio(53 downto 0) => FIXED_IO_mio(53 downto 0),
      FIXED_IO_ps_clk => FIXED_IO_ps_clk,
      FIXED_IO_ps_porb => FIXED_IO_ps_porb,
      FIXED_IO_ps_srstb => FIXED_IO_ps_srstb,
      S00_AXIS_0_tdata(31 downto 0) => axi_stream_data,
      S00_AXIS_0_tlast => axi_stream_last,
      S00_AXIS_0_tready => axi_stream_ready,
      S00_AXIS_0_tstrb(3 downto 0) => (others => '1'),
      S00_AXIS_0_tvalid => axi_stream_valid,
      Tuner_tri_o(31 downto 0) => Tuner,
      gpio_rtl_0_tri_i(3 downto 0) => gpio_rtl_0_tri_i(3 downto 0),
      gpio_rtl_1_tri_o(3 downto 0) => gpio_rtl_1_tri_o(3 downto 0),
      iic_rtl_0_scl_i => iic_rtl_0_scl_i,
      iic_rtl_0_scl_o => iic_rtl_0_scl_o,
      iic_rtl_0_scl_t => iic_rtl_0_scl_t,
      iic_rtl_0_sda_i => iic_rtl_0_sda_i,
      iic_rtl_0_sda_o => iic_rtl_0_sda_o,
      iic_rtl_0_sda_t => iic_rtl_0_sda_t,
      reset_rtl_0 => reset
    );


    
------------------------------------------------------------------
-- DDS
------------------------------------------------------------------

PRE0:   pulse_RE2X Port map
    ( 
        CLK       => sys_clk_125,
        SIGNAL_IN => DDS_Config(31),          -- DDS_config(31) __________|----------------
        PULSE_OUT => M_AXIS_DDS_config_tvalid -- tvalid         ____________|-|____________
    );
    
M_AXIS_DDS_config_tdata <= '0' & DDS_Config(30 downto 0);

DDS : dds_compiler_0
  PORT MAP (
    aclk                 => sys_clk_125,
    aresetn              => gpio_rtl_1_tri_o(1),
    s_axis_config_tvalid => M_AXIS_DDS_config_tvalid,
    s_axis_config_tdata  => M_AXIS_DDS_config_tdata,
    m_axis_data_tvalid   => M_AXIS_dds_tvalid, 
    m_axis_data_tdata    => M_AXIS_dds_tdata
  );
  
------------------------------------------------------------------
-- Tuner
------------------------------------------------------------------
PRE1:   pulse_RE2X Port map
    ( 
        CLK       => sys_clk_125,
        SIGNAL_IN => Tuner(31),               -- Tuner(31) __________|----------------
        PULSE_OUT => M_AXIS_Tuner_tvalid -- tvalid         ____________|-|____________
    );
    
M_AXIS_Tuner_tdata <= '0' & Tuner(30 downto 0);


TNR : DDS_Tuner
  PORT MAP (
    aclk                            => sys_clk_125,
    aresetn                         => gpio_rtl_1_tri_o(1),
    s_axis_config_tvalid            => M_AXIS_Tuner_tvalid,
    s_axis_config_tdata             => M_AXIS_Tuner_tdata,
    m_axis_data_tvalid              => M_AXIS_Tuner_out_tvalid,
    m_axis_data_tdata(31 downto 16) => sine,
    m_axis_data_tdata(15 downto 0)  => cosine
    
  );
  
 -----------------------------------------------------------------
 -- Mixer
 -----------------------------------------------------------------
 IMIX : mult_gen_0
  PORT MAP (
                CLK => sys_clk_125,
                A   => cosine,
                B   => M_AXIS_dds_tdata,
                P   => I
           );
           
QMIX : mult_gen_0
  PORT MAP (
                CLK => sys_clk_125,
                A   => sine,
                B   => M_AXIS_dds_tdata,
                P   => Q
           );
  
------------------------------------------------------------------
-- FIR Filter chain
------------------------------------------------------------------
FLT1 : FIR1
  PORT MAP (
    aclk                             => sys_clk_125,
    s_axis_data_tvalid               => M_AXIS_dds_tvalid,
    s_axis_data_tready               => open,
    s_axis_data_tdata(31 downto 16)  => Q(29 downto 14), --sine,   --M_AXIS_dds_tdata,
    s_axis_data_tdata(15 downto 0)   => I(29 downto 14), --cosine, --M_AXIS_dds_tdata,
    m_axis_data_tvalid               => M_AXIS_FIR1_tvalid,
    m_axis_data_tdata                => M_AXIS_FIR1_tdata
  );
  
FLT2 : FIR2
  PORT MAP (
    aclk                            => sys_clk_125,
    s_axis_data_tvalid              => M_AXIS_FIR1_tvalid,
    s_axis_data_tready              => open,
    s_axis_data_tdata(31 downto 16) => M_AXIS_FIR1_tdata(77 downto 62),
    s_axis_data_tdata(15 downto 0)  => M_AXIS_FIR1_tdata(37 downto 22),
    m_axis_data_tvalid              => M_AXIS_FIR2_tvalid,
    m_axis_data_tdata               => M_AXIS_FIR2_tdata
  );
  
-----------------------------------------------------------------
-- Filter bypass switch
-----------------------------------------------------------------
DAC_data_right <= M_AXIS_FIR2_tdata(77 downto 62) when sw(0) = '1' else M_AXIS_dds_tdata;
DAC_data_left  <= M_AXIS_FIR2_tdata(37 downto 22) when sw(0) = '1' else M_AXIS_dds_tdata;


--custom_peripheral connections
axi_stream_data <= DAC_data_right & DAC_data_left;
axi_stream_last <= '0'; 
axi_stream_valid <= M_AXIS_FIR2_tvalid when sw(0) = '1' else M_AXIS_dds_tvalid;
------------------------------------------------------------------
-- Audio Codec Interface 
------------------------------------------------------------------ 
DACIF:    lowlevel_dac_intfc Port map
      ( 
            rst                     => reset_PB,
            clk125                  => sys_clk_125,
            data_word(31 downto 16) => DAC_data_right,
            data_word(15 downto 0)  => DAC_data_left,
            sdata                   => sdata,
            lrck                    => lrck,
            bclk                    => bclk,
            mclk                    => mclk,
            latched_data            => latched_data      -- not used
      );

-- Outputs to CODEC on Zybo Board
ac_pbdat         <= sdata;     
ac_pblrc         <= not lrck;            -- added to make consistent with datasheet.     
ac_bclk          <= bclk;
ac_mclk          <= mclk;
M_AXIS_0_tready  <= latched_data; 
ac_reclrc        <= '0';                 -- record clock disabled   
ac_muten         <= gpio_rtl_1_tri_o(0); -- Mute is active low.

-- LEDS
led(0)           <= gpio_rtl_1_tri_o(0); -- DAC Mute LED: '1' = mute off, '0' = mute on
led(1)           <= gpio_rtl_1_tri_o(1); -- DDS reset: '0' = reset, '1' = active.
led(2)           <= '0';
led(3)           <= '0'; 
------------------------------------------------------------------
-- ILA
------------------------------------------------------------------
write_to_file <= M_AXIS_dds_tvalid and gpio_rtl_1_tri_o(2);

ILA : ila_0
PORT MAP (
	       clk       => sys_clk_125,
	       probe0(0) => gpio_rtl_1_tri_o(1),      -- 1  bit
	       probe1(0) => M_AXIS_DDS_config_tvalid, -- 1  bit 
	       probe2    => M_AXIS_DDS_config_tdata,  -- 32 bits
	       probe3(0) => M_AXIS_dds_tvalid,        -- 1  bit
	       probe4    => M_AXIS_dds_tdata,         -- 16 bits 
	       probe5(0) => write_to_file,            -- 1  bit
	       probe6    => DAC_data_right,           -- 16 bits
	       probe7    => DAC_data_left             -- 16 bits
          );

------------------------------------------------------------------
-- Test Signal
------------------------------------------------------------------
--AD:   process(sysclk, reset_PB)
--            begin
--                if reset_PB = '1' then
--                    dcnt <= "000";
--                elsif rising_edge(sysclk) then
--                    if latched_data = '1' then
--                        dcnt <= dcnt + 1;
--                    end if;
--                end if;
--        end process;
        
-- data_word <= x"00000000" when dcnt = "000" else
--              x"1b9e1b9e" when dcnt = "001" else
--              x"27102710" when dcnt = "010" else
--              x"1b9e1b9e" when dcnt = "011" else
--              x"00000000" when dcnt = "100" else
--              x"e462e462" when dcnt = "101" else
--              x"d8f0d8f0" when dcnt = "110" else
--              x"e462e462";
-- data_word <= x"00000000";

end Behavioral;
