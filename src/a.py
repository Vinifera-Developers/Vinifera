#!/usr/bin/env python3
import os

def fix_file(path):
    with open(path, "rb") as f:
        data = f.read()

    # Detect the newline used in the file
    if b"\r\n" in data:
        nl = b"\r\n"
    else:
        nl = b"\n"

    # Strip trailing spaces/tabs/newlines at EOF
    data = data.rstrip(b" \t\r\n")

    # Now add exactly one newline
    data = data + nl

    with open(path, "wb") as f:
        f.write(data)


def main():
    for root, dirs, files in os.walk("."):
        for filename in files:
            if filename.lower().endswith(".cpp"):
                path = os.path.join(root, filename)
                print("Fixing:", path)
                fix_file(path)


if __name__ == "__main__":
    main()
