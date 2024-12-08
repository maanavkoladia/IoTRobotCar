def calculate_baud_divisors(system_clock, baud_rate):
    # Full baud rate divisor
    brd = system_clock / (16 * baud_rate)
    
    # Integer and fractional parts
    ibrd = int(brd)  # Integer part
    brdf = brd - ibrd  # Fractional part

    # Calculate FBRD with rounding
    fbrd = int((brdf * 64) + 0.5)  # Equivalent to rounding to nearest integer

    return ibrd, fbrd

# Constants
system_clock = 80000000  # 80 MHz system clock
baud_rate = 115200       # Desired baud rate

# Calculate IBRD and FBRD
ibrd, fbrd = calculate_baud_divisors(system_clock, baud_rate)

# Print results
print(f"Integer Baud-Rate Divisor (IBRD): {ibrd}")
print(f"Fractional Baud-Rate Divisor (FBRD): {fbrd}")

