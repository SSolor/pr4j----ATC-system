import tkinter as tk
from tkinter import messagebox
import ctypes
import os

# Load the C++ DLL
dll_path = os.path.join(os.path.dirname(__file__), "server.dll")
socket_lib = ctypes.CDLL(dll_path)

# Define function signatures
socket_lib.init_winsock.restype = ctypes.c_int
socket_lib.cleanup_winsock.restype = None
socket_lib.send_message.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
socket_lib.send_message.restype = ctypes.c_int

# Initialize Winsock
if socket_lib.init_winsock() != 0:
    messagebox.showerror("Error", "Failed to initialize Winsock")
    exit(1)

class ClientApp:
    def __init__(self, root):
        self.root = root
        self.root.title("TCP Client")
        self.root.geometry("300x150")
        
        # Server configuration
        self.server_ip = "127.0.0.1"
        self.server_port = 8888
        
        # UI elements
        label = tk.Label(root, text="Click to send message to server", font=("Arial", 12))
        label.pack(pady=20)
        
        self.button = tk.Button(root, text="Click", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=10, height=2)
        self.button.pack(pady=10)
        
        self.response_label = tk.Label(root, text="", font=("Arial", 10), fg="blue")
        self.response_label.pack(pady=10)
    
    def send_message(self):
        # Prepare buffer for response
        response_buffer = ctypes.create_string_buffer(1024)
        
        # Send message and get response
        result = socket_lib.send_message(
            self.server_ip.encode('utf-8'),
            self.server_port,
            response_buffer,
            1024
        )
        
        if result > 0:
            response = response_buffer.value.decode('utf-8')
            self.response_label.config(text=f"Server says: {response}")
        elif result == -1:
            messagebox.showerror("Error", "Failed to create socket")
        elif result == -2:
            messagebox.showerror("Error", "Failed to connect to server")
        elif result == -3:
            messagebox.showerror("Error", "Failed to send message")
        elif result == -4:
            messagebox.showerror("Error", "Failed to receive response")
    
    def on_closing(self):
        socket_lib.cleanup_winsock()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = ClientApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()