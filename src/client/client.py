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
        self.init_main_page_PRE_FLIGHT("Ryan")
        
    ### Message Logic Functions ###
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
            if hasattr(self, 'response_text'):
                self.display_server_response(response)
            else:
                messagebox.showinfo("Server Response", response)
        else:
            # Map error codes to messages
            if result == -1:
                err = "Failed to create socket"
            elif result == -2:
                err = "Failed to connect to server"
            elif result == -3:
                err = "Failed to send message"
            elif result == -4:
                err = "Failed to receive response"
            else:
                err = f"Unknown error (code={result})"

            # Display error in the response area if available
            if hasattr(self, 'response_text'):
                self.display_server_response(f"ERROR: {err}")
 
    def create_response_area(self):
        # Create a small, read-only text area to show server responses
        self.response_frame = tk.Frame(self.root)
        self.response_frame.pack(fill='x', padx=10, pady=(0,5))
        self.response_text = tk.Text(self.response_frame, height=4, wrap='word', font=("Arial", 10))
        self.response_text.pack(fill='x')
        self.response_text.configure(state='disabled')

    def display_server_response(self, text):
        try:
            self.response_text.configure(state='normal')
            self.response_text.delete('1.0', tk.END)
            self.response_text.insert(tk.END, text)
            self.response_text.configure(state='disabled')
        except Exception:
            # Fallback to messagebox if widget not available for some reason
            messagebox.showinfo("Server Response", text)

    
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
            self.init_main_page_PRE_FLIGHT(username)
        else:
            messagebox.showerror("Login Failed", "Invalid username or password")
    

    ##############################################
    ## GUI elements for main page -- PRE_FLIGHT ##
    ##############################################
    def init_main_page_PRE_FLIGHT(self, username):
        for widget in self.root.winfo_children():
            widget.destroy()

        label = tk.Label(self.root, text="Welcome to ATC System " + username, font=("Arial", 12))
        label.pack(pady=10)
        # Server response area (appears under the welcome message)
        self.create_response_area()
        
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
        self.button = tk.Button(self.root, text="Request Taxi", command=lambda: self.init_main_page_ACTIVE_AIRSPACE(username), 
                    font=("Arial", 14), bg="#4CAF50", fg="white", 
                    width=30, height=2)
        self.button.pack(pady=5)


        ### DATA_TRANSFER ###
        ## Flight Manual
        self.button = tk.Button(self.root, text="Request Flight Manual", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)


        ### EMERGENCY ###
        ## Emergency notification
        self.button = tk.Button(self.root, text="EMERGENCY", command=self.send_message,
                                font=("Arial", 14), bg="#f44336", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)

        ### Logout button ###
        logout_button = tk.Button(self.root, text="Logout", command=self.init_login_page, 
                    font=("Arial", 12), bg="#f44336", fg="white", width=10, height=1)
        logout_button.pack(pady=10)



    ###################################################
    ## GUI elements for main page -- ACTIVE_AIRSPACE ##
    ###################################################
    def init_main_page_ACTIVE_AIRSPACE(self, username):
        for widget in self.root.winfo_children():
            widget.destroy()

        label = tk.Label(self.root, text="Welcome to ATC System " + username, font=("Arial", 12))
        label.pack(pady=10)
        # Server response area (appears under the welcome message)
        self.create_response_area()

        ### ACTIVE_AIRSPACE specific buttons ###
        ## Telemetry data 
        self.button = tk.Button(self.root, text="Telemetry Update", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)

        ## Air-Traffic data
        self.button = tk.Button(self.root, text="Air-Traffic Update", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)

        ## Runway Clearance Request
        self.button = tk.Button(self.root, text="Clear Runway", command=lambda: self.init_main_page_PRE_FLIGHT(username), 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)
        
        ### Global Buttons ###
        ## Flight Manual
        self.button = tk.Button(self.root, text="Request Flight Manual", command=self.send_message, 
                                font=("Arial", 14), bg="#4CAF50", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)

        ## Emergency notification
        self.button = tk.Button(self.root, text="EMERGENCY", command=self.send_message,
                                font=("Arial", 14), bg="#f44336", fg="white", 
                                width=30, height=2)
        self.button.pack(pady=5)


    ### Functions ###



    # Cleanup Winsock on closing the application
    def on_closing(self):
        socket_lib.cleanup_winsock()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = ClientApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()