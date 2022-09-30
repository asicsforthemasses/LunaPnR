LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

ENTITY LFSR31 IS
    PORT(
        clk     : IN std_logic; 
        rst_an  : IN std_logic;
        dout    : OUT std_logic
    );
END LFSR31;

ARCHITECTURE rtl OF LFSR31 IS
  SIGNAL curState, nextState: std_logic_vector (30 DOWNTO 0);
  SIGNAL feedback: std_logic;
BEGIN

    proc_clk: PROCESS(clk, rst_an)
    BEGIN
        IF (rst_an = '0') THEN
            curState <= (OTHERS =>'0');
            curState(0) <= '1';
        ELSIF rising_edge(clk) THEN
            curState <= nextState;
        END IF;
    END PROCESS proc_clk;
  
    -- Polynomial 31, 28
    feedback <= curState(30) XNOR curState(27);
    nextState <= curstate(29 DOWNTO 0) & feedback;
    dout <= curState(0);

END rtl;
