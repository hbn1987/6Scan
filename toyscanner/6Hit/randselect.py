import random

if __name__=="__main__":
    result=[]
    srcfd=open("./seeds","r")
    dstfd=open("./seeds@10","w")#
    linenum=100000#
    srclines=srcfd.readlines()
    srclen=len(srclines)
    while len(set(result))<linenum:
        s=random.randint(0,srclen-1)
        result.append(srclines[s])
    for content in sorted(set(result)):
        dstfd.write(content)
    srcfd.close()
    dstfd.close()