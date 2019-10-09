f = open("mottoTest", 'w')
str = "Faith is to believe what you do not see, the reward of this faith is to see what you believe."
for i in range(int(250000000 / len(str))):
    f.write(str)
    f.write('\n')
f.close()
