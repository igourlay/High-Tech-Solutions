
# Elementary transposition cipher with user defined keyword
# Written by: Ivan Gourlay

plainText = input("Enter some text: ").lower()
plainText = list(plainText)

print(plainText)

keyword = input("Enter keyword: ").lower()
keyword = keyword.replace(" ","")
keyword = list(keyword)

keys = list("abcdefghijklmnopqrstuvwxyz")
values = list([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26])
cipher_key = dict(zip(keys,values))

transpose_key = dict(zip(values,keys))

cipher_text = ""

index = 0

while index < len(plainText):

    for move in range(len(keyword)):

        if index == len(plainText):
            break

        if plainText[index] == " ":
            cipher_text += " "
            index += 1

        y = cipher_key.get(plainText[index]) + cipher_key.get(keyword[move])

        if y>26:
            y = abs(y-26)
            cipher_text += transpose_key.get(y)
            index += 1
        else:
            cipher_text += transpose_key.get(y)
            index += 1

print(cipher_text)





