def process_hex_file(input_file, output_file):
    """
    Processes a hex text file by removing the first 14 characters of each line, inserting a space every 2 characters,
    and converting the resulting hex values to a binary file.

    Args:
        input_file: Path to the input text file.
        output_file: Path to the output binary file.
    """
    lines = []
    # Step 1: Remove the first 14 characters from each line
    with open(input_file, 'r') as infile:
        for line in infile:
            lines.append(line[14:])

    # Step 2: Insert a space every 2 characters for each line
    spaced_lines = []
    for line in lines:
        # Remove any trailing newlines before processing
        line = line.rstrip('\n')
        spaced_line = ' '.join(line[i:i+2] for i in range(0, len(line), 2))
        spaced_lines.append(spaced_line)

    # Step 3: Convert hex to binary and write to output file
    with open(output_file, 'wb') as out_file:
        for line in spaced_lines:
            hex_data = line.strip().split()
            if hex_data:  # Only process non-empty lines
                byte_array = bytearray(int(h, 16) for h in hex_data)
                out_file.write(byte_array)

# Example usage
input_file = "input.txt"
output_file = "image.jpg"
process_hex_file(input_file, output_file)

