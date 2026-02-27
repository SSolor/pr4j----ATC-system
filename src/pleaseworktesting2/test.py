import ctypes
import os
import threading
import time
import platform

# Load the C++ DLL
# dll_path = os.path.join(os.path.dirname(__file__), "test.dll")
# tststlib = ctypes.CDLL(dll_path)
tststlib = ctypes.CDLL('./test.dll')

# Define function signatures
tststlib.numsms.restype = ctypes.c_int;


if __name__ == "__main__":
      x = tststlib.numsms()
      print(x)