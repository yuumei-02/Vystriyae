import ctypes
import os

vystriyae_lib = None

class System:
   def __init__(self, dll_path: str) -> None:
      global vystriyae_lib
      vystriyae_lib = ctypes.CDLL(dll_path)

   @staticmethod
   def init() -> None:
      vystriyae_lib.System_init()

   @staticmethod
   def run() -> None:
      vystriyae_lib.System_run()

   @staticmethod
   def close() -> None:
      vystriyae_lib.System_close()

def set_target_fps(fps: int) -> None:
   vystriyae_lib.set_target_fps(ctypes.c_int(fps))

def get_frame_count() -> int:
   vystriyae_lib.get_frame_count.restype = ctypes.c_int
   return vystriyae_lib.get_frame_count()

def get_target_fps() -> int:
   vystriyae_lib.get_target_fps.restype = ctypes.c_int
   return vystriyae_lib.get_target_fps()

def get_fps() -> float:
   vystriyae_lib.get_fps.restype = ctypes.c_float
   return vystriyae_lib.get_fps()

class Layer:
   def on_create(self) -> None:
      assert False, "Missing implementation for on_create"

   def on_destroy(self) -> None:
      assert False, "Missing implementation for on_destroy"

   def on_input(self) -> None:
      assert False, "Missing implementation for on_input"

   def on_render(self) -> None:
      assert False, "Missing implementation for on_render"

# typedef void* (*LayerOnCreate)();
# typedef void  (*LayerOnDestroy)(void* context);
# typedef void  (*LayerOnInput)(Vector* InputEvent_queue, RenderRegion* region, void* context);
# typedef void  (*LayerOnRender)(CharMapSlice charmap, void* context);
