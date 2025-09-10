      * CBLPolyCall Structures Copybook v1.0
      * OBINexus Aegis Engineering - PolyCall Data Structures
      
       01  POLYCALL-MESSAGE.
           05  PC-MSG-HEADER.
               10  PC-MSG-TYPE         PIC X(4).
               10  PC-MSG-LENGTH       PIC 9(8) COMP.
               10  PC-MSG-SEQUENCE     PIC 9(8) COMP.
               10  PC-MSG-TIMESTAMP    PIC 9(18) COMP.
           05  PC-MSG-BODY             PIC X(4092).
           
       01  POLYCALL-CONNECTION.
           05  PC-CONN-ID              PIC 9(8) COMP.
           05  PC-CONN-HOST            PIC X(256).
           05  PC-CONN-PORT            PIC 9(5) COMP.
           05  PC-CONN-STATUS          PIC X(20).
           05  PC-CONN-PROTOCOL        PIC X(10).
