class Color:
    BLACK = [0,0,0]
    WHITE = [255,255,255]
    RED = [255,0,0]
    GREEN = [0,255,0]
    BLUE = [0,0,255]
    YELLOW = [255,255,0]
    PURPLE = [255,0,255]
    CYAN = [0,255,255]
    GRAY = [128,128,128]
    LIGHT_GRAY = [192,192,192]
    LIGHT_RED = [255,128,128]
    LIGHT_GREEN = [128,255,128]
    LIGHT_BLUE = [128,128,255]
    LIGHT_YELLOW = [255,255,128]
    LIGHT_PURPLE = [255,128,255]
    LIGHT_CYAN = [128,255,255]
class Console:
    @staticmethod
    def clear_screen():
        print("\033[2J",end="",flush=True)
    @staticmethod
    def line_up(n:int):
        print(f"\033[{n}A",end="",flush=True)
    @staticmethod
    def line_down(n:int):
        print(f"\r\n",end="",flush=True)
    @staticmethod
    def set_font_color(r,g,b):
        print(f"\033[38;2;{r};{g};{b}m",end="",flush=True)
    @staticmethod
    def set_background_color(r,g,b):
        print(f"\033[48;2;{r};{g};{b}m",end="",flush=True)
    @staticmethod
    def reset_color():
        print("\033[0m",end="",flush=True)
    @staticmethod
    def print_with_color(text:str,font_color:list[int],background_color:list[int]):
        if font_color!=None:
            Console.set_font_color(font_color[0],font_color[1],font_color[2])
        if background_color!=None:
            Console.set_background_color(background_color[0],background_color[1],background_color[2])
        print(text,end="",flush=True)
        Console.reset_color()

    