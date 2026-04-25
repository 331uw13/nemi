# Configuration documentation

* You can find all the configuration files from `./congfigs` directory.

## nemi.ini
```
[MAIN_SETTINGS]
shell = /bin/bash
padding_x = 10.0
padding_y = 10.0
line_padding = 3.0
vsync = true

# Blink settings are for cursor.
soft_blink = true
soft_blink_pow = 1.0
blink_speed = 9.0
show_frametime = false
hide_mouse = false

# If this is set to something you
# have installed, you can open files with it from scripts.
favourite_texteditor = vim

# This is the directory where the source code for Nemi is.
# Set empty if you dont need it.
source_dir = .
recompile_num_cores = 8
```


## font.ini

* Font `filepath` is searched from `./fonts` directory.

```
[FONT_SETTINGS]     
filepath = Topaz-8.ttf
center_char_to_cell = true
char_spacing = 1.0  
italic_tilt = 0.8   
underline_height = 1
underline_offset = 1.3
```


## log.ini

* If `output` is set to anything else than `<print>` it is redirected to a file that name.
* If `include_callee` is set to "true" the log includes the function location in the source code where the message came from.


```
[NEMI_LOG_CONFIG]   
enabled = true
output = <print>    
include_callee = false
use_color = true    
enable_info = true  
enable_warnings = true
enable_errors = true
```

## color.ini

* Color theme allows for hexadecimal color code and RGB code.

```
[COLOR_THEME]
                    
black = #000000     
red = ( 180, 30, 30 )
green = ( 30, 180, 30 )
yellow = ( 180, 180, 30 )
blue = ( 30, 30, 180 )
magenta = ( 180, 30, 180 )
cyan = ( 30, 180, 180 )
white = ( 180, 180, 180 )

bright_black = ( 70, 70, 70 )
bright_red = ( 255, 30, 30 )
bright_green = ( 30, 255, 30 )
bright_yellow = ( 255, 255, 30 )
bright_blue = ( 30, 30, 255 )
bright_magenta = ( 255, 30, 255 )
bright_cyan = ( 30, 255, 255 )
bright_white = ( 255, 255, 255 )

bg = ( 4, 4, 10 )
fg = ( 200, 180, 160 )

messages_fg = #A5FFA5
messages_bg = #151515
messages_border = #FF0000
```

