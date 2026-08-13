import vystriyae

def main():
   system = vystriyae.System("./build/lib/libvystriyae.so")
   system.init()
   system.run()
   system.close()

if __name__ == "__main__":
   main()