from Console import Console


class Log:
    @staticmethod
    def Info(text:str):
        text = "[Info] " + text+"\n"
        Console.print_with_color(text,[0,255,0],None)
    @staticmethod
    def Warning(text:str):
        text = "[Warning] " + text+"\n"
        Console.print_with_color(text,[255,255,0],None)
    @staticmethod
    def Error(text:str):
        text = "[Error] " + text+"\n"
        Console.print_with_color(text,[255,0,0],None)
    @staticmethod
    def Debug(text:str):
        text = "[Debug] " + text+"\n"
        Console.print_with_color(text,[0,0,255],None)