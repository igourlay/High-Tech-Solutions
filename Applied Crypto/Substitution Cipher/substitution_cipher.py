import os

# Elementary substitution cipher of a text file with a predetermined key
# Written by: Ivan Gourlay

p = input("Enter '1' to input plaintext filename or '2' to manually input plaintext: ")

p = int(p)

if p == 1:

        s1 = os.getcwd()
        print(s1 + '\n')

        s1 += '\\'

        s2 = input("Enter file name and extension: ")
        fileName = s1 + s2

        file = open(fileName)
        raw = file.read()
        print("\n")
        print("Original text: " + raw + '\n')

        # raw = raw.replace(" ","")
        raw = raw.replace("\n", "")
        raw = raw.lower()
        print("Lowercase text: " + raw + '\n')

        key = list("abcdefghijklmnopqrstuvwxyz")

        value = list("vjtngpiqsrlzacbxwuydefhkmo")

        dictionary = dict(zip(key, value))

        x = ""

        for token in raw:
                if token in dictionary:
                        y = dictionary.get(token)
                        x += y
                else:
                        x += token

        print("Ciphertext: " + x)
        print("\n")

elif p == 2:

        z = ""

        key = list("abcdefghijklmnopqrstuvwxyz")

        value = list("vjtngpiqsrlzacbxwuydefhkmo")

        dictionary = dict(zip(key, value))

        s = input("Enter some text: ")
        print("Original text: " + s)
        s = s.lower()
        print("Lowercase text: " + s)

        for token in s:
                if token in dictionary:
                        y = dictionary.get(token)
                        z += y
                else:
                        z += token

        print("Ciphertext: " + z)
