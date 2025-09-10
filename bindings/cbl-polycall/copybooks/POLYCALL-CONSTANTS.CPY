      * CBLPolyCall Constants Copybook v1.0
      * OBINexus Aegis Engineering - PolyCall Constants
      
       01  POLYCALL-CONSTANTS.
           05  PC-SUCCESS              PIC 9(4) COMP VALUE 0.
           05  PC-ERROR-GENERAL        PIC 9(4) COMP VALUE 1.
           05  PC-ERROR-CONNECTION     PIC 9(4) COMP VALUE 100.
           05  PC-ERROR-PROTOCOL       PIC 9(4) COMP VALUE 200.
           05  PC-ERROR-TIMEOUT        PIC 9(4) COMP VALUE 300.
           05  PC-ERROR-AUTHENTICATION PIC 9(4) COMP VALUE 400.
           
       01  POLYCALL-LIMITS.
           05  PC-MAX-MESSAGE-SIZE     PIC 9(8) COMP VALUE 4096.
           05  PC-MAX-CONNECTIONS      PIC 9(4) COMP VALUE 100.
           05  PC-TIMEOUT-SECONDS      PIC 9(4) COMP VALUE 30.
