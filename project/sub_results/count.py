import pandas as pd

data = pd.read_csv('done.txt', engine='python', sep=' ')

data['todo'] = data['count'].apply(lambda x: max(0, 50-x))

print(data[data['todo'] > 0])


s = data['todo'].sum()
print(s, (s * 10) // 60, (s * 10) % 60)
