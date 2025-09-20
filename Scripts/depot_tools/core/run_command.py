import subprocess
import sys
import threading
def read_line(f):
    line=b""
    while f.readable():
        c=f.read(1)
        if c==b'\n' or c==b'':
            break
        line+=c
    return line
def run_command(command):
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=False,
        bufsize=1,  # 行缓冲
        universal_newlines=False
    )
    
    # 创建线程来读取 stdout 和 stderr
    def read_stdout():
        line =read_line(process.stdout)
        while line!=b"":
            
            u8line=None
            try:
                u8line=line.decode('utf-8')
            except:
                pass
            if u8line is not None:
                print(f"[STDOUT] {u8line}")
            else:
                print(f"[STDOUT] {line}")
            line =read_line(process.stdout)
        process.stdout.close()
    
    def read_stderr():
        line=read_line(process.stderr)
        while line!=b"":
            print(f"[STDERR] {line.rstrip().decode('utf-8')}", file=sys.stderr)
            u8line=None
            try:
                u8line=line.decode('utf-8')
            except:
                pass
            if u8line is not None:
                print(f"[STDERR] {u8line}")
            else:
                print(f"[STDERR] {line}")
            line=read_line(process.stderr)
        process.stderr.close()
    
    # 启动读取线程
    stdout_thread = threading.Thread(target=read_stdout)
    stderr_thread = threading.Thread(target=read_stderr)
    
    stdout_thread.start()
    stderr_thread.start()
    
    # 等待进程结束
    process.wait()
    
    # 等待线程结束
    stdout_thread.join()
    stderr_thread.join()
    
    return process.returncode

    


