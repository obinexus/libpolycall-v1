      * CBLPolyCall FFI Bridge v1.0
      * OBINexus Aegis Engineering - COBOL to PolyCall Interface
      * Technical Lead: Nnamdi Michael Okpala
      
       IDENTIFICATION DIVISION.
       PROGRAM-ID. POLYCALL-BRIDGE.
       
       ENVIRONMENT DIVISION.
       CONFIGURATION SECTION.
       SPECIAL-NAMES.
           CALL-CONVENTION 0 IS C-CALLING.
       
       DATA DIVISION.
       WORKING-STORAGE SECTION.
       01  WS-POLYCALL-VERSION     PIC X(10) VALUE "1.0.0".
       01  WS-BRIDGE-STATUS        PIC 9(4) COMP.
       01  WS-ERROR-MESSAGE        PIC X(256).
       01  WS-PROTOCOL-BUFFER      PIC X(4096).
       
       LINKAGE SECTION.
       01  LNK-OPERATION           PIC X(20).
       01  LNK-INPUT-DATA          PIC X(1024).
       01  LNK-OUTPUT-DATA         PIC X(1024).
       01  LNK-RESULT-CODE         PIC 9(4) COMP.
       
       PROCEDURE DIVISION USING LNK-OPERATION,
                               LNK-INPUT-DATA,
                               LNK-OUTPUT-DATA,
                               LNK-RESULT-CODE.
       
       MAIN-LOGIC.
           PERFORM INITIALIZE-BRIDGE
           
           EVALUATE LNK-OPERATION
               WHEN "CONNECT"
                   PERFORM POLYCALL-CONNECT
               WHEN "SEND_MESSAGE"
                   PERFORM POLYCALL-SEND
               WHEN "RECEIVE_MESSAGE"
                   PERFORM POLYCALL-RECEIVE
               WHEN "DISCONNECT"
                   PERFORM POLYCALL-DISCONNECT
               WHEN OTHER
                   MOVE 999 TO LNK-RESULT-CODE
                   MOVE "UNKNOWN_OPERATION" TO LNK-OUTPUT-DATA
           END-EVALUATE
           
           EXIT PROGRAM.
       
       INITIALIZE-BRIDGE.
           MOVE ZERO TO WS-BRIDGE-STATUS
           MOVE SPACES TO WS-ERROR-MESSAGE
           MOVE SPACES TO WS-PROTOCOL-BUFFER.
       
       POLYCALL-CONNECT.
      *    Call external PolyCall library function
           CALL "polycall_connect" USING BY REFERENCE LNK-INPUT-DATA
                                        BY REFERENCE WS-BRIDGE-STATUS
           
           IF WS-BRIDGE-STATUS = ZERO
               MOVE "CONNECTED" TO LNK-OUTPUT-DATA
               MOVE ZERO TO LNK-RESULT-CODE
           ELSE
               MOVE "CONNECTION_FAILED" TO LNK-OUTPUT-DATA
               MOVE WS-BRIDGE-STATUS TO LNK-RESULT-CODE
           END-IF.
       
       POLYCALL-SEND.
      *    Call external PolyCall send function
           CALL "polycall_send" USING BY REFERENCE LNK-INPUT-DATA
                                     BY REFERENCE WS-PROTOCOL-BUFFER
                                     BY REFERENCE WS-BRIDGE-STATUS
           
           IF WS-BRIDGE-STATUS = ZERO
               MOVE "MESSAGE_SENT" TO LNK-OUTPUT-DATA
               MOVE ZERO TO LNK-RESULT-CODE
           ELSE
               MOVE "SEND_FAILED" TO LNK-OUTPUT-DATA
               MOVE WS-BRIDGE-STATUS TO LNK-RESULT-CODE
           END-IF.
       
       POLYCALL-RECEIVE.
      *    Call external PolyCall receive function
           CALL "polycall_receive" USING BY REFERENCE WS-PROTOCOL-BUFFER
                                        BY REFERENCE LNK-OUTPUT-DATA
                                        BY REFERENCE WS-BRIDGE-STATUS
           
           IF WS-BRIDGE-STATUS = ZERO
               MOVE ZERO TO LNK-RESULT-CODE
           ELSE
               MOVE "RECEIVE_FAILED" TO LNK-OUTPUT-DATA
               MOVE WS-BRIDGE-STATUS TO LNK-RESULT-CODE
           END-IF.
       
       POLYCALL-DISCONNECT.
      *    Call external PolyCall disconnect function
           CALL "polycall_disconnect" USING BY REFERENCE WS-BRIDGE-STATUS
           
           IF WS-BRIDGE-STATUS = ZERO
               MOVE "DISCONNECTED" TO LNK-OUTPUT-DATA
               MOVE ZERO TO LNK-RESULT-CODE
           ELSE
               MOVE "DISCONNECT_FAILED" TO LNK-OUTPUT-DATA
               MOVE WS-BRIDGE-STATUS TO LNK-RESULT-CODE
           END-IF.