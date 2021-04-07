import matplotlib.pyplot as plt
import sys, os
def get_input(filename):
    rows = []
    with open (filename) as f:
        for r in f:
            tokens = r.split(":")
            size = int(tokens[0].split("size ")[1])
            seconds = float(tokens[1].split(",")[0])
            bandwidth = float(tokens[2].rstrip("bytes/s\n"))
            rows.append((size, seconds, bandwidth))
    return rows

def plot_bandwidth(filename):
    rows = get_input(filename)
    sizes = list(range(len(rows)))
    # seconds_y = [row[1] * 100000000 for row in rows]
    bandwidth_y = [row[2] / 1000000 for row in rows]
    # plt.scatter(sizes, seconds_y, label = "Time (ms)", s=1)
    # plt.plot(sizes, bandwidth_y, label = "Bandwidth (MB/s)")
    plt.scatter(sizes, bandwidth_y, label = "Bandwidth (MB/s)", s=1)
    plt.xlabel("iteration")
    plt.legend()
    plt.title(f"{filename}")
    # plt.show()
    plt.savefig(f"{filename}.png")
    plt.close()

def plot_compare_bandwidth(f1, f2):
    rows1 = get_input(f1)
    rows2 = get_input(f2)
    print(len(rows1))
    print(len(rows2))
    sizes = list(range(len(rows1)))
    bandwidth_y1 = [row[2] / 1000000 for row in rows1]
    bandwidth_y2 = [row[2] / 1000000 for row in rows2]
    # plt.scatter(sizes, seconds_y, label = "Time (ms)", s=1)
    # plt.plot(sizes, bandwidth_y, label = "Bandwidth (MB/s)")
    plt.scatter(sizes, bandwidth_y1, label = "Bandwidth multiple (MB/s)", s=1)
    plt.scatter(sizes, bandwidth_y2, label = "Bandwidth single (MB/s)", s=1)
    plt.xlabel("iteration")
    plt.legend()
    plt.title(f"Single vs multiple")
    # plt.show()
    plt.savefig(f"compare.png")
    plt.close()
def main():
    dirs = ["multiple/", "single/"]
    for d in dirs:
        for img in filter(lambda x : not x.endswith(".png") and not x.endswith(".c"), os.listdir(d)):
            plot_bandwidth(d + img)

    plot_compare_bandwidth("multiple/inter-node", "single/inter-node")
    

main()
