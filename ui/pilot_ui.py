import tkinter as tk
from tkinter import ttk
from tkinter import scrolledtext
import subprocess
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(BASE_DIR)
CLIENT_EXE = os.path.join(PROJECT_ROOT, "build", "client.exe")


class PilotClientUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Pilot Client Console")
        self.geometry("1000x650")
        self.minsize(900, 600)

        self._setup_styles()
        self._build_header()
        self._build_controls()
        self._build_output()

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
        self.COLOR_DARK = "#0B1E33"
        self.COLOR_WHITE = "#FFFFFF"
        self.COLOR_MUTED = "#5D6D7E"

        self.configure(bg=self.COLOR_BG)

        style.configure("TFrame", background=self.COLOR_BG)
        style.configure("TLabel", background=self.COLOR_BG, foreground="#0D2033", font=("Segoe UI", 10))
        style.configure("Card.TFrame", background="white", relief="solid", borderwidth=1)
        style.configure("CardTitle.TLabel", background="white", foreground="#6C7A89", font=("Segoe UI", 9))
        style.configure("CardValue.TLabel", background="white", foreground="#0D2033", font=("Segoe UI", 16, "bold"))

    # -------------------------
    # Header
    # -------------------------
    def _build_header(self):
        header = tk.Frame(self, bg=self.COLOR_NAV, height=85)
        header.pack(fill="x")

        left = tk.Frame(header, bg=self.COLOR_NAV)
        left.pack(side="left", fill="y", padx=18)

        tk.Label(
            left,
            text="PILOT CONSOLE",
            bg=self.COLOR_NAV,
            fg="white",
            font=("Segoe UI", 22, "bold")
        ).pack(anchor="w", pady=(14, 0))

        tk.Label(
            left,
            text="Aircraft Client Interface",
            bg=self.COLOR_NAV,
            fg="#CFE0FF",
            font=("Segoe UI", 11)
        ).pack(anchor="w", pady=(0, 14))

        right = tk.Frame(header, bg=self.COLOR_NAV)
        right.pack(side="right", fill="y", padx=18)

        self.status_badge = tk.Label(
            right,
            text="Client Ready",
            bg=self.COLOR_SUCCESS,
            fg="#134B20",
            font=("Segoe UI", 10, "bold"),
            padx=18,
            pady=10
        )
        self.status_badge.pack(anchor="e", pady=20)

    # -------------------------
    # Controls
    # -------------------------
    def _build_controls(self):
        container = ttk.Frame(self)
        container.pack(fill="x", padx=18, pady=16)

        # Left main card
        left_card = ttk.Frame(container, style="Card.TFrame")
        left_card.pack(side="left", fill="both", expand=True, padx=(0, 12))

        title_row = tk.Frame(left_card, bg="white")
        title_row.pack(fill="x", padx=14, pady=(10, 0))

        tk.Label(
            title_row,
            text="Weather Request Panel",
            bg="white",
            fg="#0D2033",
            font=("Segoe UI", 13, "bold")
        ).pack(anchor="w")

        tk.Label(
            left_card,
            text="Enter the destination/location to request current weather data from the ATC server.",
            bg="white",
            fg=self.COLOR_MUTED,
            font=("Segoe UI", 10),
            wraplength=620,
            justify="left"
        ).pack(anchor="w", padx=14, pady=(4, 10))

        form = tk.Frame(left_card, bg="white")
        form.pack(fill="x", padx=14, pady=(0, 14))

        tk.Label(form, text="Location", bg="white", fg="#0D2033", font=("Segoe UI", 10, "bold")).grid(row=0, column=0, sticky="w", pady=6)
        self.loc_var = tk.StringVar(value="WATERLOO")
        self.loc_entry = tk.Entry(form, textvariable=self.loc_var, width=30, font=("Segoe UI", 11), relief="solid", bd=1)
        self.loc_entry.grid(row=0, column=1, sticky="w", padx=12, pady=6)

        self.request_btn = tk.Button(
            form,
            text="Request Weather",
            bg=self.COLOR_ACCENT,
            fg="black",
            font=("Segoe UI", 11, "bold"),
            relief="flat",
            padx=18,
            pady=10,
            command=self.request_weather
        )
        self.request_btn.grid(row=1, column=0, columnspan=2, sticky="w", pady=(12, 0))

        # Right info card
        right_card = ttk.Frame(container, style="Card.TFrame")
        right_card.pack(side="right", fill="y")

        tk.Label(
            right_card,
            text="Connection Info",
            bg="white",
            fg="#0D2033",
            font=("Segoe UI", 12, "bold")
        ).pack(anchor="w", padx=14, pady=(12, 8))

        self._info_row(right_card, "Server IP", "127.0.0.1")
        self._info_row(right_card, "Port", "54000")
        self._info_row(right_card, "Mode", "Weather Request")
        self._info_row(right_card, "Protocol", "TCP Packet")

        tk.Button(
            right_card,
            text="Clear Output",
            bg="#E6E6E6",
            fg="black",
            font=("Segoe UI", 10, "bold"),
            relief="flat",
            padx=14,
            pady=8,
            command=self.clear_output
        ).pack(anchor="w", padx=14, pady=(12, 14))

    def _info_row(self, parent, label, value):
        row = tk.Frame(parent, bg="white")
        row.pack(fill="x", padx=14, pady=4)

        tk.Label(
            row,
            text=f"{label}:",
            bg="white",
            fg="#6C7A89",
            font=("Segoe UI", 10)
        ).pack(side="left")

        tk.Label(
            row,
            text=value,
            bg="white",
            fg="#0D2033",
            font=("Segoe UI", 10, "bold")
        ).pack(side="left", padx=(8, 0))

    # -------------------------
    # Output
    # -------------------------
    def _build_output(self):
        wrapper = ttk.Frame(self)
        wrapper.pack(fill="both", expand=True, padx=18, pady=(0, 18))

        tk.Label(
            wrapper,
            text="Response Console",
            bg=self.COLOR_BG,
            fg="#0D2033",
            font=("Segoe UI", 12, "bold")
        ).pack(anchor="w", pady=(0, 6))

        console_frame = tk.Frame(
            wrapper,
            bg=self.COLOR_DARK,
            highlightthickness=1,
            highlightbackground="#123A5C"
        )
        console_frame.pack(fill="both", expand=True)

        self.output = scrolledtext.ScrolledText(
            console_frame,
            width=90,
            height=20,
            bg=self.COLOR_DARK,
            fg="white",
            insertbackground="white",
            font=("Consolas", 11),
            relief="flat",
            wrap="word"
        )
        self.output.pack(fill="both", expand=True, padx=8, pady=8)

        self.log("Pilot client UI ready.\n")

    # -------------------------
    # Actions
    # -------------------------
    def log(self, msg):
        self.output.insert(tk.END, msg)
        self.output.see(tk.END)

    def clear_output(self):
        self.output.delete("1.0", tk.END)

    def request_weather(self):
        location = self.loc_var.get().strip()

        if not location:
            self.log("Please enter a location.\n")
            return

        self.status_badge.config(text="Requesting...", bg="#FFF2CC", fg="#7A5A00")
        self.log(f"> Requesting weather for {location}\n")

        try:
            result = subprocess.run(
                [CLIENT_EXE],
                input=location + "\n",
                capture_output=True,
                text=True,
                cwd=PROJECT_ROOT
            )

            if result.stdout:
                self.log(result.stdout + "\n")

            if result.stderr:
                self.log("ERROR:\n" + result.stderr + "\n")

            self.status_badge.config(text="Response Received", bg=self.COLOR_SUCCESS, fg="#134B20")

        except Exception as e:
            self.log(f"Failed to run client.exe: {e}\n")
            self.status_badge.config(text="Request Failed", bg="#FFD9D9", fg="#8A1F1F")


if __name__ == "__main__":
    app = PilotClientUI()
    app.mainloop()