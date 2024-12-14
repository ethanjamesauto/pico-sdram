import pandas as pd

# Read the CSV file
df = pd.read_csv('xilinx-sd4.csv', comment='#')
offset = df['Time (s)'][0]

with open('tmp.v', 'w') as f:
    # iterate over the rows
    for index, row in df.iterrows():
        changed = False
        for sigs in ['ser1', 'ser2', 'ser3', 'rclk', 'srclk', 'clk']:
            # see if value has changed
            if index == 0 or row[sigs] != df.iloc[index-1][sigs]:
                changed = True
                # print('#%fus' % (row['Time (s)'] - offset,))
                f.write('\n#%.3fns;' % ((row['Time (s)'] - offset) * 1e9/20))
                offset = row['Time (s)']
                break

        for sigs in ['ser1', 'ser2', 'ser3', 'rclk', 'srclk', 'clk']:
            if index == 0 or row[sigs] != df.iloc[index-1][sigs]:
               # ('  %s <= %d;' % (sigs, row[sigs]))
               f.write('  %s <= %d;' % (sigs, row[sigs]))