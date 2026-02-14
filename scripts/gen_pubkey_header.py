# Load the public key from the file and generate a C header file with the public key as a byte array.
import base64
import os

def main():
    # Load the public key from the file
    with open("public.key", "rb") as f:
        pub_key_raw = f.read().strip()

    # Convert from base64 into binary
    public_key = base64.b64decode(pub_key_raw)

    # Generate the C header file with the public key as a byte array
    with open("../libs/jsign/internal/public_key.h", "w") as f:
        f.write("#ifndef PUBLIC_KEY_H\n")
        f.write("#define PUBLIC_KEY_H\n\n")
        f.write("const unsigned char public_key[] = {\n")
        for i in range(0, len(public_key), 16):
            line = ", ".join(f"0x{b:02x}" for b in public_key[i:i+16])
            f.write(f"    {line},\n")
        f.write("};\n\n")
        f.write("#endif // PUBLIC_KEY_H\n")

if __name__ == "__main__":
    main()