target remote localhost:2331
set verbose on

set output-radix 16
set pagination on

#Reset USB phy
set *((u32*)0xE000A040) = 0

monitor reset
monitor halt

load
