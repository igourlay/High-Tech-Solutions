import nltk

f = open("plainText1.txt")
raw = f.read().lower()

print(raw)

tokens = nltk.word_tokenize(raw)

print(tokens)

keys = list([])
absolute_count_values = list([])
relative_count_values = list([])
total_words = 0

alphabet = "abcdefghijklmnopqrstuvwxyz"

for token in tokens:
        if token[:1] in alphabet:
                total_words += 1

for token in tokens:
    keys.append(token)
    absolute_count_values.append(tokens.count(token))
    relative_count_values.append(round(tokens.count(token)/total_words,3))

absolute_word_frequency = dict(zip(keys,absolute_count_values))
relative_word_frequency = dict(zip(keys,relative_count_values))

print(absolute_word_frequency)
print (total_words)
print(relative_word_frequency)

# for searching bigrams and trigrams use raw.count(token) for searching all instances of 'is','or','ca', etc

