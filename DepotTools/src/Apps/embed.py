import sys

input_path=sys.argv[1]
output_path=sys.argv[2]
variable_name=sys.argv[3]



def byte_to_hex(b):
    return hex(b)



with open(input_path, 'rb') as f:
    data = f.read()
    
# Write header file


with open(output_path, 'w') as f:
    f.write("#include <array>\n")
    f.write('extern "C"\n')
    f.write(f"const std::array<unsigned char,{len(data)}> " + variable_name + " = {")
    for i in range(len(data)):
        char=data[i]
        
        f.write(byte_to_hex(char) + ', ')
        if i % 16 == 15:
            f.write('\n')
    f.write('};')
    