import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import subprocess
import threading
import time
import re
from datetime import datetime

SERVER_EXE = "build/server.exe"

class ATCTowerUI(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("ATC Tower Console")
        self.geometry("1200x700")
        self.minsize(1100, 650)

        self.server_process = None
        self.log_id = 0
        self.tower_active = True

        # ====== Styles ======
        self._setup_styles()

        # ====== Layout ======
        self._build_header()
        self._build_controls()
        self._build_log_area()

        # Periodic UI updates (filter apply)
        self.after(250, self._apply_filter)

    # -------------------------
    # Styling
    # -------------------------
    def _setup_styles(self):
        style = ttk.Style(self)
        style.theme_use("clam")

        self.COLOR_BG = "#E9F1F8"
        self.COLOR_NAV = "#0A3D8F"
        self.COLOR_ACCENT = "#F39C12"
        self.COLOR_SUCCESS = "#DFF3E3"
        self.COLOR_DARK_TABLE = "#0B1E33"
        self.COLOR_WHITE = "#FFFFFF"
        self.COLOR_MUTED = "#DDE6F0"

        self.configure(bg=self.COLOR_BG)

        style.configure("TFrame", background=self.COLOR_BG)
        style.configure("TLabel", background=self.COLOR_BG, foreground="#0D2033", font=("Segoe UI", 10))
        style.configure("Title.TLabel", background=self.COLOR_NAV, foreground="white", font=("Segoe UI", 22, "bold"))
        style.configure("SubTitle.TLabel", background=self.COLOR_NAV, foreground="#CFE0FF", font=("Segoe UI", 11))

        style.configure("Accent.TButton", font=("Segoe UI", 10, "bold"))
        style.map("Accent.TButton",
                  background=[("active", self.COLOR_ACCENT)],
                  foreground=[("active", "black")])

        style.configure("Primary.TButton", font=("Segoe UI", 10, "bold"))
        style.configure("Ghost.TButton", font=("Segoe UI", 10))

        style.configure("Card.TFrame", background="white", relief="solid", borderwidth=1)
        style.configure("CardTitle.TLabel", background="white", foreground="#6C7A89", font=("Segoe UI", 9))
        style.configure("CardValue.TLabel", background="white", foreground="#0D2033", font=("Segoe UI", 16, "bold"))

        # Treeview style
        style.configure("Log.Treeview",
                        background=self.COLOR_DARK_TABLE,
                        fieldbackground=self.COLOR_DARK_TABLE,
                        foreground="white",
                        rowheight=26,
                        borderwidth=0,
                        font=("Consolas", 10))
        style.configure("Log.Treeview.Heading",
                        background="#123A5C",
                        foreground="white",
                        font=("Segoe UI", 10, "bold"))
        style.map("Log.Treeview.Heading", background=[("active", "#123A5C")])

    # -------------------------
    # Header
    # -------------------------
    def _build_header(self):
        header = ttk.Frame(self, style="TFrame")
        header.pack(fill="x")

        # Use a normal tk.Frame for solid background color
        nav = tk.Frame(header, bg=self.COLOR_NAV, height=70)
        nav.pack(fill="x")

        left = tk.Frame(nav, bg=self.COLOR_NAV)
        left.pack(side="left", fill="y", padx=18)

        tk.Label(left, text="ATC TOWER", bg=self.COLOR_NAV, fg="white",
                 font=("Segoe UI", 22, "bold")).pack(anchor="w", pady=(12, 0))
        tk.Label(left, text="Air Traffic Control Server", bg=self.COLOR_NAV, fg="#CFE0FF",
                 font=("Segoe UI", 11)).pack(anchor="w", pady=(0, 12))

        right = tk.Frame(nav, bg=self.COLOR_NAV)
        right.pack(side="right", fill="y", padx=18)

        self.radar_btn = tk.Button(right, text="RADAR LINK", bg=self.COLOR_ACCENT, fg="black",
                                   font=("Segoe UI", 10, "bold"), relief="flat",
                                   padx=14, pady=8, command=self._radar_link)
        self.radar_btn.pack(anchor="e", pady=16)

    def _radar_link(self):
        messagebox.showinfo("Radar Link", "Radar Link is a UI placeholder for now.")

    # -------------------------
    # Controls row (port + status + buttons)
    # -------------------------
    def _build_controls(self):
        container = ttk.Frame(self)
        container.pack(fill="x", padx=18, pady=14)

        # Port card
        card = ttk.Frame(container, style="Card.TFrame")
        card.pack(side="left", fill="x", expand=True, padx=(0, 12))

        ttk.Label(card, text="Runway Port", style="CardTitle.TLabel").pack(anchor="w", padx=14, pady=(10, 0))

        self.port_var = tk.StringVar(value="54000")
        self.port_value = ttk.Label(card, textvariable=self.port_var, style="CardValue.TLabel")
        self.port_value.pack(anchor="w", padx=14, pady=(0, 10))

        # Status badge (right)
        self.status_badge = tk.Label(container, text="Tower Active", bg=self.COLOR_SUCCESS, fg="#134B20",
                                     font=("Segoe UI", 10, "bold"), padx=18, pady=10)
        self.status_badge.pack(side="right")

        # Buttons row under card
        button_row = ttk.Frame(self)
        button_row.pack(fill="x", padx=18, pady=(0, 8))

        self.tower_btn = tk.Button(button_row, text="Tower Standby", bg="#F57C00", fg="white",
                                   font=("Segoe UI", 11, "bold"), relief="flat",
                                   padx=18, pady=10, command=self._toggle_tower)
        self.tower_btn.pack(side="left")

        self.clear_btn = tk.Button(button_row, text="Clear Log", bg="#E6E6E6", fg="black",
                                   font=("Segoe UI", 11, "bold"), relief="flat",
                                   padx=18, pady=10, command=self._clear_log)
        self.clear_btn.pack(side="left", padx=(12, 0))

        ttk.Label(button_row, text="Passive tower: accepts and logs incoming data",
                  font=("Segoe UI", 10), foreground="#506479").pack(side="left", padx=18)

        # Start/Stop server buttons (top-right area)
        self.start_btn = tk.Button(button_row, text="Start Server", bg="#2D6CDF", fg="white",
                                   font=("Segoe UI", 11, "bold"), relief="flat",
                                   padx=18, pady=10, command=self._start_server)
        self.start_btn.pack(side="right")

        self.stop_btn = tk.Button(button_row, text="Stop Server", bg="#B0B7C3", fg="black",
                                  font=("Segoe UI", 11, "bold"), relief="flat",
                                  padx=18, pady=10, command=self._stop_server)
        self.stop_btn.pack(side="right", padx=(0, 12))

    def _toggle_tower(self):
        self.tower_active = not self.tower_active
        if self.tower_active:
            self.status_badge.config(text="Tower Active", bg=self.COLOR_SUCCESS, fg="#134B20")
            self.tower_btn.config(text="Tower Standby", bg="#F57C00", fg="white")
        else:
            self.status_badge.config(text="Tower Standby", bg="#FFE7D0", fg="#7A3D00")
            self.tower_btn.config(text="Tower Activate", bg="#2D6CDF", fg="white")

    # -------------------------
    # Log area (filter + table)
    # -------------------------
    def _build_log_area(self):
        wrapper = ttk.Frame(self)
        wrapper.pack(fill="both", expand=True, padx=18, pady=(4, 18))

        ttk.Label(wrapper, text="Flight Log", font=("Segoe UI", 11, "bold")).pack(anchor="w", pady=(0, 6))

        filter_row = ttk.Frame(wrapper)
        filter_row.pack(fill="x", pady=(0, 8))

        ttk.Label(filter_row, text="Filter").pack(side="left")

        self.filter_var = tk.StringVar(value="")
        self.filter_entry = ttk.Entry(filter_row, textvariable=self.filter_var, width=28)
        self.filter_entry.pack(side="left", padx=(8, 10))

        self.filter_col = tk.StringVar(value="LogID")
        self.filter_combo = ttk.Combobox(filter_row, textvariable=self.filter_col, width=14, state="readonly")
        self.filter_combo["values"] = ("LogID", "Client ID", "Flag", "Job Description", "TimeStamp", "Client Status", "Last Location")
        self.filter_combo.pack(side="left")

        # Table frame
        table_frame = tk.Frame(wrapper, bg=self.COLOR_DARK_TABLE, highlightthickness=1, highlightbackground="#123A5C")
        table_frame.pack(fill="both", expand=True)

        cols = ("LogID", "Client ID", "Flag", "Job Description", "TimeStamp", "Client Status", "Last Location")
        self.tree = ttk.Treeview(table_frame, columns=cols, show="headings", style="Log.Treeview")
        self.tree.pack(side="left", fill="both", expand=True)

        # headings
        for c in cols:
            self.tree.heading(c, text=c)
            self.tree.column(c, width=140, anchor="w")

        self.tree.column("LogID", width=70)
        self.tree.column("Flag", width=60)
        self.tree.column("Job Description", width=360)
        self.tree.column("TimeStamp", width=160)
        self.tree.column("Client Status", width=140)
        self.tree.column("Last Location", width=140)

        scrollbar = ttk.Scrollbar(table_frame, orient="vertical", command=self.tree.yview)
        scrollbar.pack(side="right", fill="y")
        self.tree.configure(yscrollcommand=scrollbar.set)

        # keep original rows for filtering
        self.all_rows = []

    # -------------------------
    # Server process control
    # -------------------------
    def _start_server(self):
        if self.server_process is not None:
            self._add_log("127.0.0.1", "Y", "Server already running", "Running", "127.0.0.1")
            return

        port = self.port_var.get().strip()
        if not port.isdigit():
            messagebox.showerror("Invalid Port", "Port must be a number.")
            return

        try:
            self.server_process = subprocess.Popen(
                [SERVER_EXE],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True
            )
        except Exception as e:
            messagebox.showerror("Start Server Failed", str(e))
            self.server_process = None
            return

        self._add_log("127.0.0.1", "Y", f"Server started on port {port}", "N/A", "127.0.0.1")

        t = threading.Thread(target=self._read_server_output, daemon=True)
        t.start()

    def _stop_server(self):
        if self.server_process is None:
            self._add_log("127.0.0.1", "Y", "Server not running", "N/A", "127.0.0.1")
            return

        self.server_process.terminate()
        self.server_process = None
        self._add_log("127.0.0.1", "Y", "Server stopped", "N/A", "127.0.0.1")

    def _read_server_output(self):
        """Reads server stdout and pushes it into the log table."""
        if self.server_process is None or self.server_process.stdout is None:
            return

        for line in self.server_process.stdout:
            line = line.strip()
            if not line:
                continue

            # Try to infer useful fields from text
            # You can tweak these patterns based on your server prints.
            client_id = "127.0.0.1"
            flag = "Y"
            status = "Running"
            last_loc = "127.0.0.1"

            if "Client connected" in line:
                flag = "G"
                status = "Connected"
            elif "Client disconnected" in line:
                flag = "Y"
                status = "Disconnected"
            elif "WEATHER_REQUEST" in line:
                flag = "Y"
                status = "Connected"
                # extract location if present
                m = re.search(r"WEATHER_REQUEST:\s*(.+)$", line)
                if m:
                    last_loc = m.group(1).strip()

            self._add_log(client_id, flag, line, status, last_loc)

        # if server exits
        self.server_process = None

    # -------------------------
    # Log helpers
    # -------------------------
    def _add_log(self, client_id, flag, job, status, last_location):
        self.log_id += 1
        ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        row = (str(self.log_id), client_id, flag, job, ts, status, last_location)

        # store full row list for filtering
        self.all_rows.append(row)

        # insert into tree
        self.tree.insert("", "end", values=row)

    def _clear_log(self):
        self.all_rows.clear()
        for item in self.tree.get_children():
            self.tree.delete(item)
        self.log_id = 0

    def _apply_filter(self):
        """Rebuild the tree based on filter input."""
        query = self.filter_var.get().strip().lower()
        col = self.filter_col.get()

        # Map column name to index
        col_index = {
            "LogID": 0,
            "Client ID": 1,
            "Flag": 2,
            "Job Description": 3,
            "TimeStamp": 4,
            "Client Status": 5,
            "Last Location": 6
        }.get(col, 0)

        # If nothing typed, do nothing heavy
        # (We still schedule next run)
        if query == "":
            # If tree already matches all_rows, don’t rebuild every time
            if len(self.tree.get_children()) != len(self.all_rows):
                self._rebuild_tree(self.all_rows)
            self.after(250, self._apply_filter)
            return

        filtered = []
        for r in self.all_rows:
            if query in str(r[col_index]).lower():
                filtered.append(r)

        self._rebuild_tree(filtered)
        self.after(250, self._apply_filter)

    def _rebuild_tree(self, rows):
        self.tree.delete(*self.tree.get_children())
        for r in rows:
            self.tree.insert("", "end", values=r)


if __name__ == "__main__":
    app = ATCTowerUI()
    app.mainloop()