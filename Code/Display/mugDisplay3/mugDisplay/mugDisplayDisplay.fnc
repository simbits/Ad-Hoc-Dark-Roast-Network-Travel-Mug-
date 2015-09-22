#constant scroll_blit_sector       $ 0x0020, 0x0000       /* offset 0xC400 */
#constant selected_message_sector  $ 0x0010, 0x0000       /* offset selected message */

#CONST
    X := 0,
    Y,
    WIDTH,
    HEIGHT,
    FG_COLOUR,
    BG_COLOUR,
    TEXT_LINE,
    TEXT_COL,
    TEXT_TYPE_BOLD,
    TEXT_WRAP
#END

var selection_area_properties[] := [0, 118, 127, 9, GREEN, BLACK, 15, 0, 1, 0];
var message_window_properties[] := [0, 24, 127, 90, BLUE, BLACK, 13, 0, 0, 1];
var name_area_properties[] := [0, 0, 100, 9, GREEN, BLACK, 1, 0, 0, 0];
var power_icon_properties[] := [106, 2, ICON_WIDTH, ICON_HEIGHT, 0, BLACK];
var signal_icon_properties[] := [62, 2, ICON_WIDTH, ICON_HEIGHT, 0, BLACK];
var transmission_icon_properties[] := [84, 2, ICON_WIDTH, ICON_HEIGHT, 0, BLACK];

var selected_message_buffer[MSG_BUFFER_SIZE];
var selected_message_length;
var selected_message;
var selected_message_offset;

func displayIconAtPosition(var sectHi, var sectLo, var x, var y, var frame)
    media_SetSector(sectHi, sectLo);
    media_VideoFrame(x, y, frame);
endfunc

func clearArea(var x, var y, var w, var h, var fg_colour, var bg_colour)
    var pixels;

    gfx_Clipping(ON);
    gfx_ClipWindow(x, y, x+w, y+h);

    pixels := gfx_FocusWindow();

    disp_BlitPixelFill(bg_colour, pixels);
    gfx_Clipping(OFF);
endfunc
/*
func text_wrap(var str, var bfr, var offset)
    var txtStr, i, chars, width;
    var lastDelimPos;
    var foundMoreChars := FALSE;

    txtStr := str + offset;
    chars := strlen(str);
    width := 0;
    lastDelimPos := 0;

    for (i:=0; i<9; i++, offset++)

        if (txtStr[i] == ',')
            lastDelimPos := i;
        if (txtStr[i] == ' ')
            lastDelimPos := i;
        if (txtStr[i] == '-')
            lastDelimPos := i;
        if (txtStr[i] == '_')
           lastDelimPos := i;

        if (chars == offset)
            offset := 0;
            break;
        endif
        bfr[i] := txtStr[i];
    next

    bfr[i] := 0x00;

    if (offset != 0)
        for (i:=offset; i<strlen(str); i++)
            var c1, c2;
            c1 := str[i] & 0x00ff;
            c2 := (str[i] & 0xff00) >> 8;
            if (c1 != ' ' || c1 != '\n' || c1 != '\t' || c1 != '\0')
                foundMoreChars := TRUE;
                break;
            endif
            if (c2 != ' ' || c2 != '\n' || c2 != '\t' || c2 != '\0')
                foundMoreChars := TRUE;
                break;
            endif
            offset++;
        next
    endif

    if (foundMoreChars == FALSE)
        offset := 0;
    endif

    return offset;

endfunc
*/

func text_wrap(var str, var bfr, var offset)
    var txtStr, i, chars, width;
    var offsetCntr;
    var bfrPos;
    var lastDelimPos;
    var foundMoreChars := TRUE;
    var foundStart := FALSE;
    var twist := FALSE;
    var strStart, strEnd;

    txtStr := str;
    chars := strlen(str);
    width := 0;
    lastDelimPos := 0;
    bfrPos:=0;
    offsetCntr:=0;

    strStart := offset;
    strEnd := 0;

    for (i:=0; i<chars; i++)
        var c;

        c := txtStr[i] & 0x00ff;

        if (offsetCntr >= offset)
            if (!foundStart)
                 foundStart := TRUE;
            endif

            if (c == '\n')
                offsetCntr++;
                break;
            else if ( c == '\0')
                foundMoreChars := FALSE;
                break;
            else
                if (!twist)
                    bfr[bfrPos] := c;
                else
                    bfr[bfrPos] |= (c << 8);
                    bfrPos++;
                endif
            endif
        endif

        offsetCntr++;
        c := (txtStr[i] & 0xff00) >> 8;

        if (offsetCntr >= offset)
            if (!foundStart)
                 foundStart := TRUE;
                 twist := TRUE;
            endif
            if (c == '\n')
                offsetCntr++;
                break;
            else if ( c == '\0')
                foundMoreChars := FALSE;
                break;
            else
                if (!twist)
                    bfr[bfrPos] |= (c << 8);
                    bfrPos++;
                else
                    bfr[bfrPos] := c;
                endif
            endif
        endif

        offsetCntr++;
    next

    if (foundMoreChars == FALSE)
        return 0;
    endif
    return offsetCntr;

endfunc

