#constant SERIAL_BUFFER_SIZE    128
#constant MSG_BUFFER_SIZE       32

var serial_buffer_ptr := 0;
var serial_buffer[SERIAL_BUFFER_SIZE];

func getStringPtr()
    return serial_buffer + serial_buffer_ptr - 1;
endfunc

func hasNextChar()
    if (com_Count() > 1) // ignore checksum (last byte)
        return TRUE;
    endif
    return FALSE;
endfunc

func hasChars()
    return com_Count() - 1; // ignore checksum
endfunc

func getNextChar()
    serial_buffer_ptr++;
    return serin();
endfunc

func verifySerialBufferContents()
    var bytes;

    bytes := com_Count();
    if (bytes <= 0)
        setError("Zero package");
        return FALSE;
    endif

    if (com_Checksum())
        txt_MoveCursor(6, 0);
        setError("Checksum failed");
        return FALSE;
    endif

    return TRUE;
endfunc

func resetSerialBuffer()
    serial_buffer_ptr := 0;
    com_Init(serial_buffer, 0, '$');
endfunc

func sendAck()
    to(COM0); print("OK\n");
endfunc

func sendNAck(var errMsg)
    to(COM0); print("ERR:", [STR] errMsg, "\n");
endfunc

func initCommunication(var baudrate)
    resetSerialBuffer();
    if (baudrate == 0)
        repeat
            baudrate := com_AutoBaud(500);
            txt_MoveCursor(2, 0);
            print("Waiting 'U'...");
        until (baudrate);
        print("\nOk");
        pause(500);
        gfx_Cls();
    else
        setbaud(baudrate);
    endif
    sendAck();
endfunc
