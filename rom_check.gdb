set confirm off
target remote :1234
# Check what's loaded at ROM base
echo \n=== ROM CONTENT CHECK ===\n
x/10i 0x0
echo \n=== CPU STATE ===\n
info registers
echo \n=== MEMORY LAYOUT ===\n
info mem
echo \n=== PC LOCATION ===\n
print $pc
x/5i $pc
quit
