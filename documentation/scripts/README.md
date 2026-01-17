# Scripting documentation



### "Hello world" example
```perl
use strict;
use warnings;

#!REGISTER_EVENT
sub event_key_input {
    print("Key = $_[0], modifiers = $_[1]\n");
}

#!REGISTER_EVENT
sub event_help_message {
    nemi::create_msg("Hello! this is my script's help message.");
}

sub init_script() {
    print("My script was loaded!\n");  # Prints to stdout

    # Global variables should be initialized here...
}
```
-----------------------------
### Keybinds example
```perl
use strict;
use warnings;

our $script_name;

#!REGISTER_EVENT
sub event_keybind_press {
    nemi::create_msg("$script_name got keybind event $_[0]");
}

sub init_script() {
    $script_name = "keybind_example";
    nemi::add_keybind($script_name, "example_name", "lctrl + a");
    nemi::add_keybind($script_name, "list_files", "lshift + lctrl + l");
}

# Notes:
#  keybinds expect US layout but you can
#  lookup which key corresponds to your keyboard.
#
# There are currently few bugs with parsing the keybind's keys,
# try to swap the order if you encounter problems. I will fix the bug later.
```
-----------------------------
### Rendering from script example
```perl
use strict;
use warnings;

#!REGISTER_EVENT
sub event_render {
    nemi::draw_rect(100, 100, 50, 50, 0x00FFAA);
}

sub init_script() {
}

# See currently available rendering functions below.
```
-----------------------------

### Loading scripts

You can name your script what ever you want. To load the script you have to add it into 
`configs/scripts.ini` file. Example:
```
[SCRIPTS]
my_script = ./scripts/my_script.pl
```

-----------------------------

## Available events

The reason why `#!REGISTER_EVENT` exists is because that way we can keep track of
what events the script actually wants so it doesnt need to implement all event functions just to not use them at all.

```
Function                     Arguments
--------                     ---------
event_help_message           < No arguments >        // When this event is called user wants a help message for the script.
event_key_input              $_[0]: Key, $_[1]: Key modifier
event_char_input             $_[0]: Character
event_win_resized            $_[0]: New window width, $_[1]: New window height
event_term_buffer_changed    $_[0]: If the current buffer is altbuffer it is 1 otherwise 0.
event_keybind_press          $_[0]: Keybind event name.
event_render                 < No arguments >
```


## Available functions
```

////  Terminal control  ////

nemi::term_get_rows                < No arguments >
nemi::term_get_cols                < No arguments >
nemi::term_ignore_chars            < No arguments >    // Terminals will ignore character inputs.
nemi::term_ignore_keys             < No arguments >    // Terminals will ignore key inputs.
nemi::term_unignore_chars          < No arguments >
nemi::term_unignore_keys           < No arguments >
nemi::term_hide_cells              (col, row, width, height) // Parts of the rendered cells can be
nemi::term_show_cells              (col, row, width, height) // hidden to make room for your own stuff
nemi::term_scroll_y                (offset)
nemi::term_get_char                (column, row)
nemi::term_get_cursor_x            < No arguments >
nemi::term_get_cursor_y            < No arguments >
nemi::term_set_cell_custom_fg      (column, row, hex_rgb_color)
nemi::term_set_cell_custom_bg      (column, row, hex_rgb_color)
nemi::term_set_cell_custom_attrs   (column, row, attrs) // 0x2: Underline, 0x4: Italic, 0x8: Blink
nemi::term_clear_cell_custom_fg    (column, row)
nemi::term_clear_cell_custom_bg    (column, row)
nemi::term_clear_cell_custom_attrs (column, row)
nemi::term_copy_to_clipboard       (start_column, start_row, end_column, end_row) 
nemi::term_get_yscroll_offset      < No arguments >


////  Rendering  ////

nemi::draw_rect           (x, y, width, height, hexrgb_color)
nemi::draw_text           (x, y, text, hexrgb_color)
nemi::draw_rect_cells     (column, row, width_columns, height_rows, hexrgb_color)
nemi::draw_text_cells     (column, row, text, hexrgb_color)


////  Misc  ////

nemi::keydown              (GLFW Key number)
nemi::create_msg           (text)
nemi::add_keybind          (script_name, event_name, keybind_str)
nemi::recompile            < No arguments >  // Only works if 'source_dir' in config is set, and loader supports it.
nemi::script_keybinds      (script_name)     // Write script's keybinds to message terminal.
nemi::help                 (str)             // Can be used to get help message for specific thing.

```

