# About
Vystriyae is a terminal rendering library in C for creating TUI applications. <br>
The library has. <br>
- Unicode handling. <br>
- Colors and styles. <br>
- Mouse, signal and keyboard input. Keyscans and textual input can be "seperated" via the IME layer. <br>
- An IME layer for custom and non ascii keyboard layouts. <br>
- A notification system. <br>
- A layering system with Z buffering. <br>
- Input and non input blocking frames. (Whether or not a frame requires user input in order to advance to the next frame. The first being the norm in TUIs) <br>

# Dependencies
Currently, the library depends on my C standard library (Mektova-C-Utils) and only builds on C23. <br>
However, the Mektova-C-Utils dependency will be removed in the near future. <br>
- (Mektova-C-Utils)[https://github.com/yuumei-02/Mektova-C-Utils]

