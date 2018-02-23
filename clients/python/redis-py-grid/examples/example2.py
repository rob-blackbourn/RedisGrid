from redisgrid import StrictRedis
import pandas as pd

client = StrictRedis(host="localhost")

df = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
client.grid_save_df('df', df)
df2 = client.grid_load_df('df')
print(df2)