func print_message(var str, var area)
    var i, len;
    var found_delim;
    var found_name;

    found_delim := FALSE;
    found_name := FALSE;

    len := (strlen(str)+1) / 2;
    for (i:=0; i<len; i++)
        var c1, c2;

        c1 := str[i] & 0x00ff;
        c2 := (str[i] & 0xff00) >> 8;

        if (found_delim == FALSE)
            if (c2 == '/')
                found_delim := TRUE;
                found_name := TRUE;
                txt_FGcolour(RED);
                txt_Bold(1);
                continue;
            else if (c2 == '*')
                found_delim := TRUE;
                found_name := TRUE;
                txt_FGcolour(YELLOW);
                txt_Bold(1);
                continue;
            else if (c2 == '^')
                found_delim := TRUE;
                found_name := TRUE;
                txt_FGcolour(PURPLE);
                txt_Bold(1);
                continue;
            endif
        endif

        if (found_name && c1 == ' ')
            txt_FGcolour(area[FG_COLOUR]);
            txt_Bold(area[TEXT_TYPE_BOLD]);
            found_name := FALSE;
        endif

        if (i > 0 || c1 != ' ')
            putch(c1);
        endif

        if (found_name && c2 == ' ')
            txt_FGcolour(area[FG_COLOUR]);
            txt_Bold(area[TEXT_TYPE_BOLD]);
            found_name := FALSE;
        endif

        putch(c2);
    next
endfunc

func display_text_in_area(var str, var area)
    var lines;
    var c1, c2;

    clearArea(@area);

    gfx_Clipping(ON);
    gfx_ClipWindow(area[X],
                   area[Y],
                   area[X] + area[WIDTH],
                   area[Y] + area[HEIGHT]);

    txt_MoveCursor(area[TEXT_LINE], area[TEXT_COL]);
    txt_Bold(area[TEXT_TYPE_BOLD]);
    txt_FGcolour(area[FG_COLOUR]);
    txt_BGcolour(area[BG_COLOUR]);

    print_message(str, area);
    lines := 1;

    gfx_Clipping(OFF);

    return lines;
endfunc

func scroll_text_in_selection_area()
    var private delay := 5;
    var len;

    len := strlen(selected_message);

    if (len < 17)
        return;
    endif

    if (delay == 0)
        if (++selected_message_offset == len-1)
            selected_message_offset := 0;
            delay := 5;
        endif

        if (selected_message_offset <= (strlen(selected_message) + 1) / 2)
            display_text_in_area(selected_message+selected_message_offset, selection_area_properties);
        endif
    else
        delay--;
    endif
endfunc

func display_selection_text(var str, var offset)
    selected_message := str;
    selected_message_offset := offset;

    if (selected_message_offset <= (strlen(selected_message) / 2) + 1)
        display_text_in_area(selected_message+selected_message_offset, selection_area_properties);
    endif
endfunc

func display_selection_text_copy(var str, var offset)
    var len, i;

    len := (strlen(str) + 1) / 2;

    for (i:=0; i<MSG_BUFFER_SIZE; i++)
        if (i < len)
            selected_message_buffer[i] := str[i];
        else
            selected_message_buffer[i] := 0x00;
        endif
    next

    display_selection_text(selected_message_buffer, offset);
endfunc

func _write_str_to_window(var msg, var area)
    var pixels;
    var width, height;

    if (msg[0] == 0x00)
        return;
    endif

    width := strwidth(msg);
    height := strheight() + 3;

    gfx_Clipping(ON);
    gfx_ClipWindow(area[X],
                   area[Y] + height,
                   area[X] + area[WIDTH],
                   area[Y] + area[HEIGHT]);

    gfx_FocusWindow();
    media_SetSector(scroll_blit_sector);
    pixels := disp_BlitPixelsToMedia();

    display_text_in_area(msg, area);

    gfx_ClipWindow(area[X],
                   area[Y],
                   area[X] + area[WIDTH],
                   area[Y] + area[HEIGHT] - height);

    pixels := gfx_FocusWindow();
    media_SetSector(scroll_blit_sector);
    disp_BlitPixelsFromMedia(pixels);
    gfx_Clipping(OFF);
endfunc

func write_str_to_window(var str, var area)
    var lines;

    if (area[TEXT_WRAP])
        var offset;

        //offset := text_wrap(str, bfr, offset);

        repeat
            var bfr[11];
            var i;

            for (i:=0; i<11; i++)
                bfr[i] := 0;
            next

            offset := text_wrap(str, bfr, offset);
            _write_str_to_window(bfr, area);
            lines++;
        until (offset == 0);

    else
        lines := _write_str_to_window(str, area);
    endif

    return lines;
endfunc

func write_message(var str)
    write_str_to_window(str, message_window_properties);
endfunc

func write_selected_message()
    write_str_to_window(selected_message, message_window_properties);
    //clearArea(@selection_area_properties);
endfunc

func write_name(var str)
    txt_FGcolour(GREEN);
    txt_BGcolour(BLACK);
    txt_Bold(0);
    txt_Height(1);
    txt_Width(1);
    txt_MoveCursor(1, 0);
    print([STR]str);
endfunc

func clear_message_area()
    clearArea(@message_window_properties);
endfunc

func initScreenLayout()
    gfx_Hline(3+ICON_HEIGHT, 0, 127, GREEN);
    gfx_Hline(116, 0, 127, GREEN);
    clearArea(@message_window_properties);
    clearArea(@selection_area_properties);
endfunc
