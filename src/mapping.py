import sys
arg = sys.argv
if(len(arg) != 3):
    print("Wrong argument number!")
    sys.exit(1)
fin = open(arg[1], "r", encoding = "big5-hkscs")
fop = open(arg[2], "w", encoding = "big5-hkscs")
fop1 = open("test.txt", "w")
ZhuYin_list = fin.readlines()
fin.close()
Big5_list = []
for line in ZhuYin_list:
    character = line[0]
    #print("Character:", character)
    new_zhu = 1
    for n in range(len(line)):
        if(new_zhu == 1):
            if(line[n].encode('big5-hkscs')[0] == 0xA3):
                if(line[n].encode('big5-hkscs')[1] >= 0x74 and line[n].encode('big5-hkscs')[1] <= 0xBA):
                    new_zhu = 0
                    if(len(Big5_list) == 0):
                        Big5_list.append(line[n])
                        Big5_list[0] = Big5_list[0] + " " + character
                    else:
                        exist_ch = 0
                        exist_zhu = 0
                        for i in range(len(Big5_list)):
                            if(Big5_list[i][0] == line[n]):
                                exist_zhu = 1
                                for j in range(len(Big5_list[i])):
                                    if(Big5_list[i][j] == character):
                                        exist_ch = 1
                                        break
                                if(exist_ch == 0):
                                    Big5_list[i] = Big5_list[i] + " " + character
                                break
                        if(exist_zhu == 0):
                            Big5_list.append(line[n])
                            Big5_list[-1] = Big5_list[-1] + " " + character
        
        else:
            if(line[n] == '/'):
                new_zhu = 1
    Big5_list.append(character)
    Big5_list[-1] = Big5_list[-1] + " " + character
for n in range(len(Big5_list)):
    Big5_list[n] = Big5_list[n] + "\n"
fop.writelines(Big5_list)
fop1.writelines(Big5_list)     
fop.close()
fop1.close()
