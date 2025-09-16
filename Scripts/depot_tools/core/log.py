def error(msg):
    # red print
    print(f"\033[91m[ERROR] {msg}\033[0m")

def info(msg):
    # green print
    print(f"\033[92m[INFO] {msg}\033[0m")

def warning(msg):
    # yellow print
    print(f"\033[93m[WARNING] {msg}\033[0m")
def debug(msg):
    # blue print
    print(f"\033[94m[DEBUG] {msg}\033[0m")