import matplotlib.pyplot as plt
import sys
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

def main():
    file1 = sys.argv[1].split("/")[-1]
    file2 = sys.argv[2].split("/")[-1]
    rows1 = get_input(sys.argv[1])
    rows2 = get_input(sys.argv[2])

    sizes = list(range(len(rows1)))
    bandwidth_1 = [row[2] / 1000000 for row in rows1]
    bandwidth_2 = [row[2] / 1000000 for row in rows2]
    plt.plot(sizes, bandwidth_1, label = f"Bandwidth {file1} (MB/s)")
    plt.plot(sizes, bandwidth_2, label = f"Bandwidth {file2} (MB/s)")
    plt.xlabel("log(packet size)")
    plt.legend()
    plt.title(f"Comparison {file1} {file2}")
    plt.show()
    plt.savefig(f"{file1}==={file2}_compare.png")

main()
