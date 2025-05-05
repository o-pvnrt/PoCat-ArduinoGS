## Introduction
This project aims to provide a simple Ground Station to ease development of the PoCat PockeQubes and provide easy debugging and data reception coordinated with the COMMS Subsystem. This is then an Arduino script
to control an AB01 Dev-Board (V2) to provide simple reception and transmission of data as
well as a script to decode information from the PoCat-1 Payload.

---

## Arduino Ground Station

### File: `PoCat_LoRaArduinoGS/PoCat_LoRaArduinoGS.ino`

This Arduino sketch implements the main logic for sending and receiving telecommands over LoRa, including:

- **Packet interleaving** for error resilience
- **Telecommand handling** via serial interface
- **LoRa radio configuration** (frequency, spreading factor, coding rate, etc.)
- **Callbacks for transmission and reception**
- **Support for a wide range of telecommands** (see `telecommandIDS` enum in the code)

The code is documented throughout for clarity and ease of modification.

#### Dependencies

The sketch makes use of the Cubecell Framework (https://docs.heltec.org/en/node/asr650x/asr650x_general_docs/quick_start/cubecell-use-arduino-board-manager.html). This should be installed in order to communicate with the board.
---

## Image Decoding Tool

### Folder: `Image Decoding`

This tool decodes raw LoRa data received from the PoCat 1 VGA Camera and reconstructs the original JPEG image taken by the satellite.

### Inputs

- A `.txt` file containing the raw messages sent by the satellite.
  - Each line should represent a received message in hexadecimal format, as output by your LoRa receiver.

### Outputs

- A `.jpg` file containing the decoded image.
  - The script processes the input file and reconstructs the original JPEG image captured by the PoCat 1 camera.

### How to Use

1. Run the provided `Filter.py` script or use the included `Filter.exe` file for a graphical interface.
2. Select your raw input `.txt` file when prompted.
3. Preview the file contents in the application.
4. Click "Save as JPG" and choose the output location and filename.
5. The decoded JPEG image will be saved to your chosen location.

#### Notes

- The `.exe` file provides the same functionality as the Python script, without requiring Python to be installed.
- Make sure your input file is in the correct format (raw hex messages from the satellite).
- The `Filter_exe_code.py` file provides the source code for building the executable.

---

## Project Structure

- `PoCat_LoRaArduinoGS/PoCat_LoRaArduinoGS.ino` — Main Arduino ground station code
- `Image Decoding/` — Tools for decoding and reconstructing images from received LoRa data


---