import ctypes
import os

vystriyae_lib = None

class System:
   def __init__(self, dll_path: str):
      global vystriyae_lib
      vystriyae_lib = ctypes.CDLL(dll_path)

   @staticmethod
   def init():
      vystriyae_lib.System_init()

   @staticmethod
   def run():
      vystriyae_lib.System_run()

   @staticmethod
   def close():
      vystriyae_lib.System_close()
