

#%%

x = '''
J iwzbn'y ebbn yijp jssjywemb jn w aijmb. Qwdeb pxsbwujnv qdpbmg po yijn awp w qjpywhb.
J gbbm fngfmgjmmbu. J gbbm vfjmyd. J gbbm mjhb J'q bwyjnv qdpbmg. Dby xwsy og qb hnoap os wy mbwpy ebmjbzbp yiwy J'q twxwemb.
J gbbm w twnumb ebjnv efsny wy eoyi bnup, efy jy'p noy vjzjnv w esjviy mjviy.
Aid twn'y J gotfp? Qd uwdp wsb jmm-pxbny, wnu dby …jnv; J'q noy vsoajnv; J wq yisoajnv wawd qd yjqb.
Aiwy wq J aoshjnv yoawsu?
Yibsb'p wn jnybsnwm mwth og ionbpyd. J'q noy bzbn mbyyjnv qd qjnu awnubs. J wq rfjbyjnv jy.
Wq J awjyjnv gos poqbyijnv?
Aibn J vby ioqb, J uon'y awny yo xsovswq wndqosb. J gbbm mjhb J'q yswujnv qd xwyjbntb gos qonbd.'''




x=  x.lower()

c_counts = {}

for c in x:
  if c in c_counts:
    c_counts[c] += 1
  else:
    c_counts[c] = 1



count_list = sorted(c_counts.items(), key=lambda x: x[1], reverse=True)

for c, count in count_list:
  print(c, " : ", count)



key = '''
ji
be
gf
ml
hk
qm
dy
ps
yt
oo
nn
ih
vg
ud
aw
wa
sr
xp
zv
eb
fu
tc
rq
'''

keymap = {
}

for row in key.split("\n"):
  if len(row) == 2:
    keymap[row[0]] = row[1]




def decrypt_char(x):
  if x in keymap:
    return keymap[x].upper()
  return x

def decrypt(x):
  return "".join(list(map(lambda x: decrypt_char(x), x)))


print(decrypt(x))




res = '''
IT FEELS LIKE I'M BEING GROUND DOWN, LIKE I'M BEING THRASHED AROUND. I'M TIRED, AND IN THE PLACE I LOVE MOST.
I'VE BEEN LOOKING FOR A JOB, TRYING TO LIMIT MY APPLICATIONS I'M INTERESTED IN, BUT NONE OF THEM SEEM INTERESTED IN ME. THIS HURTS MUCH MORE THAN BEING REJECTED FROM PLACES WHERE I'M LESS INTERESTED/INVESTED IN THE WORK THEY ARE DOING. THE WORLD IS TELLING ME THAT I'M NOT GOOD AT THE THINGS I ENJOY.
I HURT MY SHOULDER. MARVIN IS NOT DOING WELL. MONEY FEELS TIGHT. SIEVVY FEELS HOPELESS. I DIDN'T GET INTO THE MASTER'S PROGRAM. I'M SO CONFUSED ABOUT THE CURRENT STATE OF THE JOB I (ALMOST?) GOT. MY PARENTS ARE GETTING SEPARATED.
MY NEW POTENTIAL-MAYBE-IS-IT-REALLY-HAPPENING-EMPLOYMENT TERMS THAT I'M DEFINING SEEMS LIKE A FAT GAMBLE. A WAY TO PRETEND LIKE I'M DOING THE RIGHT THING, BECAUSE IN THE MOST UNLIKELY SCENARIO, I WILL MAKE A GOOD DEAL MONEY. I HATE THE RAT RACE, BUT ONLY BECAUSE I'M NOT WINNING. I FEEL LIKE I'M NOT SMART ENOUGH, BUT I ALSO FEEL LIKE I AM.
I WANT TO PROGRAM CREATIVELY. I HAVEN'T DONE THAT IN A WHILE. THIS SECRET SECTION IS A BIT OF THAT DESIRE FINALLY MAKING ITS WAY OUT.
I'VE GOTTEN THRASHED BEFORE, BUT THIS FEELS WORSE. SLOWER. LIKE A MILL.
'''
