import serial
import csv
import os
import time

# Configure the serial port
serial_port = 'COM5'  # Replace with your serial port (e.g., COM1, COM2, etc.)
baud_rate = 115200

# Open the serial port
ser = serial.Serial(serial_port, baud_rate)

# Create initial log files
log_files = {}
log_files["1.00"] = open('Dev_1.txt', 'w')
log_files["2.00"] = open('Dev_2.txt', 'w')
log_files["3.00"] = open('Dev_3.txt', 'w')

# Initialize the current_time_ms dictionary for all device identifiers
current_time_ms = {identifier: 0 for identifier in log_files.keys()}

try:
    while True:
        # Read a line from the serial port
        line = ser.readline().decode('utf-8').strip()
        print(line)
        
        # Get the identifier from the line
        values = line.split(',')
        if len(values) >= 1:
            identifier = values[0]
            
            # Write the line to the appropriate log file
            if identifier in log_files:
                log_file = log_files[identifier]
                log_file.write(line + '\n')
                log_file.flush()  # Flush to ensure immediate write

                # Increment timestamp for the current device identifier
                current_time_ms[identifier] += 1  # Increment timestamp by 1ms

except KeyboardInterrupt:
    # Close the serial port and log files on Ctrl+C
    ser.close()
    for log_file in log_files.values():
        log_file.close()
    print('\nData Log process complete\n')

filepaths = ['Dev_1.txt', 'Dev_2.txt', 'Dev_3.txt']
output_file = 'Output.csv'

# Lists to store data from different files
data_lists = [[] for _ in range(len(filepaths))]

# Dictionary to store timestamps for each device
timestamps = {}

# Record timestamps starting from 0ms for each device
for identifier in log_files.keys():
    timestamps[identifier] = []

for filepath in filepaths:
    identifier = os.path.splitext(os.path.basename(filepath))[0]  # Extract identifier from filename
    with open(filepath, 'r') as file:
        lines = file.readlines()

    with open(filepath, 'w') as file:
        for line in lines:
            data = ','.join(line.strip().split(',')[1:])
            file.write(data + '\n')
            current_time_ms[identifier] += 1  # Increment timestamp by 1ms
            timestamps[identifier].append(current_time_ms[identifier])

# Read data from each file and store it in the appropriate list
for index, filepath in enumerate(filepaths):
    identifier = os.path.splitext(os.path.basename(filepath))[0]  # Extract identifier from filename
    with open(filepath, 'r') as file:
        lines = file.readlines()

        for line in lines:
            values = line.strip().split(',')
            if len(values) >= 6:
                data_lists[index].append([float(val) if val else None for val in values[:6]])  # Skip the timestamp

# Prepare column headers for the CSV
column_headers = []

for identifier in log_files.keys():
    column_headers.extend([f'{identifier}_ACC_X', f'{identifier}_ACC_Y', f'{identifier}_ACC_Z',
                           f'{identifier}_GYRO_X', f'{identifier}_GYRO_Y', f'{identifier}_GYRO_Z'])

# Write data to the output CSV file with timestamps
with open(output_file, 'w', newline='') as csvfile:
    csvwriter = csv.writer(csvfile)
    
    # Write column headers
    csvwriter.writerow(column_headers)
    
    # Find the maximum number of rows among data lists
    max_rows = max(len(data) for data in data_lists)
    
    # Write data rows with timestamps for each device
    for row_idx in range(max_rows):
        row_data = []
        for data in data_lists:
            if row_idx < len(data):
                row_data.extend(data[row_idx])
            else:
                row_data.extend([None] * 6)  # Padding with None if data is missing
        csvwriter.writerow(row_data)

print('\n.csv process complete\n')
time.sleep(3)

# Specify the directory where the .txt files are located
directory = r"C:\Users\yaswa\OneDrive\Documents\Python"

# Get a list of all files in the directory
file_list = os.listdir(directory)

# Loop through the files and delete .txt files
for file in file_list:
    if file.endswith(".txt"):
        file_path = os.path.join(directory, file)
        os.remove(file_path)
        print(f"Deleted: {file_path}")
