import socket
import sys

# TODO: Change this to your ESP32's IP address
SERVER_IP = "192.168.1.1"  

# TODO: Change this to the port your ESP32 is listening on
UDP_PORT = 4321            

# The server's address tuple (IP, Port). We need this for sendto()
server_address = (SERVER_IP, UDP_PORT)

# 1. Create a UDP socket. There is no 'connect()' step for this simple UDP client
try:
    # Use SOCK_DGRAM for UDP
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
except socket.error as e:
    print(f"Error creating socket: {e}")
    sys.exit()

print(f"UDP socket created. Ready to send data to {SERVER_IP}:{UDP_PORT}")

# 2. Loop for user input
try:
    while True:
        # Ask the user for data
        message = input("Enter hex data to send (or 'q' to quit): ")

        # Check for the exit command
        if message.lower() == 'q':
            print("Exit command received. Closing socket.")
            break  # Exit the while loop

        # Send the data
        try:
            # Convert the hex string to raw bytes
            data_to_send = bytes.fromhex(message)
            
            # Use sendto() to send the data to the server address
            client_socket.sendto(data_to_send, server_address)
            
            print(f"Sent: '{message}'")
        
        except ValueError:
            # Handle cases where the input is not valid hex
            print("Error: Input was not a valid hex string. Please try again.")
        except socket.error as e:
            print(f"Error sending data: {e}")
            break # Exit the loop if sending fails

except KeyboardInterrupt:
    print("\nScript terminated by user.")

finally:
    # 3. Close the socket
    client_socket.close()
    print("Socket closed.")