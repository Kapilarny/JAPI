import pefile
import sys
import os.path

dll = pefile.PE(sys.argv[1])
dll_basename = os.path.basename(sys.argv[1]).split('.')[0]

string = "LIBRARY {}\n".format(sys.argv[2])
string += "\n"
string += "EXPORTS\n"

for export in dll.DIRECTORY_ENTRY_EXPORT.symbols:
    if export.name:
        string += ('\t{}={}.{} @{}\n'.format(export.name.decode(), dll_basename, export.name.decode(), export.ordinal))

with open(sys.argv[3], 'w') as f:
    f.write(string)