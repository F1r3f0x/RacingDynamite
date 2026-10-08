import sys
import re

def main():
    if len(sys.argv) < 3:
        print("Usage: refactor_func.py <file> <func_name>")
        return
    
    file_path = sys.argv[1]
    func_name = sys.argv[2]
    
    with open(file_path, 'r') as f:
        c = f.read()
        
    pattern = r"/\*\*\s*\n\s*\*\s*@original " + func_name + r".*?\n\s*\*/\n.*?^}"
    match = re.search(pattern, c, re.DOTALL | re.MULTILINE)
    if match:
        print(match.group(0))
    else:
        print("Function not found")

if __name__ == "__main__":
    main()
