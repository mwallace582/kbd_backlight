# kbd_backlight
Controls the keyboard backlight on Linux

Options:

-m

  Maximum, set to 100%
  
-o

  Off, set to 0%

-s `<level>`

  Set the level to `<level>` - must be between 0 and maximum

-u `<increment>`

  Increment the level by `<increment>` - must be between 0 and maximum

-d <increment>

  Decrement the level by `<increment>` - must be between 0 and maximum

This is hardcoded to use `/sys/class/leds/kbd_backlight/brightness`.  If
your system does not use this file for backlight control, then this
program isn't for you.
