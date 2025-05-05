import customtkinter as ctk
from tkinter import filedialog, messagebox
import os

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

class HexToJPGApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title('Po-Cat 1 Image Filter')
        self.geometry('800x450')
        ctk.set_appearance_mode('dark')  # Set dark mode
        ctk.set_default_color_theme('blue')  # Set custom color theme  

        self.input_file = None

        self.select_btn = ctk.CTkButton(self, text='Select TXT File', command=self.select_file)
        self.select_btn.pack(pady=10)

        self.preview_frame = ctk.CTkFrame(self)
        self.preview_frame.pack(pady=10, fill='both', expand=True)

        self.preview_box = ctk.CTkTextbox(self.preview_frame, width=750, height=300)
        self.preview_box.pack(fill='both', expand=True, padx=20)

        self.save_btn = ctk.CTkButton(self, text='Save as JPG', command=self.save_jpg)
        self.save_btn.pack(pady=20)

    def select_file(self):
        file_path = filedialog.askopenfilename(filetypes=[('Text Files', '*.txt')])
        if file_path:
            self.input_file = file_path
            with open(file_path, 'r') as f:
                preview = f.read()
            self.preview_box.delete('1.0', ctk.END)
            self.preview_box.insert('1.0', preview)

    def save_jpg(self):
        if not self.input_file:
            messagebox.showerror('Error', 'Please select a TXT file.')
            return
        output_path = filedialog.asksaveasfilename(
            defaultextension='.jpg',
            filetypes=[('JPEG Image', '*.jpg')],
            title='Save as JPG'
        )
        if not output_path:
            return
        try:
            process_hex_file(self.input_file, output_path)
            messagebox.showinfo('Success', f'Image saved as {output_path}')
        except Exception as e:
            messagebox.showerror('Error', f'Failed to save image: {e}')

if __name__ == '__main__':
    app = HexToJPGApp()
    app.mainloop()

