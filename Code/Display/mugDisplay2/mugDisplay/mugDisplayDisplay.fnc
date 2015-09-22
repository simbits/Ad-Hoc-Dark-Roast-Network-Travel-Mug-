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

func displaySplashScreen(var sectHiWord, var sectLoWord, var x, var y)
    var n := 0;

    gfx_Contrast(1);

    media_SetSector(sectHiWord, sectLoWord);
    media_Image(x, y);

    while(n++ < 16)
        gfx_Contrast(n);                                // brighten up count 1 to 16
        pause(50);
    wend

    pause(SPLASH_SCREEN_DELAY);

    while(--n)
        gfx_Contrast(n);
        pause(80);
    wend

    gfx_Cls();
    gfx_Contrast(16);
endfunc

func text_wrap(var str, var bfr, var offset)
    var txtStr, i, chars, width;

    txtStr := str + offset;
    chars := strlen(str);
    width := 0;

    for (i:=0; i<10; i++, offset++)
        if (chars == offset)
            offset := 0;
            break;
        endif
        bfr[i] := txtStr[i];
    next

    bfr[i] := 0x00;

    return offset;

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

        putch(c1);

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
        var bfr[11];

        offset := text_wrap(str, bfr, offset);

        repeat
            _write_str_to_window(bfr, area);
            offset := text_wrap(str, bfr, offset);
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

func initScreenLayout()
    gfx_Hline(3+ICON_HEIGHT, 0, 127, GREEN);
    gfx_Hline(116, 0, 127, GREEN);
    clearArea(@message_window_properties);
    clearArea(@selection_area_properties);

    txt_Height(5);
    txt_Width(4);
    txt_MoveCursor(1, 0);
    print(" MUG");
    txt_Height(1);
    txt_Width(1);
endfunc
