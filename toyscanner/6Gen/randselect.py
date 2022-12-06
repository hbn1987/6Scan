import random

if __name__=="__main__":
    result=[]
    srcfd=open("../seedsets/biasdseeds","r")
    dstfd=open("./output/biasdseeds@700","w")#
    linenum=700#
    srclines=srcfd.readlines()
    srclen=len(srclines)
    while len(set(result))<linenum:
        s=random.randint(0,srclen-1)
        result.append(srclines[s])
    for content in sorted(set(result)):
        dstfd.write(content)
    srcfd.close()
    dstfd.close()