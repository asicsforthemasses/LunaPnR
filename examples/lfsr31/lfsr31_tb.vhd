
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

ENTITY LFSR31_TB IS
END LFSR31_TB;

ARCHITECTURE tb OF LFSR31_TB IS

    SIGNAL clk, rst_an    : std_logic;
    SIGNAL data           : std_logic;
    SIGNAL quit           : std_logic := '0';
BEGIN

    u_DUT: ENTITY work.LFSR31(rtl) 
    PORT MAP
    (
        clk => clk,
        rst_an => rst_an, 
        dout => data
    );
   
    proc_clk: PROCESS
    BEGIN
        clk <= '1';
        WAIT FOR 10 ns;
        clk <= '0';
        WAIT FOR 10 ns;
        IF (quit = '1') THEN
            WAIT;
        END IF;
    END PROCESS proc_clk;

    proc_sim: PROCESS
    BEGIN
        rst_an <= '0';
        WAIT FOR 20 ns;
        rst_an <= '1';
        WAIT FOR 10000 ns;
        quit <= '1';
        WAIT;
    END PROCESS proc_sim;


END tb;
