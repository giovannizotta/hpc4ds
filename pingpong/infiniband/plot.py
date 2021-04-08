import matplotlib.pyplot as plt
import sys
rows = []
def get_input(filename):
    with open (filename) as f:
        for r in f:
            tokens = r.split(":")
            size = int(tokens[0].split("size ")[1])
            seconds = float(tokens[1].split(",")[0])
            bandwidth = float(tokens[2].rstrip("bytes/s\n"))
            rows.append((size, seconds, bandwidth))

def main():
    get_input(sys.argv[1])
    sizes = list(range(len(rows)))
    seconds_y = [row[1] * 1000 for row in rows]
    bandwidth_y = [row[2] / 1000000 for row in rows]
    plt.plot(sizes, seconds_y, label = "Time (ms)")
    plt.plot(sizes, bandwidth_y, label = "Bandwidth (MB/s)")
    plt.xlabel("log(packet size)")
    plt.legend()
    plt.title(f"{sys.argv[1]}")
    plt.show()
    plt.savefig(f"{sys.argv[1]}.png")

main()
