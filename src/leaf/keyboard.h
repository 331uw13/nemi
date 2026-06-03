#ifndef LEAF_KEYBOARD_H
#define LEAF_KEYBOARD_H


// https://unix.stackexchange.com/questions/799766/is-there-a-list-of-every-scancode-that-linux-uses
#ifdef GRAPHICS_LINUX_FBDEV
#define KEYBOARD_MOD_SHIFT      1
#define KEYBOARD_MOD_CONTROL    2
#define KEYBOARD_MOD_ALT        3 
#define KEYBOARD_MOD_SUPER      4 
#define KEYBOARD_MOD_CAPS_LOCK  5 
#define KEYBOARD_MOD_NUM_LOCK   6 

#define KEYBOARD_KEY_SPACE      57
#define KEYBOARD_KEY_APOSTROPHE 40
#define KEYBOARD_KEY_COMMA      51
#define KEYBOARD_KEY_MINUS      12
#define KEYBOARD_KEY_PERIOD     52
#define KEYBOARD_KEY_SLASH      53
#define KEYBOARD_KEY_0          11
#define KEYBOARD_KEY_1          2
#define KEYBOARD_KEY_2          3 
#define KEYBOARD_KEY_3          4 
#define KEYBOARD_KEY_4          5 
#define KEYBOARD_KEY_5          6 
#define KEYBOARD_KEY_6          7 
#define KEYBOARD_KEY_7          8 
#define KEYBOARD_KEY_8          9 
#define KEYBOARD_KEY_9          10 
#define KEYBOARD_KEY_SEMICOLON  39 
#define KEYBOARD_KEY_EQUAL      13
#define KEYBOARD_KEY_A          30
#define KEYBOARD_KEY_B          48
#define KEYBOARD_KEY_C          46
#define KEYBOARD_KEY_D          32
#define KEYBOARD_KEY_E          18
#define KEYBOARD_KEY_F          33
#define KEYBOARD_KEY_G          34
#define KEYBOARD_KEY_H          35
#define KEYBOARD_KEY_I          23
#define KEYBOARD_KEY_J          36
#define KEYBOARD_KEY_K          37
#define KEYBOARD_KEY_L          38
#define KEYBOARD_KEY_M          50
#define KEYBOARD_KEY_N          49
#define KEYBOARD_KEY_O          24
#define KEYBOARD_KEY_P          25
#define KEYBOARD_KEY_Q          16
#define KEYBOARD_KEY_R          19
#define KEYBOARD_KEY_S          31
#define KEYBOARD_KEY_T          20
#define KEYBOARD_KEY_U          22
#define KEYBOARD_KEY_V          47
#define KEYBOARD_KEY_W          17
#define KEYBOARD_KEY_X          45
#define KEYBOARD_KEY_Y          21
#define KEYBOARD_KEY_Z          44
#define KEYBOARD_KEY_LEFT_BRACKET  26
#define KEYBOARD_KEY_BACKSLASH     43
#define KEYBOARD_KEY_RIGHT_BRACKET 27 
#define KEYBOARD_KEY_GRAVE_ACCENT  41
#define KEYBOARD_KEY_WORLD_1       -1 // ?
#define KEYBOARD_KEY_WORLD_2       -2 // ?
#define KEYBOARD_KEY_ESCAPE        1
#define KEYBOARD_KEY_ENTER         28 
#define KEYBOARD_KEY_TAB           15
#define KEYBOARD_KEY_BACKSPACE     14
#define KEYBOARD_KEY_INSERT        110
#define KEYBOARD_KEY_DELETE        111
#define KEYBOARD_KEY_RIGHT         106
#define KEYBOARD_KEY_LEFT          105
#define KEYBOARD_KEY_DOWN          108
#define KEYBOARD_KEY_UP            103
#define KEYBOARD_KEY_PAGE_UP       104
#define KEYBOARD_KEY_PAGE_DOWN     109
#define KEYBOARD_KEY_HOME          102
#define KEYBOARD_KEY_END           107 
#define KEYBOARD_KEY_CAPS_LOCK     58
#define KEYBOARD_KEY_SCROLL_LOCK   70
#define KEYBOARD_KEY_NUM_LOCK      69
#define KEYBOARD_KEY_PRINT_SCREEN  -3  // didnt find.
#define KEYBOARD_KEY_PAUSE         119
#define KEYBOARD_KEY_F1            59
#define KEYBOARD_KEY_F2            60
#define KEYBOARD_KEY_F3            61
#define KEYBOARD_KEY_F4            62
#define KEYBOARD_KEY_F5            63
#define KEYBOARD_KEY_F6            64 
#define KEYBOARD_KEY_F7            65
#define KEYBOARD_KEY_F8            66
#define KEYBOARD_KEY_F9            67
#define KEYBOARD_KEY_F10           68
#define KEYBOARD_KEY_F11           87
#define KEYBOARD_KEY_F12           88
#define KEYBOARD_KEY_F13           183
#define KEYBOARD_KEY_F14           184
#define KEYBOARD_KEY_F15           185
#define KEYBOARD_KEY_F16           186
#define KEYBOARD_KEY_F17           187
#define KEYBOARD_KEY_F18           188
#define KEYBOARD_KEY_F19           189
#define KEYBOARD_KEY_F20           190
#define KEYBOARD_KEY_F21           191
#define KEYBOARD_KEY_F22           192
#define KEYBOARD_KEY_F23           193
#define KEYBOARD_KEY_F24           194
#define KEYBOARD_KEY_F25           -4  // didnt find.
#define KEYBOARD_KEY_KP_0          0 
#define KEYBOARD_KEY_KP_1          0 
#define KEYBOARD_KEY_KP_2          0 
#define KEYBOARD_KEY_KP_3          0 
#define KEYBOARD_KEY_KP_4          0 
#define KEYBOARD_KEY_KP_5          0 
#define KEYBOARD_KEY_KP_6          0 
#define KEYBOARD_KEY_KP_7          0 
#define KEYBOARD_KEY_KP_8          0 
#define KEYBOARD_KEY_KP_9          0 
#define KEYBOARD_KEY_KP_DECIMAL    0 
#define KEYBOARD_KEY_KP_DIVIDE     0 
#define KEYBOARD_KEY_KP_MULTIPLY   0 
#define KEYBOARD_KEY_KP_SUBTRACT   0 
#define KEYBOARD_KEY_KP_ADD        0 
#define KEYBOARD_KEY_KP_ENTER      0 
#define KEYBOARD_KEY_KP_EQUAL      0 
#define KEYBOARD_KEY_LEFT_SHIFT    0 
#define KEYBOARD_KEY_LEFT_CONTROL  0 
#define KEYBOARD_KEY_LEFT_ALT      0 
#define KEYBOARD_KEY_LEFT_SUPER    0 
#define KEYBOARD_KEY_RIGHT_SHIFT   0 
#define KEYBOARD_KEY_RIGHT_CONTROL 0 
#define KEYBOARD_KEY_RIGHT_ALT     0 
#define KEYBOARD_KEY_RIGHT_SUPER   0 
#define KEYBOARD_KEY_MENU          0 

