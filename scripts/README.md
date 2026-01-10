# Scripting documentation.


### Simple "Hello world" example.
```perl
use strict;
use warnings;

#!REGISTER_EVENT
sub event_key_input {
    print("Key = $_[0], modifiers = $_[1]\n");
}

sub init_script() {
    print("My script was loaded!\n");  # Prints to stdout

    # Global variables should be initialized here...
}
```

You can name your script what ever you want. To load the script you have to add it into 
`configs/scripts.ini` file.
 Example:
```
[SCRIPTS]
my_script = ./scripts/my_script.pl
^
 `-- This is the script's name so calling Nemi::help("my_script") makes more sense :)
```

Above perl code is going to tell the terminal emulator to call `event_key_input` for the script. 
The reason why `#!REGISTER_EVENT` exists is because that way we can keep track of
what events the script actually wants so it doesnt need to implement all event functions just to not use them at all.


Everytime a key is pressed, second argument is key modifier: Ctrl, Shift or Alt... or combination of them.
You can read https://www.glfw.org/docs/latest/group__keys.html for key codes
and https://www.glfw.org/docs/latest/group__mods.html for key modifiers.


## Available events.

```

Function                     Arguments
----------------------------------------------------
event_key_input ............ 0: key, 1: key modifier
event_char_input ........... 0: character
event_win_resized .......... 0: new window width, 1: new window height
event_term_buffer_changed .. 0: If the current buffer is altbuffer it is 1 otherwise 0.


... More will be added later ...

```


## Function calls to C code side.
```

////  Terminal control ////

Nemi::term_get_rows        < No arguments >
Nemi::term_get_cols        < No arguments >
Nemi::term_ignore_chars    < No arguments >    // Terminals will ignore character inputs.
Nemi::term_ignore_keys     < No arguments >    // Terminals will ignore key inputs.
Nemi::term_unignore_chars  < No arguments >
Nemi::term_unignore_keys   < No arguments >
Nemi::term_hide_cells      (col, row, width, height) // Parts of the rendered cells can be
Nemi::term_show_cells      (col, row, width, height) // hidden to make room for your own stuff
Nemi::term_scroll_y        (offset)
Nemi::term_get_char        (column, row)
Nemi::term_get_cursor_x    < No arguments >
Nemi::term_get_cursor_y    < No arguments >
Nemi::term_set_cell_custom_fg (column, row, hex_rgb_color)
Nemi::term_set_cell_custom_bg (column, row, hex_rgb_color)
Nemi::term_clear_cell_custom_fg (column, row)
Nemi::term_clear_cell_custom_bg (column, row)
Nemi::term_copy_to_clipboard    (start_column, start_row, end_column, end_row) 


////  Rendering  ////

here "rb" is short for "render buffer".
Render buffer consists of "nodes" which can be mesh, or text.
And they are just double linked list
because we can iterate through only active nodes when its time to render
and not waste time searching for them.

'Nemi::rb_add...' functions create nodes for the render buffer
and they return their 'node_index'


Nemi::new_renderbuf        (Max number of nodes)  // Returns rb_index.
Nemi::rb_remove_node       (rb_index, node_index)
Nemi::rb_add_rect          (rb_index, x, y, w, h, hex_color)
Nemi::rb_add_text          (rb_index, x, y, text, hex_color)
Nemi::rb_update_rect       (rb_index, node_index, x, y, w, h, hex_color)
Nemi::rb_update_text       (rb_index, node_index, x, y, text, hex_color)
Nemi::rb_hide_node         (rb_index, node_index) // Disable node rendering.
Nemi::rb_show_node         (rb_index, node_index)
Nemi::rb_use_cellcoords    (rb_index)   // Map X and Y coordinates to Column and Row.
Nemi::rb_use_arbcoords     (rb_index)   // No coordinate mapping, Normal X and Y.

// Setting layer specifies the rendering order.
// Text is always rendered last,
// thus changing text's layer is not going to have any effect.
// Layer changing works for everything else.
Nemi::rb_node_layer_first  (rb_index, node_index);
Nemi::rb_node_layer_last   (rb_index, node_index);

////  Misc  ////

Nemi::keydown              (GLFW Key number)
Nemi::create_msg           (text)

```

