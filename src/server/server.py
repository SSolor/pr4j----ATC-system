import tkinter as tk
from tkinter import ttk
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
        self.root.title("ATC Tower Console")
        self.root.geometry("900x600")
        self.root.configure(bg="#e6e6e6")

        self.port = 8888
        self.is_running = False
        self.server_thread = None
        self.log_id = 1

        if server_lib.init_winsock() != 0:
            self.add_log("-", "Y", "Winsock init failed", "N/A", "-")
            return

        self.build_ui()

    # ---------------- UI BUILD ---------------- #

    def build_ui(self):
        # Header Bar
        header = tk.Frame(self.root, bg="#1e1e1e", height=60)
        header.pack(fill="x")

        tk.Label(
            header,
            text="ATC TOWER Air Traffic Control Server",
            fg="white",
            bg="#1e1e1e",
            font=("Segoe UI", 18, "bold")
        ).place(x=20, y=15)

        tk.Button(
            header,
            text="RADAR LINK",
            bg="#ff8c42",
            fg="black",
            font=("Segoe UI", 12, "bold"),
            width=12,
            height=1
        ).place(x=750, y=15)

        # Runway Port Panel
        port_frame = tk.Frame(self.root, bg="white", bd=2, relief="groove")
        port_frame.place(x=20, y=80, width=250, height=120)

        tk.Label(port_frame, text="Runway Port", font=("Segoe UI", 12, "bold"), bg="white").pack(pady=5)
        tk.Label(port_frame, text=str(self.port), font=("Segoe UI", 20, "bold"), bg="white").pack()

        self.status_label = tk.Label(port_frame, text="Tower Standby", fg="red", bg="white", font=("Segoe UI", 12))
        self.status_label.pack(pady=5)

        # Control Buttons
        tk.Button(
            self.root,
            text="Tower Standby",
            bg="#ff8c42",
            fg="black",
            font=("Segoe UI", 12, "bold"),
            width=15,
            command=self.toggle_server
        ).place(x=300, y=100)

        tk.Button(
            self.root,
            text="Clear Log",
            bg="#d9d9d9",
            fg="black",
            font=("Segoe UI", 12),
            width=12,
            command=self.clear_log
        ).place(x=300, y=150)

        tk.Label(
            self.root,
            text="Passive tower: accepts and logs incoming data.",
            font=("Segoe UI", 10),
            bg="#e6e6e6"
        ).place(x=300, y=190)

        # Flight Log Table
        columns = ("LogID", "ClientID", "Flag", "Job", "Time", "Status", "Location")

        self.table = ttk.Treeview(self.root, columns=columns, show="headings", height=18)
        self.table.place(x=20, y=230, width=860, height=340)

        for col in columns:
            self.table.heading(col, text=col)
            self.table.column(col, width=120, anchor="center")

        self.add_log("-", "Y", f"Server initialized on port {self.port}", "N/A", "-")

    # ---------------- LOGGING ---------------- #

    def add_log(self, client_id, flag, job, status, location):
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        self.table.insert("", "end", values=(
            self.log_id, client_id, flag, job, timestamp, status, location
        ))
        self.log_id += 1

    def clear_log(self):
        for row in self.table.get_children():
            self.table.delete(row)
        self.log_id = 1

    # ---------------- SERVER CONTROL ---------------- #

    def toggle_server(self):
        if not self.is_running:
            self.start_server()
        else:
            self.stop_server()

    def start_server(self):
        result = server_lib.start_server(self.port)

        if result == 0:
            self.is_running = True
            self.status_label.config(text="Tower Active", fg="green")
            self.add_log("-", "Y", f"Server started on port {self.port}", "N/A", "-")

            self.server_thread = threading.Thread(target=self.server_loop, daemon=True)
            self.server_thread.start()
        else:
            self.add_log("-", "Y", "Server failed to start", "ERROR", "-")

    def server_loop(self):
        while self.is_running:
            buf = ctypes.create_string_buffer(64)
            result = server_lib.check_for_client(buf, 64)

            if result == 1:
                ip = buf.value.decode()
                self.add_log(ip, "G", "Client connected", "Connected", ip)
                self.add_log(ip, "Y", "Sent: hello client", "Connected", ip)

            time.sleep(0.1)

    def stop_server(self):
        self.is_running = False
        server_lib.stop_server()
        self.status_label.config(text="Tower Standby", fg="red")
        self.add_log("-", "Y", "Server stopped", "N/A", "-")

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