#endif


#ifdef GRAPHICS_OPENGL

#include <GLFW/glfw3.h>

#define KEYBOARD_MOD_SHIFT     GLFW_MOD_SHIFT    
#define KEYBOARD_MOD_CONTROL   GLFW_MOD_CONTROL  
#define KEYBOARD_MOD_ALT       GLFW_MOD_ALT      
#define KEYBOARD_MOD_SUPER     GLFW_MOD_SUPER    
#define KEYBOARD_MOD_CAPS_LOCK GLFW_MOD_CAPS_LOCK
#define KEYBOARD_MOD_NUM_LOCK  GLFW_MOD_NUM_LOCK 

#define KEYBOARD_KEY_SPACE      GLFW_KEY_SPACE        
#define KEYBOARD_KEY_APOSTROPHE GLFW_KEY_APOSTROPHE   
#define KEYBOARD_KEY_COMMA      GLFW_KEY_COMMA        
#define KEYBOARD_KEY_MINUS      GLFW_KEY_MINUS        
#define KEYBOARD_KEY_PERIOD     GLFW_KEY_PERIOD       
#define KEYBOARD_KEY_SLASH      GLFW_KEY_SLASH        
#define KEYBOARD_KEY_0          GLFW_KEY_0            
#define KEYBOARD_KEY_1          GLFW_KEY_1            
#define KEYBOARD_KEY_2          GLFW_KEY_2            
#define KEYBOARD_KEY_3          GLFW_KEY_3            
#define KEYBOARD_KEY_4          GLFW_KEY_4            
#define KEYBOARD_KEY_5          GLFW_KEY_5            
#define KEYBOARD_KEY_6          GLFW_KEY_6            
#define KEYBOARD_KEY_7          GLFW_KEY_7            
#define KEYBOARD_KEY_8          GLFW_KEY_8            
#define KEYBOARD_KEY_9          GLFW_KEY_9            
#define KEYBOARD_KEY_SEMICOLON  GLFW_KEY_SEMICOLON    
#define KEYBOARD_KEY_EQUAL      GLFW_KEY_EQUAL        
#define KEYBOARD_KEY_A          GLFW_KEY_A            
#define KEYBOARD_KEY_B          GLFW_KEY_B            
#define KEYBOARD_KEY_C          GLFW_KEY_C            
#define KEYBOARD_KEY_D          GLFW_KEY_D            
#define KEYBOARD_KEY_E          GLFW_KEY_E            
#define KEYBOARD_KEY_F          GLFW_KEY_F            
#define KEYBOARD_KEY_G          GLFW_KEY_G            
#define KEYBOARD_KEY_H          GLFW_KEY_H            
#define KEYBOARD_KEY_I          GLFW_KEY_I            
#define KEYBOARD_KEY_J          GLFW_KEY_J            
#define KEYBOARD_KEY_K          GLFW_KEY_K            
#define KEYBOARD_KEY_L          GLFW_KEY_L            
#define KEYBOARD_KEY_M          GLFW_KEY_M            
#define KEYBOARD_KEY_N          GLFW_KEY_N            
#define KEYBOARD_KEY_O          GLFW_KEY_O            
#define KEYBOARD_KEY_P          GLFW_KEY_P            
#define KEYBOARD_KEY_Q          GLFW_KEY_Q            
#define KEYBOARD_KEY_R          GLFW_KEY_R            
#define KEYBOARD_KEY_S          GLFW_KEY_S            
#define KEYBOARD_KEY_T          GLFW_KEY_T            
#define KEYBOARD_KEY_U          GLFW_KEY_U            
#define KEYBOARD_KEY_V          GLFW_KEY_V            
#define KEYBOARD_KEY_W          GLFW_KEY_W            
#define KEYBOARD_KEY_X          GLFW_KEY_X            
#define KEYBOARD_KEY_Y          GLFW_KEY_Y            
#define KEYBOARD_KEY_Z          GLFW_KEY_Z            
#define KEYBOARD_KEY_LEFT_BRACKET GLFW_KEY_LEFT_BRACKET 
#define KEYBOARD_KEY_BACKSLASH    GLFW_KEY_BACKSLASH    
#define KEYBOARD_KEY_RIGHT_BRACKET GLFW_KEY_RIGHT_BRACKET
#define KEYBOARD_KEY_GRAVE_ACCENT  GLFW_KEY_GRAVE_ACCENT 
#define KEYBOARD_KEY_WORLD_1       GLFW_KEY_WORLD_1      
#define KEYBOARD_KEY_WORLD_2       GLFW_KEY_WORLD_2      
#define KEYBOARD_KEY_ESCAPE        GLFW_KEY_ESCAPE       
#define KEYBOARD_KEY_ENTER         GLFW_KEY_ENTER        
#define KEYBOARD_KEY_TAB           GLFW_KEY_TAB          
#define KEYBOARD_KEY_BACKSPACE     GLFW_KEY_BACKSPACE    
#define KEYBOARD_KEY_INSERT        GLFW_KEY_INSERT       
#define KEYBOARD_KEY_DELETE        GLFW_KEY_DELETE       
#define KEYBOARD_KEY_RIGHT         GLFW_KEY_RIGHT        
#define KEYBOARD_KEY_LEFT          GLFW_KEY_LEFT         
#define KEYBOARD_KEY_DOWN          GLFW_KEY_DOWN         
#define KEYBOARD_KEY_UP            GLFW_KEY_UP           
#define KEYBOARD_KEY_PAGE_UP       GLFW_KEY_PAGE_UP      
#define KEYBOARD_KEY_PAGE_DOWN     GLFW_KEY_PAGE_DOWN    
#define KEYBOARD_KEY_HOME          GLFW_KEY_HOME         
#define KEYBOARD_KEY_END           GLFW_KEY_END          
#define KEYBOARD_KEY_CAPS_LOCK     GLFW_KEY_CAPS_LOCK    
#define KEYBOARD_KEY_SCROLL_LOCK   GLFW_KEY_SCROLL_LOCK  
#define KEYBOARD_KEY_NUM_LOCK      GLFW_KEY_NUM_LOCK     
#define KEYBOARD_KEY_PRINT_SCREEN  GLFW_KEY_PRINT_SCREEN 
#define KEYBOARD_KEY_PAUSE         GLFW_KEY_PAUSE        
#define KEYBOARD_KEY_F1            GLFW_KEY_F1           
#define KEYBOARD_KEY_F2            GLFW_KEY_F2           
#define KEYBOARD_KEY_F3            GLFW_KEY_F3           
#define KEYBOARD_KEY_F4            GLFW_KEY_F4           
#define KEYBOARD_KEY_F5            GLFW_KEY_F5           
#define KEYBOARD_KEY_F6            GLFW_KEY_F6           
#define KEYBOARD_KEY_F7            GLFW_KEY_F7           
#define KEYBOARD_KEY_F8            GLFW_KEY_F8           
#define KEYBOARD_KEY_F9            GLFW_KEY_F9           
#define KEYBOARD_KEY_F10           GLFW_KEY_F10          
#define KEYBOARD_KEY_F11           GLFW_KEY_F11          
#define KEYBOARD_KEY_F12           GLFW_KEY_F12          
#define KEYBOARD_KEY_F13           GLFW_KEY_F13          
#define KEYBOARD_KEY_F14           GLFW_KEY_F14          
#define KEYBOARD_KEY_F15           GLFW_KEY_F15          
#define KEYBOARD_KEY_F16           GLFW_KEY_F16          
#define KEYBOARD_KEY_F17           GLFW_KEY_F17          
#define KEYBOARD_KEY_F18           GLFW_KEY_F18          
#define KEYBOARD_KEY_F19           GLFW_KEY_F19          
#define KEYBOARD_KEY_F20           GLFW_KEY_F20          
#define KEYBOARD_KEY_F21           GLFW_KEY_F21          
#define KEYBOARD_KEY_F22           GLFW_KEY_F22          
#define KEYBOARD_KEY_F23           GLFW_KEY_F23          
#define KEYBOARD_KEY_F24           GLFW_KEY_F24          
#define KEYBOARD_KEY_F25           GLFW_KEY_F25          
#define KEYBOARD_KEY_KP_0          GLFW_KEY_KP_0         
#define KEYBOARD_KEY_KP_1          GLFW_KEY_KP_1         
#define KEYBOARD_KEY_KP_2          GLFW_KEY_KP_2         
#define KEYBOARD_KEY_KP_3          GLFW_KEY_KP_3         
#define KEYBOARD_KEY_KP_4          GLFW_KEY_KP_4         
#define KEYBOARD_KEY_KP_5          GLFW_KEY_KP_5         
#define KEYBOARD_KEY_KP_6          GLFW_KEY_KP_6         
#define KEYBOARD_KEY_KP_7          GLFW_KEY_KP_7         
#define KEYBOARD_KEY_KP_8          GLFW_KEY_KP_8         
#define KEYBOARD_KEY_KP_9          GLFW_KEY_KP_9         
#define KEYBOARD_KEY_KP_DECIMAL    GLFW_KEY_KP_DECIMAL   
#define KEYBOARD_KEY_KP_DIVIDE     GLFW_KEY_KP_DIVIDE    
#define KEYBOARD_KEY_KP_MULTIPLY   GLFW_KEY_KP_MULTIPLY  
#define KEYBOARD_KEY_KP_SUBTRACT   GLFW_KEY_KP_SUBTRACT  
#define KEYBOARD_KEY_KP_ADD        GLFW_KEY_KP_ADD       
#define KEYBOARD_KEY_KP_ENTER      GLFW_KEY_KP_ENTER     
#define KEYBOARD_KEY_KP_EQUAL      GLFW_KEY_KP_EQUAL     
#define KEYBOARD_KEY_LEFT_SHIFT    GLFW_KEY_LEFT_SHIFT   
#define KEYBOARD_KEY_LEFT_CONTROL  GLFW_KEY_LEFT_CONTROL 
#define KEYBOARD_KEY_LEFT_ALT      GLFW_KEY_LEFT_ALT     
#define KEYBOARD_KEY_LEFT_SUPER    GLFW_KEY_LEFT_SUPER   
#define KEYBOARD_KEY_RIGHT_SHIFT   GLFW_KEY_RIGHT_SHIFT  
#define KEYBOARD_KEY_RIGHT_CONTROL GLFW_KEY_RIGHT_CONTROL
#define KEYBOARD_KEY_RIGHT_ALT     GLFW_KEY_RIGHT_ALT    
#define KEYBOARD_KEY_RIGHT_SUPER   GLFW_KEY_RIGHT_SUPER  
#define KEYBOARD_KEY_MENU          GLFW_KEY_MENU         

#endif


#define KEYBOARD_KEY_LAST          KEYBOARD_KEY_MENU



#endif
