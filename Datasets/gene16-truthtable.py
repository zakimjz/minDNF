#!/usr/bin/env python
idx = {'A':1,'B':2,'C':3,'D':4,
       'E':5,'F':6, 'G':7, 'H': 8, 'I': 9, 'J':10, 'K':11, 
       'L':12, 'M':13, 'N':14, 'X1':15, 'X2':16}

def powerset(elements):
    if len(elements) > 0:
        head = elements[0]
        for tail in powerset(elements[1:]):
            yield [head] + tail
            yield tail
    else:
        yield []

#n means negative/repression. each gene is turned on only if 
#the rule is met
#order of rules is important, since I do sequential check
order = ['B', 'M', 'E', 'G', 'L', 'F', 'D', 'H', 'J']
rules = {  
         'B':['nA'],
         'M':['nK'],
         'E':['nB'],
         'G':['B'],
         'L':['C', 'M'],
         'F':['A', 'nL'],
         'D':['X1', 'X2', 'C', 'F'],
         'H':['nF'],
         'J':['nH'],
        }
#print idx, rules

#input genes
input = ['A', 'C', 'I', 'K', 'N', 'X1', 'X2']
input = ['X2', 'X1', 'N', 'K', 'I', 'C', 'A']

#generate truth table
cnt = 1
for x in powerset(input):
    #print x
    tup = x
    for g in [y for y  in input if y not in tup]:
        tup.append('n'+g)
    for g in order:
        tt = map(lambda x: x in tup, rules[g])
        #print g, rules[g], tt, "--", all(tt)
        if all(tt):
            tup.append(g)
        else:
            tup.append('n'+g)

    #print tup
    vals = [idx[y] for y in idx if y in tup]
    print cnt, cnt, len(vals), " ".join([str(y) for y in sorted(vals)])
    cnt += 1
