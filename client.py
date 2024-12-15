import serial
import threading
import time

# Configuration parameters
ESP32_PORT = '/dev/ttyUSB0'  # Replace with your ESP32's Serial port
BAUD_RATE = 921600
TIMEOUT = 1

class ESP32Client:
    def __init__(self, port, baud_rate, timeout=1):
        self.ser = serial.Serial(port, baud_rate, timeout=timeout)
        self.running = True
        if self.ser.is_open:
            print(f"Connected to ESP32 on {port} at {baud_rate} baud.")

        # Start a separate thread for listening to Serial messages
        self.listener_thread = threading.Thread(target=self.listen_serial)
        self.listener_thread.daemon = True
        self.listener_thread.start()
        self.sync_time()

    def listen_serial(self):
        """Continuously listens for incoming messages from the ESP32."""
        while self.running:
            if self.ser.in_waiting > 0:  # Check if there is data waiting
                response = self.ser.readline().decode().strip()
                print(f"\n[ESP32] {response}")  # Print ESP32's response immediately
            time.sleep(0.1)  # Avoid busy waiting

    def send_command(self, command:str):
        """Send a command to the ESP32 and handle any immediate response."""
        self.ser.write((command + '\n').encode())
        print(f"[Sent] {command}")

    def point_to(self, azimuth, elevation):
        """Send point_to command with azimuth and elevation parameters."""
        command = f"point_to {azimuth} {elevation}"
        self.send_command(command)

    def ping(self):
        self.send_command("ping")
    
    def cancel(self):
        """Send cancel command."""
        self.send_command("cancel")
    
    def status(self):
        """Request status of the antenna pointing mechanism."""
        self.send_command("status")
    
    def close(self):
        """Stop the listener and close the Serial connection."""
        self.running = False
        self.listener_thread.join()
        self.ser.close()
        print("Serial connection closed.")

    def sync_time(self):
        """Synchronizes clocks"""
        command = f"sync_time {time.time()}"
        self.send_command(command)

# Usage
if __name__ == "__main__":
    esp32 = ESP32Client(ESP32_PORT, BAUD_RATE, TIMEOUT)

    try:
        while True:
            try: 
                # Get user input and send commands accordingly
                cmd = input("Enter command (point_to <az> <elev>, cancel, status, or quit): ")
                # TODO asses whether we should let user send any command
                if cmd.startswith("point_to"):
                    _, az, elev = cmd.split()
                    esp32.point_to(az, elev)
                elif cmd.startswith("ping"):
                    esp32.ping()
                    
                elif cmd == "cancel":
                    esp32.cancel()
                elif cmd == "status":
                    esp32.status()
                elif cmd == "quit":
                    break
                else:
                    esp32.send_command(cmd)
            except ValueError:
                esp32.send_command(cmd)

    finally:
        esp32.close()