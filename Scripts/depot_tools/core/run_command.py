import subprocess
import sys
import threading
def run_command(command):
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,  # 行缓冲
        universal_newlines=True
    )
    
    # 创建线程来读取 stdout 和 stderr
    def read_stdout():
        for line in iter(process.stdout.readline, ''):
            print(f"[STDOUT] {line.rstrip()}")
        process.stdout.close()
    
    def read_stderr():
        for line in iter(process.stderr.readline, ''):
            print(f"[STDERR] {line.rstrip()}", file=sys.stderr)
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

    


