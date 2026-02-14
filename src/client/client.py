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
        self.root.geometry("400x600")
        
        # Server configuration
        self.server_ip = "127.0.0.1"
        self.server_port = 8888
        
        # UI startup
        #self.init_login_page() 

        # UI TESTING, 
        ## Directly go to main page
        self.init_main_page("Ryan")
        
    
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
    
    #################################
    ## GUI elements for login page ##
    #################################
    def init_login_page(self):
        for widget in self.root.winfo_children():
            widget.destroy()   

        label = tk.Label(self.root, text="Login", font=("Arial", 16))
        label.pack(pady=10)

        self.username_entry = tk.Entry(self.root, font=("Arial", 12))
        self.username_entry.pack(pady=5)
        self.password_entry = tk.Entry(self.root, font=("Arial", 12), show="*")
        self.password_entry.pack(pady=5)

        login_button = tk.Button(self.root, text="Login", command=self.vaidate_login, 
                    font=("Arial", 12), bg="#4CAF50", fg="white", width=10, height=1)
        login_button.pack(pady=10)
        

    # login validation function      
    def vaidate_login(self):
        username = self.username_entry.get()
        password = self.password_entry.get()

        if username == "admin" and password == "password":
            messagebox.showinfo("Login Successful", "Welcome, admin!")
            self.init_main_page(username)
        else:
            messagebox.showerror("Login Failed", "Invalid username or password")
    

    ################################
    ## GUI elements for main page ##
    ################################
    def init_main_page(self, username):
        for widget in self.root.winfo_children():
            widget.destroy()

        label = tk.Label(self.root, text="Welcome to ATC System " + username, font=("Arial", 12))
        label.pack(pady=10)
        
        ### PRE_FLIGHT ###
        ## Flight Plan
        self.button = tk.Button(self.root, text="Flight Plan", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)
        ## Weather Report
        self.button = tk.Button(self.root, text="Weather Report", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)
        ## Taxi Request
        self.button = tk.Button(self.root, text="Request Taxi", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)


        ### DATA_TRANSFER ###
        ## Flight Manual
        self.button = tk.Button(self.root, text="Request Flight Manual", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)

        ### ACTIVE_AIRSPACE ###
        ## Runway Clearance Request
        self.button = tk.Button(self.root, text="Request Runway Clearance", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)

        ## Telemetry data & Air-Traffic data
        self.button = tk.Button(self.root, text="Request Aircraft Status", command=self.send_message, ## Telemetry data & Air-Traffic data (may be 2 buttons later on)
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)

        ### Server response ###
        self.response_label = tk.Label(self.root, text="", font=("Arial", 10), fg="blue") ## Should probably be a popup instead of a label, but for testing purposes this is fine
        self.response_label.pack(pady=5)


        ### Logout button ###
        logout_button = tk.Button(self.root, text="Logout", command=self.init_login_page, 
                    font=("Arial", 12), bg="#f44336", fg="white", width=10, height=1)
        logout_button.pack(pady=10)



 

  


    
    def on_closing(self):
        socket_lib.cleanup_winsock()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = ClientApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()