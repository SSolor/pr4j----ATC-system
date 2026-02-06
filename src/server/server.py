import tkinter as tk
from tkinter import scrolledtext
import ctypes
import os
import threading
import time

# Load the C++ DLL
dll_path = os.path.join(os.path.dirname(__file__), "server.dll")
server_lib = ctypes.CDLL(dll_path)

# Define function signatures
server_lib.init_winsock.restype = ctypes.c_int
server_lib.cleanup_winsock.restype = None
server_lib.start_server.argtypes = [ctypes.c_int]
server_lib.start_server.restype = ctypes.c_int
server_lib.check_for_client.argtypes = [ctypes.c_char_p, ctypes.c_int]
server_lib.check_for_client.restype = ctypes.c_int
server_lib.stop_server.restype = None
server_lib.is_server_running.restype = ctypes.c_int

class ServerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("TCP Server")
        self.root.geometry("500x400")
        
        self.port = 8888
        self.is_running = False
        self.server_thread = None
        
        # Initialize Winsock
        if server_lib.init_winsock() != 0:
            self.log("Failed to initialize Winsock")
            return
        
        # UI elements
        title_label = tk.Label(root, text="TCP Server Control Panel", font=("Arial", 16, "bold"))
        title_label.pack(pady=10)
        
        # Port display
        port_frame = tk.Frame(root)
        port_frame.pack(pady=5)
        tk.Label(port_frame, text=f"Port: {self.port}", font=("Arial", 12)).pack()
        
        # Start/Stop button
        self.control_button = tk.Button(root, text="Start Server", command=self.toggle_server,
                                       font=("Arial", 14), bg="#4CAF50", fg="white",
                                       width=15, height=2)
        self.control_button.pack(pady=20)
        
        # Status label
        self.status_label = tk.Label(root, text="Server stopped", font=("Arial", 12), fg="red")
        self.status_label.pack(pady=5)
        
        # Log area
        log_label = tk.Label(root, text="Server Log:", font=("Arial", 10))
        log_label.pack(pady=5)
        
        self.log_text = scrolledtext.ScrolledText(root, width=60, height=12, state='disabled')
        self.log_text.pack(pady=5)
        
    def log(self, message):
        self.log_text.config(state='normal')
        self.log_text.insert(tk.END, f"{message}\n")
        self.log_text.see(tk.END)
        self.log_text.config(state='disabled')
    
    def toggle_server(self):
        if not self.is_running:
            self.start_server()
        else:
            self.stop_server()
    
    def start_server(self):
        result = server_lib.start_server(self.port)
        
        if result == 0:
            self.is_running = True
            self.control_button.config(text="Stop Server", bg="#f44336")
            self.status_label.config(text="Server running", fg="green")
            self.log(f"Server started on port {self.port}")
            
            # Start server thread
            self.server_thread = threading.Thread(target=self.server_loop, daemon=True)
            self.server_thread.start()
        elif result == -1:
            self.log("ERROR: Failed to create socket")
        elif result == -2:
            self.log("ERROR: Failed to bind socket")
        elif result == -3:
            self.log("ERROR: Failed to listen on socket")
    
    def server_loop(self):
        while self.is_running:
            client_ip_buffer = ctypes.create_string_buffer(64)
            result = server_lib.check_for_client(client_ip_buffer, 64)
            
            if result == 1:
                client_ip = client_ip_buffer.value.decode('utf-8')
                self.log(f"Client connected from {client_ip}")
                self.log(f"Sent: hello client")
            elif result == -1:
                self.log("ERROR: Error accepting client")
            
            time.sleep(0.1)  # Small delay to prevent CPU spinning
    
    def stop_server(self):
        self.is_running = False
        server_lib.stop_server()
        
        self.control_button.config(text="Start Server", bg="#4CAF50")
        self.status_label.config(text="Server stopped", fg="red")
        self.log("Server stopped - all clients disconnected")
    
    def on_closing(self):
        if self.is_running:
            self.stop_server()
        server_lib.cleanup_winsock()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = ServerApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()