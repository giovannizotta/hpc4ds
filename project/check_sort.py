t = """"""
def check_sort(t):
    t = t.split("\n")
    t = [e.split(":") for e in t]
    k = [e[0] for e in t]
    v = [int(e[1]) for e in t]
    if len(set(k)) == len(k):
        print("Same length")
    else:
        print(str(len(set(k))) + " vs " + str(len(k)))
        for i in range(len(k)):
            for j in range(i+1, len(k)):
                if k[i] == k[j]:
                    print(i, j, k[i], k[j])
    if sorted(v) == v:
        print("Sorted!")
    else:
        print("Not sorted :(")

check_sort(t)