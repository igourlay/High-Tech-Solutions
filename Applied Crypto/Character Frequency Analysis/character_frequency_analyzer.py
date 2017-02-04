import os

# Elementary character frequency cipher

## Creating an object that can be added to a list to conduct raw value computations

class Data(object):
    character = ""
    number = 0

def make_Data(character,number):
    data = Data()
    data.character = character
    data.number = number
    return data

## Creating dictionary of letters and their historical character frequency

key = list("abcdefghijklmnopqrstuvwxyz")

values = list([0.082, 0.015, 0.028, 0.043, 0.127, 0.022, 0.020, 0.061, 0.070, 0.002, 0.008,
              0.040, 0.024, 0.067, 0.075, 0.019, 0.001, 0.060, 0.063, 0.091, 0.028, 0.010,
              0.023, 0.001, 0.020, 0.001])

freq_dist_key = dict(zip(key,values))

## Allow user to analyze any file in current directory (ciphertext)

s1 = os.getcwd()
print(s1 + '\n')

s1 += '\\'

s2 = input("Enter file name and extension: ")
fileName = s1 + s2

file = open(fileName)
raw = file.read()

print('\n'+raw + '\n')
raw = raw.lower()

## Creating and adding data objects to list for further computations

raw_len = 0

raw_list = list([])

for token in key:
    num_char = raw.count(token)
    raw_len += num_char
    s = make_Data(token,num_char)
    raw_list.append(s)
    print("Character: " + token + ", " + str(num_char))

print('\n' + "Total number of characters: " + str(raw_len) +'\n')


## Computing character frequency in selected file and creating frequency distribution list (ciphertext)

char_freq = list([])

for index in range(len(raw_list)):
    freq = float(raw_list[index].number/raw_len)
    freq = round(freq,3)
    print(raw_list[index].character + ": " + str(freq))
    s = make_Data(raw_list[index].character,freq)
    char_freq.append(s)

## Correlating ciphertext character frequency to historical character frequency

historical_char_freq = list([])

for token in key:
    s = make_Data(token,freq_dist_key.get(token))
    historical_char_freq.append(s)

predicted_key = list([])
predicted_value = list([])


for i in range(0,100,1):
    for index in range(len(historical_char_freq)):
        print(index)
        for token in key:
            print(token)
            if (abs(freq_dist_key.get(token)-(historical_char_freq[index].number))<(0.015*(100/(len(historical_char_freq)+1)))):
                predicted_key.append(token)
                predicted_value.append(historical_char_freq[index].character)
                del historical_char_freq[index]
                break
        if(len(historical_char_freq)<len(char_freq)):
            break


predicted_cipher = dict(zip(predicted_key,predicted_value))

print(predicted_cipher)