#!/usr/bin/env python3

"""
A simple server that listens for incoming connections and calculates the multiply-accumulation of two integers.
The server expects the two integers to be sent as a null-terminated string.
The server multiplies them and adds the result with the internal accumulaor value and responds with the accumulated sum as a null-terminated string.
"""
import socket


accumulator = 0


def handle_client(conn):
    global accumulator
    try:
        while True:
            # Receive data from the client
            data = conn.recv(1024)
            if not data:
                break  # Connection closed by client

            # Decode the null-terminated string and remove the null character
            message = data.decode("utf-8").rstrip("\x00")
            print(f"Received: {message}")

            # Parse the two integers
            try:
                a, b = map(int, message.split())
                result = a * b
                accumulator += result
            except ValueError:
                print("Invalid input format. Expected two integers.")
                result = 0

            # Send the accumulated result as a null-terminated string
            response = f"{accumulator}\x00"
            conn.sendall(response.encode("utf-8"))
            print(f"Sent: {response}")

    finally:
        conn.close()


def start_server():
    # Create a TCP/IP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to the port
    server_address = ("localhost", 1212)
    print(f"Starting server on {server_address}")
    server_socket.bind(server_address)

    # Listen for incoming connections
    server_socket.listen(1)

    # while True:
    print("Waiting for a connection...")
    connection, client_address = server_socket.accept()
    print(f"Connection from {client_address}")

    # Handle the client connection
    handle_client(connection)


if __name__ == "__main__":
    start_server()
