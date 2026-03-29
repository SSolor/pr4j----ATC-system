import tkinter as tk
from tkinter import ttk
import tkinter.font as tkFont
import subprocess
import threading
import os
from datetime import datetime
import queue
import time

class ATCScreen:
    # Professional Color Palette
    PRIMARY_BG = "#0f1419"      # Deep navy background
    SECONDARY_BG = "#1a1f2e"    # Slightly lighter for cards
    ACCENT_BLUE = "#00d4ff"     # Bright cyan accent
    ACCENT_GREEN = "#00ff85"    # Fresh green for active
    ACCENT_ORANGE = "#ff6b35"   # Warning orange
    TEXT_PRIMARY = "#ffffff"    # White text
    TEXT_SECONDARY = "#a0aec0"  # Muted gray text
    BORDER_COLOR = "#2d3748"    # Subtle borders
    SERVER_PORT = 54000
    
    def __init__(self, root):
        self.root = root
        self.root.title("ATC Tower Control Center")
        self.root.geometry("1400x850")
        self.root.configure(bg=self.PRIMARY_BG)
        
        # Thread-safe queue for server output
        self.log_queue = queue.Queue()
        self.connection_queue = queue.Queue()
        
        # Server process and tracking
        self.server_process = None
        self.server_running = False
        self.active_connections = {}
        self.server_start_time = None
        self.metric_values = {}
        
        # Configure styles
        self.setup_styles()
        
        # Header Section
        self.create_header()
        
        # Main Content Area
        self.create_main_content()
        
        # Footer Section
        self.create_footer()
        
        # Start the C++ server
        self.start_server()
        
        # Start UI update thread
        self.start_ui_update_thread()
        
        # Start polling for server updates
        self.poll_server_output()
        self.poll_connections()
        
        # Cleanup on exit
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
    
    def start_server(self):
        """Launch the C++ server executable"""
        try:
            # Get the build directory
            current_dir = os.path.dirname(os.path.abspath(__file__))
            build_dir = os.path.join(os.path.dirname(current_dir), "build")
            server_exe = os.path.join(build_dir, "server.exe")
            
            if not os.path.exists(server_exe):
                self.add_log(f"❌ ERROR: Server executable not found at {server_exe}")
                return
            
            # Start the server process
            self.server_process = subprocess.Popen(
                [server_exe],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
            
            self.server_running = True
            self.server_start_time = datetime.now()
            self.add_log("✓ ATC Server Engine Started")
            
            # Start thread to read server output
            self.server_thread = threading.Thread(target=self.read_server_output, daemon=True)
            self.server_thread.start()
            
        except Exception as e:
            self.add_log(f"❌ Failed to start server: {str(e)}")
            self.server_running = False
    
    def read_server_output(self):
        """Read server output in a separate thread"""
        try:
            while self.server_running and self.server_process:
                line = self.server_process.stdout.readline()
                if line:
                    clean_line = line.strip()
                    self.log_queue.put(clean_line)
                    
                    # Parse connection info
                    lower_line = clean_line.lower()
                    if "client connected" in lower_line:
                        self.connection_queue.put(("connect", "New Aircraft"))
                    elif "client disconnected" in lower_line:
                        self.connection_queue.put(("disconnect", None))
        except Exception as e:
            self.log_queue.put(f"❌ Error reading server output: {str(e)}")
    
    def start_ui_update_thread(self):
        """Start thread for periodic UI updates"""
        def update_uptime():
            while self.server_running:
                time.sleep(1)
                if self.server_start_time:
                    uptime = datetime.now() - self.server_start_time
                    hours = int(uptime.total_seconds() // 3600)
                    minutes = int((uptime.total_seconds() % 3600) // 60)
                    seconds = int(uptime.total_seconds() % 60)
                    self.root.after(0, lambda h=hours, m=minutes, s=seconds: self.set_metric("SYSTEM UPTIME", f"{h:02}:{m:02}:{s:02}"))
        
        uptime_thread = threading.Thread(target=update_uptime, daemon=True)
        uptime_thread.start()
    
    def poll_server_output(self):
        """Poll for server output and update UI"""
        if self.server_process and self.server_running and self.server_process.poll() is not None:
            self.server_running = False
            self.add_log("❌ Server process exited")
            self.status_text.config(text="● System: Offline", fg=self.ACCENT_ORANGE)

        try:
            # Process log queue
            while not self.log_queue.empty():
                log_msg = self.log_queue.get_nowait()
                self.add_log(log_msg)
                self.update_last_activity()
        except queue.Empty:
            pass
        
        try:
            # Process connection queue
            while not self.connection_queue.empty():
                action, data = self.connection_queue.get_nowait()
                if action == "connect":
                    self.add_connection(data)
                elif action == "disconnect":
                    self.remove_connection()
        except queue.Empty:
            pass
        
        # Schedule next poll (every 100ms)
        if self.server_running:
            self.root.after(100, self.poll_server_output)

    def poll_connections(self):
        """Poll active TCP connections for server port and refresh UI list."""
        if not self.server_running or not self.server_process:
            return

        try:
            result = subprocess.run(
                ["netstat", "-ano", "-p", "tcp"],
                capture_output=True,
                text=True,
                check=False
            )

            endpoints = []
            server_pid = str(self.server_process.pid)
            local_suffix = f":{self.SERVER_PORT}"

            for line in result.stdout.splitlines():
                parts = line.split()
                if len(parts) < 5 or parts[0] != "TCP":
                    continue

                local_addr, remote_addr, state, pid = parts[1], parts[2], parts[3], parts[4]
                if state == "ESTABLISHED" and local_addr.endswith(local_suffix) and pid == server_pid:
                    endpoints.append(remote_addr)

            self.refresh_connections(sorted(set(endpoints)))
        except Exception as e:
            self.add_log(f"⚠ Connection poll failed: {str(e)}")

        if self.server_running:
            self.root.after(1000, self.poll_connections)

    def refresh_connections(self, endpoints):
        """Render connection list from currently established remote endpoints."""
        self.aircraft_list.delete(0, tk.END)
        self.active_connections.clear()

        if not endpoints:
            self.aircraft_list.insert(tk.END, "No active connections")
            self.set_metric("ACTIVE CONNECTIONS", "0")
            return

        for idx, endpoint in enumerate(endpoints):
            label = f"Aircraft {idx + 1}  {endpoint}"
            self.aircraft_list.insert(tk.END, label)
            self.active_connections[idx] = label

        self.set_metric("ACTIVE CONNECTIONS", str(len(endpoints)))
    
    def add_connection(self, name):
        """Add a connection to the aircraft list"""
        if "No active connections" in self.aircraft_list.get(0, tk.END):
            self.aircraft_list.delete(0, tk.END)
        
        timestamp = datetime.now().strftime("%H:%M:%S")
        connection_id = len(self.aircraft_list.get(0, tk.END))
        connection_label = f"[{timestamp}] {name}"
        self.aircraft_list.insert(tk.END, connection_label)
        self.active_connections[connection_id] = connection_label
    
    def remove_connection(self):
        """Remove a connection from the aircraft list"""
        if self.aircraft_list.size() > 0:
            self.aircraft_list.delete(self.aircraft_list.size() - 1)
        
        if self.aircraft_list.size() == 0:
            self.aircraft_list.insert(tk.END, "No active connections")
    
    def on_closing(self):
        """Cleanup and exit"""
        self.server_running = False
        if self.server_process:
            try:
                self.server_process.terminate()
                self.server_process.wait(timeout=2)
            except:
                self.server_process.kill()
        self.root.destroy()
    
    def setup_styles(self):
        """Configure modern color scheme and fonts"""
        style = ttk.Style()
        style.theme_use('clam')
        
        # Configure colors
        style.configure('TFrame', background=self.PRIMARY_BG)
        style.configure('TLabel', background=self.PRIMARY_BG, foreground=self.TEXT_PRIMARY)
        style.configure('Accent.TLabel', background=self.PRIMARY_BG, foreground=self.ACCENT_BLUE)
        style.configure('Horizontal.TProgressbar', background=self.ACCENT_GREEN, troughcolor=self.SECONDARY_BG)
        
        # Font definitions
        self.font_title = tkFont.Font(family="Segoe UI", size=24, weight="bold")
        self.font_subtitle = tkFont.Font(family="Segoe UI", size=11, weight="bold")
        self.font_normal = tkFont.Font(family="Segoe UI", size=10)
        self.font_small = tkFont.Font(family="Segoe UI", size=9)
        self.font_mono = tkFont.Font(family="Consolas", size=9)
    
    def create_header(self):
        """Create professional header with system state"""
        header_frame = tk.Frame(self.root, bg=self.SECONDARY_BG, height=80)
        header_frame.pack(fill="x", padx=0, pady=0)
        header_frame.pack_propagate(False)
        
        # Add subtle border
        border = tk.Frame(header_frame, bg=self.BORDER_COLOR, height=1)
        border.pack(fill="x", side="bottom")
        
        content_frame = tk.Frame(header_frame, bg=self.SECONDARY_BG)
        content_frame.pack(expand=True, fill="both", padx=25, pady=15)
        
        # Left side: Title
        left_frame = tk.Frame(content_frame, bg=self.SECONDARY_BG)
        left_frame.pack(side="left", fill="both", expand=True)
        
        title_label = tk.Label(left_frame, text="ATC TOWER", fg=self.ACCENT_BLUE, 
                              bg=self.SECONDARY_BG, font=self.font_title)
        title_label.pack(anchor="w")
        
        subtitle_label = tk.Label(left_frame, text="Control Center Dashboard", 
                                 fg=self.TEXT_SECONDARY, bg=self.SECONDARY_BG, 
                                 font=tkFont.Font(family="Segoe UI", size=10))
        subtitle_label.pack(anchor="w")
        
        # Right side: System State Indicator
        right_frame = tk.Frame(content_frame, bg=self.SECONDARY_BG)
        right_frame.pack(side="right", fill="y", padx=20)
        
        state_indicator_frame = tk.Frame(right_frame, bg=self.SECONDARY_BG)
        state_indicator_frame.pack(anchor="e")
        
        state_label_text = tk.Label(state_indicator_frame, text="SYSTEM STATE", 
                                   fg=self.TEXT_SECONDARY, bg=self.SECONDARY_BG, 
                                   font=self.font_small)
        state_label_text.pack(anchor="e")
        
        self.state_label = tk.Label(state_indicator_frame, text="● STARTUP", 
                                   fg=self.ACCENT_GREEN, bg=self.SECONDARY_BG, 
                                   font=self.font_subtitle)
        self.state_label.pack(anchor="e", pady=(5, 0))
    
    def create_main_content(self):
        """Create the main content area with three sections"""
        main_frame = tk.Frame(self.root, bg=self.PRIMARY_BG)
        main_frame.pack(expand=True, fill="both", padx=20, pady=20)
        
        # Top row: Stats/Metrics
        self.create_metrics_row(main_frame)
        
        # Middle row: Aircraft List and Log (side by side)
        content_row = tk.Frame(main_frame, bg=self.PRIMARY_BG)
        content_row.pack(expand=True, fill="both", pady=(20, 0))
        
        self.create_aircraft_section(content_row)
        self.create_log_section(content_row)
        
        # Bottom: Progress Bar
        self.create_progress_section(main_frame)
    
    def create_metrics_row(self, parent):
        """Create metric cards at the top"""
        metrics_frame = tk.Frame(parent, bg=self.PRIMARY_BG)
        metrics_frame.pack(fill="x", pady=(0, 20))
        
        # Metric card 1
        self.create_metric_card(metrics_frame, "ACTIVE CONNECTIONS", "0", 0)
        
        # Metric card 2
        self.create_metric_card(metrics_frame, "SYSTEM UPTIME", "00:00:00", 1)
        
        # Metric card 3
        self.create_metric_card(metrics_frame, "DATA TRANSFERRED", "0 MB", 2)
        
        # Metric card 4
        self.create_metric_card(metrics_frame, "LAST ACTIVITY", "--:--:--", 3)
    
    def create_metric_card(self, parent, label, value, position):
        """Create a single metric card"""
        card = tk.Frame(parent, bg=self.SECONDARY_BG)
        card.pack(side="left", fill="both", expand=True, padx=7.5)
        
        # Top border accent
        border_top = tk.Frame(card, bg=self.ACCENT_BLUE, height=3)
        border_top.pack(fill="x")
        
        content = tk.Frame(card, bg=self.SECONDARY_BG)
        content.pack(fill="both", expand=True, padx=15, pady=15)
        
        label_widget = tk.Label(content, text=label, fg=self.TEXT_SECONDARY, 
                               bg=self.SECONDARY_BG, font=self.font_small)
        label_widget.pack(anchor="w")
        
        value_widget = tk.Label(content, text=value, fg=self.ACCENT_BLUE, 
                               bg=self.SECONDARY_BG, font=self.font_subtitle)
        value_widget.pack(anchor="w", pady=(5, 0))
        self.metric_values[label] = value_widget
    
    def create_aircraft_section(self, parent):
        """Create active aircraft panel"""
        aircraft_frame = tk.Frame(parent, bg=self.SECONDARY_BG, width=300)
        aircraft_frame.pack_propagate(False)
        aircraft_frame.pack(side="left", fill="both", padx=(0, 15), expand=False)
        
        # Header with icon
        header = tk.Frame(aircraft_frame, bg=self.SECONDARY_BG)
        header.pack(fill="x", padx=20, pady=20)
        
        title = tk.Label(header, text="✈ ACTIVE AIRCRAFT", fg=self.ACCENT_BLUE, 
                        bg=self.SECONDARY_BG, font=self.font_subtitle)
        title.pack(anchor="w")
        
        # Aircraft list
        self.aircraft_list = tk.Listbox(aircraft_frame, bg=self.PRIMARY_BG, 
                                        fg=self.TEXT_PRIMARY, font=self.font_mono,
                                        bd=0, highlightthickness=0, 
                                        selectmode="extended", height=25)
        self.aircraft_list.pack(fill="both", expand=True, padx=20, pady=(0, 20))
        self.aircraft_list.insert(tk.END, "No active connections")
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(aircraft_frame, orient="vertical", command=self.aircraft_list.yview)
        scrollbar.pack(side="right", fill="y", padx=(0, 20), pady=(0, 20))
        self.aircraft_list.config(yscrollcommand=scrollbar.set)
    
    def create_log_section(self, parent):
        """Create audit log panel"""
        log_frame = tk.Frame(parent, bg=self.SECONDARY_BG)
        log_frame.pack(side="right", fill="both", expand=True)
        
        # Header
        header = tk.Frame(log_frame, bg=self.SECONDARY_BG)
        header.pack(fill="x", padx=20, pady=20)
        
        title = tk.Label(header, text="📋 SYSTEM LOG", fg=self.ACCENT_BLUE, 
                        bg=self.SECONDARY_BG, font=self.font_subtitle)
        title.pack(anchor="w")
        
        # Log text with professional styling
        self.log_text = tk.Text(log_frame, state="disabled", bg=self.PRIMARY_BG, 
                               fg=self.ACCENT_GREEN, font=self.font_mono,
                               bd=0, highlightthickness=0, wrap="word")
        self.log_text.pack(fill="both", expand=True, padx=20, pady=(0, 20))
        
        # Scrollbar for log
        scrollbar = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        scrollbar.pack(side="right", fill="y", padx=(0, 20), pady=(0, 20))
        self.log_text.config(yscrollcommand=scrollbar.set)
    
    def create_progress_section(self, parent):
        """Create file transfer progress section"""
        progress_frame = tk.Frame(parent, bg=self.SECONDARY_BG)
        progress_frame.pack(fill="x", pady=(20, 0))
        
        # Border
        border_top = tk.Frame(progress_frame, bg=self.BORDER_COLOR, height=1)
        border_top.pack(fill="x")
        
        content = tk.Frame(progress_frame, bg=self.SECONDARY_BG)
        content.pack(fill="x", padx=20, pady=15)
        
        label_frame = tk.Frame(content, bg=self.SECONDARY_BG)
        label_frame.pack(fill="x", pady=(0, 10))
        
        header_row = tk.Frame(label_frame, bg=self.SECONDARY_BG)
        header_row.pack(fill="x")
        
        self.progress_label = tk.Label(header_row, text="📁 File Transfer: Idle", 
                                      fg=self.ACCENT_BLUE, bg=self.SECONDARY_BG, 
                                      font=self.font_subtitle)
        self.progress_label.pack(anchor="w", side="left")
        
        self.progress_percent = tk.Label(header_row, text="0%", 
                                        fg=self.TEXT_SECONDARY, bg=self.SECONDARY_BG, 
                                        font=self.font_small)
        self.progress_percent.pack(anchor="e", side="right")
        
        bar_frame = tk.Frame(content, bg=self.PRIMARY_BG, height=8)
        bar_frame.pack(fill="x", pady=(10, 0))
        
        self.progress = ttk.Progressbar(bar_frame, orient="horizontal", 
                                        mode="determinate", length=500)
        self.progress.pack(fill="x", expand=True)
    
    def create_footer(self):
        """Create professional footer"""
        footer_frame = tk.Frame(self.root, bg=self.SECONDARY_BG, height=60)
        footer_frame.pack(fill="x", side="bottom")
        footer_frame.pack_propagate(False)
        
        # Top border
        border = tk.Frame(footer_frame, bg=self.BORDER_COLOR, height=1)
        border.pack(fill="x", side="top")
        
        content = tk.Frame(footer_frame, bg=self.SECONDARY_BG)
        content.pack(expand=True, fill="both", padx=25, pady=12)
        
        # Status indicators
        status_frame = tk.Frame(content, bg=self.SECONDARY_BG)
        status_frame.pack(side="left")
        
        self.status_text = tk.Label(status_frame, text="● System: Online", 
                                   fg=self.ACCENT_GREEN, bg=self.SECONDARY_BG, 
                                   font=self.font_small)
        self.status_text.pack(anchor="w")
        
        # Copyright
        copyright_label = tk.Label(content, text="ATC Control System v1.0 © 2026", 
                                  fg=self.TEXT_SECONDARY, bg=self.SECONDARY_BG, 
                                  font=self.font_small)
        copyright_label.pack(side="right", anchor="e")
    
    def add_log(self, message):
        """Adds a line to the Audit Log (REQ-LOG-010)"""
        self.log_text.config(state="normal")
        self.log_text.insert(tk.END, f"→ {message}\n")
        self.log_text.see(tk.END)
        self.log_text.config(state="disabled")
        self.root.update_idletasks()

    def set_metric(self, metric_name, value):
        """Update a metric card value by label."""
        label = self.metric_values.get(metric_name)
        if label:
            label.config(text=value)

    def update_last_activity(self):
        """Update the last activity metric to current time."""
        self.set_metric("LAST ACTIVITY", datetime.now().strftime("%H:%M:%S"))
    
    def update_progress(self, current, total):
        """Updates the progress bar for REQ-SYS-070 transfers"""
        if total == 0:
            percent = 0
        else:
            percent = (current / total) * 100
        
        self.progress['value'] = percent
        self.progress_label.config(text=f"📁 File Transfer: {int(percent)}%")
        self.progress_percent.config(text=f"{int(percent)}%")
        self.root.update_idletasks()
    
    def set_system_state(self, state):
        """Update the system state display"""
        self.state_label.config(text=f"● {state}")

if __name__ == "__main__":
    root = tk.Tk()
    app = ATCScreen(root)
    # Manual update for testing
    app.add_log("Waiting for incoming connections...")
    root.mainloop()